// The worker's purchases pass (backend-architecture 13.4 step 4):
// - Orders left `initiated` (the client crashed between Steam's authorization and finalize, or
//   the overlay was closed) are asked of Steam, each on its own backoff (from a minute,
//   doubling, at most hourly): paid ones are finalized and granted, failed ones closed. After
//   ABANDONED_AFTER_S an order still unpaid, or one Steam does not know, is closed.
// - Steam's transaction report is read every REPORT_EVERY_S (every RETRY_REPORT_S after a
//   failure): refunds and chargebacks on finalized orders are reversed, partial refunds are
//   flagged for support. The cursor advances only when every row was handled, so a failed
//   reversal is read again; reversals are idempotent.
// Without Steam configured (no publisher key), the pass does nothing.
import type postgres from "npm:postgres@3.4.5";
import type { Json } from "../supabase/functions/_shared/database.types.ts";
import {
  completeOrder,
  FlowError,
  PAID,
  PARTIAL,
  REVERSED,
  type Rpc,
} from "../supabase/functions/_shared/purchases.ts";
import { SteamError } from "../supabase/functions/_shared/steam_store.ts";
import type { WorkerContext } from "./loop.ts";

const STALE_AFTER_S = 10 * 60;
const ABANDONED_AFTER_S = 24 * 3600;
const ORDERS_PER_PASS = 20;
export const REPORT_EVERY_S = 3600;
const RETRY_REPORT_S = 300;
// Read the report from a little before the last cursor: reversals are idempotent.
const REPORT_OVERLAP_S = 600;
const REPORT_PAGE = 1000;
const IDENT = /^srv_[a-z_]+$/;
// Reversal errors that mean "nothing to do here", not "try again".
const SETTLED = ["unknown_order", "order_not_finalized"];
// A report row failing this many reads is given to support and no longer holds the cursor.
const ROW_ATTEMPTS = 5;

const message = (e: unknown) => (e instanceof Error ? e.message : String(e));

// api.srv_* calls over the worker's SQL connection, with the same errors the edge functions see.
export function sqlRpc(sql: postgres.Sql): Rpc {
  return async (fn, args) => {
    if (!IDENT.test(fn)) throw new Error(`bad function ${fn}`);
    const names = Object.keys(args);
    const call = `select api.${fn}(${names.map((n, i) => `${n} => $${i + 1}`).join(", ")}) as result`;
    try {
      const [row] = await sql.unsafe(call, names.map((n) => args[n] as postgres.ParameterOrJSON<never>));
      return (row?.result ?? null) as Json;
    } catch (e) {
      throw new FlowError(message(e));
    }
  };
}

// Steam answered with an error of its own (as opposed to being unreachable or replying with
// something unreadable). CHECK: Steam's not-found error code, to tell "unknown order" apart.
const steamRefused = (e: unknown) => e instanceof SteamError && !e.code.startsWith("http_") && e.code !== "rejected";

async function checkOrder(ctx: WorkerContext, rpc: Rpc, orderId: string, abandoned: boolean): Promise<void> {
  const steam = ctx.steam!;
  let status: string;
  try {
    status = (await steam.queryTxn(orderId)).status;
  } catch (e) {
    if (abandoned && steamRefused(e)) {
      // Closed, and support checks it with Steam: if it was charged after all, they grant by hand.
      await rpc("srv_purchase_failed", { p_order: orderId, p_status: "failed", p_reason: "steam_unknown_order" });
      await rpc("srv_purchase_flag", { p_order: orderId, p_reason: "steam_unknown_order" });
      return;
    }
    throw e;
  }
  if (PAID.includes(status)) {
    const done = await completeOrder(rpc, steam, orderId);
    if (!done.ok) throw new Error(done.error);
  } else if (status === "Failed" || abandoned) {
    await rpc("srv_purchase_failed", { p_order: orderId, p_status: "failed", p_reason: `steam_${status}` });
  } else {
    await rpc("srv_purchase_checked", { p_order: orderId });
  }
}

