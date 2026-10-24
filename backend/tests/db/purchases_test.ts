// Steam purchases against the local database with a fake Steam: the flow the store edge
// functions run, the worker's sweep of orders the client never finalized, and the refund and
// chargeback reversals from Steam's report.
import { assert, assertEquals } from "jsr:@std/assert@1.0.13";
import postgres from "npm:postgres@3.4.5";
import { finalizePurchase, initPurchase } from "../../supabase/functions/_shared/purchases.ts";
import type { InitItem, SteamOrder, SteamStore, SteamTxnStatus } from "../../supabase/functions/_shared/steam_store.ts";
import { SteamError } from "../../supabase/functions/_shared/steam_store.ts";
import type { WorkerContext } from "../../worker/loop.ts";
import { purchasesPass, sqlRpc } from "../../worker/purchases_pass.ts";

const DB_URL = Deno.env.get("DATABASE_URL") ?? "postgresql://postgres:postgres@127.0.0.1:55422/postgres";
const tag = `${Date.now() % 1_000_000}`;

class FakeSteam implements SteamStore {
  orders = new Map<string, { status: SteamTxnStatus; item: InitItem; currency: string }>();
  failFinalize = false;
  failReport = false;
  queries = 0;
  reportRows: SteamOrder[] = [];
  userCurrency() {
    return Promise.resolve("USD");
  }
  initTxn(orderId: string, _steamId: string, currency: string, item: InitItem) {
    this.orders.set(orderId, { status: "Init", item, currency });
    return Promise.resolve({ transId: `t${orderId}` });
  }
  approve(orderId: string) {
    this.orders.get(orderId)!.status = "Approved";
  }
  finalizeTxn(orderId: string) {
    const o = this.orders.get(orderId);
    if (!o || o.status !== "Approved" || this.failFinalize) {
      return Promise.reject(new SteamError("1", "finalize refused"));
    }
    o.status = "Succeeded";
    return Promise.resolve({ transId: `t${orderId}` });
  }
  queryTxn(orderId: string) {
    this.queries++;
    const o = this.orders.get(orderId);
    // Like Steam: an order it never saw is an error, not a status.
    if (!o) return Promise.reject(new SteamError("7", "order not found"));
    return Promise.resolve({ orderId, transId: `t${orderId}`, status: o.status });
  }
  report() {
    return this.failReport ? Promise.reject(new SteamError("http_503", "down")) : Promise.resolve(this.reportRows);
  }
}

