// The Steam purchase flow (backend-architecture 13.4), shared by store-steam-init,
// store-steam-finalize and the worker's purchases pass. Steam is reached through SteamStore,
// the database through the service client; tests stand in a fake Steam.
import type { Json } from "./database.types.ts";
import { dbErrorCode } from "./http.ts";
import { jsonObject } from "./rpc.ts";
import { SteamError, type SteamStore } from "./steam_store.ts";
import type { Client } from "./supabase.ts";

export interface CreatedOrder {
  order_id: string;
  amount: number;
  currency: string;
  item_id: number;
  description: string;
}

export interface OrderOf {
  order_id: string;
  status: string;
  sku: string;
}

export type FlowResult = { ok: true; body: Record<string, unknown> } | { ok: false; error: string; status: number };

const refused = (error: string, status = 409): FlowResult => ({ ok: false, error, status });
const STEAM_UNAVAILABLE = 502;

// Steam statuses that mean the money was taken.
export const PAID = ["Approved", "Succeeded"];
// Steam statuses that reverse a finalized order. A partial refund is not reversed
// automatically: support decides it (PARTIAL).
export const REVERSED: Record<string, "refunded" | "charged_back"> = {
  Refunded: "refunded",
  RefundedSuspectedFraud: "refunded",
  RefundedFriendlyFraud: "refunded",
  Chargedback: "charged_back",
};
export const PARTIAL = "PartialRefund";

// A call to an api.srv_* function: the service client in edge functions, SQL in the worker.
export type Rpc = (fn: string, args: Record<string, unknown>) => Promise<Json>;

export function clientRpc(client: Client): Rpc {
  return async (fn, args) => {
    // The generated types know every function; the flow calls them by name.
    const call = client.rpc as unknown as (
      f: string,
      a: Record<string, unknown>,
    ) => Promise<{ data: Json; error: { message?: string } | null }>;
    const { data, error } = await call(fn, args);
    if (error) throw new FlowError(dbErrorCode(error));
    return data;
  };
}

async function rpc<T>(call: Rpc, fn: string, args: Record<string, unknown>): Promise<T> {
  return await call(fn, args) as T;
}

export class FlowError extends Error {}

// Opens an order and asks Steam to show the overlay. The client then gets Steam's
// authorization callback and calls finalize.
export async function initPurchase(
  client: Rpc,
  steam: SteamStore,
  account: string,
  sku: string,
): Promise<FlowResult> {
  let steamId: string | null;
  try {
    steamId = await rpc<string | null>(client, "srv_steam_id", { p_account: account });
  } catch (e) {
    return refused(e instanceof FlowError ? e.message : "internal_error");
  }
  if (!steamId) return refused("no_steam_identity", 403);
  let currency: string;
  try {
    currency = await steam.userCurrency(steamId);
  } catch (e) {
    return refused(e instanceof SteamError ? `steam_${e.code}` : "steam_unavailable", STEAM_UNAVAILABLE);
  }
  let order: CreatedOrder;
  try {
    order = jsonObject<CreatedOrder>(
      await rpc<Json>(client, "srv_purchase_create", { p_account: account, p_sku: sku, p_currency: currency }),
    )!;
  } catch (e) {
    return refused(e instanceof FlowError ? e.message : "internal_error");
  }
  try {
    const { transId } = await steam.initTxn(order.order_id, steamId, currency, {
      itemId: order.item_id,
      amountCents: order.amount,
      description: order.description,
    });
    await rpc(client, "srv_purchase_initiated", { p_order: order.order_id, p_trans: transId });
  } catch (e) {
    const reason = e instanceof SteamError ? `steam_${e.code}` : "steam_unavailable";
    // If this fails too, the worker's sweep closes the order later.
    await rpc(client, "srv_purchase_failed", { p_order: order.order_id, p_status: "failed", p_reason: reason })
      .catch(() => {});
    return refused(reason, STEAM_UNAVAILABLE);
  }
  return { ok: true, body: { order_id: order.order_id, amount: order.amount, currency } };
}

// Finalizes with Steam, then grants in one database transaction. Safe to retry: an order Steam
// already finalized is confirmed with QueryTxn, and the grant happens once.
export async function completeOrder(client: Rpc, steam: SteamStore, orderId: string): Promise<FlowResult> {
  let transId: string;
  try {
    transId = (await steam.finalizeTxn(orderId)).transId;
  } catch (e) {
    let state;
    try {
      state = await steam.queryTxn(orderId);
    } catch {
      return refused(e instanceof SteamError ? `steam_${e.code}` : "steam_unavailable", STEAM_UNAVAILABLE);
    }
    if (state.status !== "Succeeded") return refused(`steam_${state.status.toLowerCase()}`, STEAM_UNAVAILABLE);
    transId = state.transId;
  }
  try {
    const granted = await rpc<Json>(client, "srv_purchase_finalize", { p_order: orderId, p_trans: transId });
    return { ok: true, body: jsonObject<Record<string, unknown>>(granted) ?? {} };
  } catch (e) {
    return refused(e instanceof FlowError ? e.message : "internal_error");
  }
}

export async function finalizePurchase(
  client: Rpc,
  steam: SteamStore,
  account: string,
  orderId: string,
  authorized: boolean,
): Promise<FlowResult> {
  let order: OrderOf | null;
  try {
    order = jsonObject<OrderOf>(await rpc<Json>(client, "srv_purchase_of", { p_order: orderId, p_account: account }));
  } catch (e) {
    return refused(e instanceof FlowError ? e.message : "internal_error");
  }
  if (!order) return refused("unknown_order", 404);
  if (order.status === "finalized") return { ok: true, body: { status: "finalized", duplicate: true } };
  if (order.status !== "initiated") return refused(`order_${order.status}`);
  if (!authorized) {
    try {
      await rpc(client, "srv_purchase_failed", { p_order: orderId, p_status: "cancelled", p_reason: "declined" });
    } catch (e) {
      return refused(e instanceof FlowError ? e.message : "internal_error");
    }
    return { ok: true, body: { status: "cancelled" } };
  }
  return await completeOrder(client, steam, orderId);
}