async function sweepOpenOrders(ctx: WorkerContext): Promise<void> {
  const rpc = sqlRpc(ctx.sql);
  const open = await ctx.sql<{ order_id: string; abandoned: boolean }[]>`
    select platform_order_id::text as order_id,
           created_at < now() - make_interval(secs => ${ABANDONED_AFTER_S}) as abandoned
      from game.purchases
     where status = 'initiated' and created_at < now() - make_interval(secs => ${STALE_AFTER_S})
       and (next_check_at is null or next_check_at <= now())
     order by coalesce(next_check_at, created_at) limit ${ORDERS_PER_PASS}`;
  for (const o of open) {
    try {
      await checkOrder(ctx, rpc, o.order_id, o.abandoned);
    } catch (e) {
      (ctx.log ?? console.log)("[worker]", `purchase ${o.order_id}: ${message(e)}`);
      await rpc("srv_purchase_checked", { p_order: o.order_id }).catch(() => {});
    }
  }
}

// Counts a failing report row's reads (in game.config); true once it has failed too often.
async function rowExhausted(ctx: WorkerContext, orderId: string, error: string): Promise<boolean> {
  const [row] = await ctx.sql<{ n: number }[]>`
    insert into game.config (key, value) values ('steam_report_row_failures', ${ctx.sql.json({ [orderId]: 1 })})
    on conflict (key) do update
      set value = config.value || jsonb_build_object(${orderId}::text, coalesce((config.value ->> ${orderId})::int, 0) + 1)
    returning (value ->> ${orderId})::int as n`;
  if (row.n === ROW_ATTEMPTS) {
    const ref = `order:${orderId} ${error}`;
    await ctx.sql`insert into game.support_flags (reason, ref) values ('report_row_unhandled', ${ref})`;
  }
  return row.n >= ROW_ATTEMPTS;
}

async function rowHandled(ctx: WorkerContext, orderId: string): Promise<void> {
  await ctx.sql`update game.config set value = value - ${orderId}::text
                 where key = 'steam_report_row_failures' and value ? ${orderId}`;
}

// Handles one page of the report. Returns false if any row must be read again.
async function handleReportRows(ctx: WorkerContext, rpc: Rpc, orders: { orderId: string; status: string }[]) {
  let allHandled = true;
  for (const o of orders) {
    try {
      if (o.status === PARTIAL) {
        await rpc("srv_purchase_flag", { p_order: o.orderId, p_reason: "partial_refund" });
      } else if (REVERSED[o.status]) {
        await rpc("srv_purchase_reverse", { p_order: o.orderId, p_status: REVERSED[o.status] });
      }
      await rowHandled(ctx, o.orderId);
    } catch (e) {
      if (SETTLED.some((s) => message(e).includes(s))) continue;
      (ctx.log ?? console.log)("[worker]", `report row ${o.orderId}: ${message(e)}`);
      if (!(await rowExhausted(ctx, o.orderId, message(e)))) allHandled = false;
    }
  }
  return allHandled;
}

async function setConfig(ctx: WorkerContext, key: string, value: string): Promise<void> {
  await ctx.sql`
    insert into game.config (key, value) values (${key}, ${ctx.sql.json(value)})
    on conflict (key) do update set value = excluded.value`;
}

async function readReport(ctx: WorkerContext): Promise<void> {
  const [due] = await ctx.sql<{ cursor: string | null; due: boolean }[]>`
    select (select value #>> '{}' from game.config where key = 'steam_report_cursor') as cursor,
           coalesce((select (value #>> '{}')::timestamptz from game.config where key = 'steam_report_next_at')
                    <= now(), true) as due`;
  if (!due.due) return;
  const startedAt = new Date();
  let since = due.cursor ? new Date(new Date(due.cursor).getTime() - REPORT_OVERLAP_S * 1000) : new Date(0);
  const rpc = sqlRpc(ctx.sql);
  let allHandled = true;
  try {
    for (;;) {
      const orders = await ctx.steam!.report(since, REPORT_PAGE);
      allHandled = (await handleReportRows(ctx, rpc, orders)) && allHandled;
      const last = orders.at(-1)?.time;
      if (orders.length < REPORT_PAGE || !last || new Date(last) <= since) break;
      since = new Date(last);
    }
  } catch (e) {
    (ctx.log ?? console.log)("[worker]", `steam report: ${message(e)}`);
    allHandled = false;
  }
  if (allHandled) await setConfig(ctx, "steam_report_cursor", startedAt.toISOString());
  const nextIn = (allHandled ? REPORT_EVERY_S : RETRY_REPORT_S) * 1000;
  await setConfig(ctx, "steam_report_next_at", new Date(startedAt.getTime() + nextIn).toISOString());
}

export async function purchasesPass(ctx: WorkerContext): Promise<void> {
  if (!ctx.steam) return;
  await sweepOpenOrders(ctx);
  await readReport(ctx);
}