Deno.test("Steam purchases", async (t) => {
  const sql = postgres(DB_URL, { max: 2, onnotice: () => {} });
  const steam = new FakeSteam();
  const rpc = sqlRpc(sql);
  const ctx = { sql, functionsUrl: "", log: () => {}, steam } as unknown as WorkerContext;
  const account = crypto.randomUUID();
  const claim = `p-${tag}`;
  const credit = `credit.${tag}`;
  const sponsor = `sponsor.${tag}`;
  const wallet = async () =>
    Number(
      (await sql`select coalesce(sum(balance), 0)::text as b from game.wallets
                      where account_id = ${account} and currency = 'sponsor_credit'`)[0].b,
    );
  try {
    const [busy] = await sql`select claim_id from game.claims where state = 'active'`;
    assert(!busy, `another Claim is active locally (${busy?.claim_id})`);
    const name = `pu_${tag}`;
    await sql`insert into auth.users (id, email, aud, role, raw_app_meta_data)
              values (${account}, ${name + "@users.kinetic.invalid"}, 'authenticated', 'authenticated',
                      ${sql.json({ kinetic_provider: "steam", kinetic_provider_user_id: "7656119" + tag })})`;
    await sql`select api.srv_ensure_account(${account}, 'steam', ${"7656119" + tag}, ${name})`;
    await sql`insert into game.claims (claim_id, name, arena, state, starts_at, ends_at)
              values (${claim}, 'Purchase Claim', 'YRD', 'active', now() - interval '1 day', now() + interval '6 days')`;
    const item = 900000 + Number(tag) % 99999;
    await sql`insert into game.skus (sku, kind, steam_item_id, description, prices, credit, claim_id) values
              (${credit}, 'sponsor_credit', ${item}, 'Sponsor credit, 500', ${sql.json({ USD: 499 })}, 500, null),
              (${sponsor}, 'sponsorship', ${item + 1}, 'Kessl Sponsorship', ${sql.json({ USD: 999 })}, null, ${claim})`;

    let creditOrder = "";
    await t.step("an order opens, is billed in the wallet currency, and only one at a time", async () => {
      const r = await initPurchase(rpc, steam, account, credit);
      assert(r.ok);
      creditOrder = String(r.body.order_id);
      assertEquals(steam.orders.get(creditOrder)!.item.amountCents, 499);
      const again = await initPurchase(rpc, steam, account, credit);
      assertEquals(again.ok ? "" : again.error, "purchase_open");
    });

    await t.step("finalize grants the credit once", async () => {
      steam.approve(creditOrder);
      const r = await finalizePurchase(rpc, steam, account, creditOrder, true);
      assert(r.ok && r.body.granted === true);
      assertEquals(await wallet(), 500);
      const retry = await finalizePurchase(rpc, steam, account, creditOrder, true);
      assert(retry.ok && retry.body.duplicate === true);
      assertEquals(await wallet(), 500);
      const other = await finalizePurchase(rpc, steam, crypto.randomUUID(), creditOrder, true);
      assertEquals(other.ok ? "" : other.error, "unknown_order");
    });

    await t.step("a declined overlay cancels the order", async () => {
      const r = await initPurchase(rpc, steam, account, credit);
      assert(r.ok);
      const declined = await finalizePurchase(rpc, steam, account, String(r.body.order_id), false);
      assert(declined.ok && declined.body.status === "cancelled");
    });

    await t.step("the worker finishes an order the client never finalized", async () => {
      const r = await initPurchase(rpc, steam, account, credit);
      assert(r.ok);
      const order = String(r.body.order_id);
      steam.approve(order);
      await sql`update game.purchases set created_at = now() - interval '11 minutes' where platform_order_id = ${order}`;
      await sql`delete from game.config where key = 'steam_report_next_at'`;
      await purchasesPass(ctx);
      const [p] = await sql`select status from game.purchases where platform_order_id = ${order}`;
      assertEquals(p.status, "finalized");
      assertEquals(await wallet(), 1000);
    });

    await t.step(
      "an order the client left open is checked with backoff, and Steam is not asked every tick",
      async () => {
        const r = await initPurchase(rpc, steam, account, credit);
        assert(r.ok);
        const order = String(r.body.order_id);
        await sql`update game.purchases set created_at = now() - interval '11 minutes' where platform_order_id = ${order}`;
        steam.queries = 0;
        await purchasesPass(ctx);
        await purchasesPass(ctx);
        await purchasesPass(ctx);
        assertEquals(steam.queries, 1, "one check, then backoff");
        const [p] = await sql`select status, checks, next_check_at > now() as later from game.purchases
                            where platform_order_id = ${order}`;
        assertEquals([p.status, p.checks, p.later], ["initiated", 1, true]);
        // A day on, still unpaid: closed.
        await sql`update game.purchases set created_at = now() - interval '25 hours', next_check_at = null
                 where platform_order_id = ${order}`;
        await purchasesPass(ctx);
        const [closed] = await sql`select status from game.purchases where platform_order_id = ${order}`;
        assertEquals(closed.status, "failed");
      },
    );

    await t.step("an abandoned order Steam never saw is closed, not checked forever", async () => {
      const [ghost] =
        await sql`insert into game.purchases (account_id, sku, kind, credit, amount, currency_code, created_at)
                                values (${account}, ${credit}, 'sponsor_credit', 500, 499, 'USD', now() - interval '25 hours')
                                returning platform_order_id::text as order_id`;
      await purchasesPass(ctx);
      const [p] =
        await sql`select status, failure_reason from game.purchases where platform_order_id = ${ghost.order_id}`;
      assertEquals([p.status, p.failure_reason], ["failed", "steam_unknown_order"]);
      const [flag] = await sql`select 1 as x from game.support_flags where ref = ${"order:" + ghost.order_id}`;
      assert(flag, "and support checks it with Steam");
    });

    await t.step("two Buy taps at once open one order", async () => {
      const both = await Promise.all([
        initPurchase(rpc, steam, account, credit),
        initPurchase(rpc, steam, account, credit),
      ]);
      assertEquals(both.filter((x) => x.ok).length, 1);
      assertEquals(both.filter((x) => !x.ok).map((x) => !x.ok && x.error), ["purchase_open"]);
      const open = both.find((x) => x.ok)!;
      assert(open.ok);
      // The SKU changes after the order opened: the order still grants what it was sold as.
      await sql`update game.skus set credit = 900 where sku = ${credit}`;
      steam.approve(String(open.body.order_id));
      const before = await wallet();
      assert((await finalizePurchase(rpc, steam, account, String(open.body.order_id), true)).ok);
      assertEquals(await wallet(), before + 500);
      await sql`update game.skus set credit = 500 where sku = ${credit}`;
    });

    await t.step("Sponsorship is an entitlement, and a chargeback takes it back and flags support", async () => {
      const r = await initPurchase(rpc, steam, account, sponsor);
      assert(r.ok);
      const order = String(r.body.order_id);
      steam.approve(order);
      assert((await finalizePurchase(rpc, steam, account, order, true)).ok);
      const [e] = await sql`select 1 as x from game.entitlements where account_id = ${account}
                            and entitlement = ${"sponsorship:" + claim}`;
      assert(e);
      const twice = await initPurchase(rpc, steam, account, sponsor);
      assertEquals(twice.ok ? "" : twice.error, "already_sponsored");
      // Spend most of the credit, then Steam reports a chargeback, a refund and a partial refund.
      const total = await wallet();
      await sql`update game.wallets set balance = 200 where account_id = ${account} and currency = 'sponsor_credit'`;
      steam.reportRows = [
        { orderId: order, transId: "x", status: "Chargedback" },
        { orderId: creditOrder, transId: "y", status: "Refunded" },
        { orderId: creditOrder, transId: "y", status: "PartialRefund" },
      ];
      await sql`delete from game.config where key = 'steam_report_next_at'`;
      await purchasesPass(ctx);
      const [gone] = await sql`select count(*)::int as n from game.entitlements where account_id = ${account}`;
      assertEquals(gone.n, 0);
      assertEquals(await wallet(), 0);
      assert(total > 0);
      const flags =
        await sql`select reason, ref from game.support_flags where account_id = ${account} order by flag_id`;
      assertEquals(flags.map((f) => f.reason), [
        "steam_unknown_order",
        "charged_back_sponsorship",
        "refunded_after_spend",
        "partial_refund",
      ]);
      assert(flags[2].ref.endsWith("short:300"));
    });

    await t.step("a report Steam fails to give is read again soon, from the same cursor", async () => {
      const [before] = await sql`select value #>> '{}' as v from game.config where key = 'steam_report_cursor'`;
      steam.failReport = true;
      await sql`delete from game.config where key = 'steam_report_next_at'`;
      await purchasesPass(ctx);
      const [after] = await sql`select value #>> '{}' as v from game.config where key = 'steam_report_cursor'`;
      const [next] = await sql`select (value #>> '{}')::timestamptz < now() + interval '10 minutes' as soon
                               from game.config where key = 'steam_report_next_at'`;
      assertEquals([after.v, next.soon], [before.v, true]);
      steam.failReport = false;
    });
  } finally {
    await sql.begin(async (tx) => {
      await tx`delete from game.purchases where sku in (${credit}, ${sponsor})`;
      await tx`delete from game.skus where sku in (${credit}, ${sponsor})`;
      await tx`delete from auth.users where id = ${account}`;
      await tx`delete from game.claims where claim_id = ${claim}`;
      await tx`delete from game.config where key in ('steam_report_cursor', 'steam_report_next_at')`;
    });
    await sql.end();
  }
});
