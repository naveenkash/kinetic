// Steam microtransactions (backend-architecture 13.4): the ISteamMicroTxn web API, or
// ISteamMicroTxnSandbox in dev and staging. CHECK every endpoint version, parameter and status
// name against the Steamworks documentation before launch; the shapes below follow it as of
// writing.
//
// One interface for the edge functions and the worker, so tests can stand in a fake Steam.

const STEAM_TIMEOUT_MS = 8000;

export type SteamTxnStatus =
  | "Init"
  | "Approved"
  | "Succeeded"
  | "Failed"
  | "Refunded"
  | "PartialRefund"
  | "Chargedback"
  | "RefundedSuspectedFraud"
  | "RefundedFriendlyFraud";

export interface SteamOrder {
  orderId: string;
  transId: string;
  status: SteamTxnStatus;
  // When Steam last changed the order (GetReport), RFC 3339.
  time?: string;
}

export interface InitItem {
  itemId: number;
  amountCents: number;
  description: string;
}

export interface SteamStore {
  userCurrency(steamId: string): Promise<string>;
  initTxn(orderId: string, steamId: string, currency: string, item: InitItem): Promise<{ transId: string }>;
  finalizeTxn(orderId: string): Promise<{ transId: string }>;
  queryTxn(orderId: string): Promise<SteamOrder>;
  // Orders whose status changed since `since` (refunds and chargebacks among them).
  report(since: Date, maxResults: number): Promise<SteamOrder[]>;
}

export interface SteamStoreConfig {
  appId: string;
  publisherKey: string;
  sandbox: boolean;
  baseUrl?: string;
  fetchImpl?: typeof fetch;
}

export class SteamError extends Error {
  constructor(readonly code: string, message: string) {
    super(message);
  }
}

type Params = Record<string, string>;

export class SteamWebStore implements SteamStore {
  constructor(private readonly config: SteamStoreConfig) {}

  private url(method: string, version: number): URL {
    const iface = this.config.sandbox ? "ISteamMicroTxnSandbox" : "ISteamMicroTxn";
    return new URL(`${this.config.baseUrl ?? "https://partner.steam-api.com"}/${iface}/${method}/v${version}/`);
  }

  private async call(method: string, version: number, params: Params, post: boolean): Promise<Record<string, unknown>> {
    const url = this.url(method, version);
    const all = { key: this.config.publisherKey, appid: this.config.appId, ...params };
    const init: RequestInit = { signal: AbortSignal.timeout(STEAM_TIMEOUT_MS) };
    if (post) {
      init.method = "POST";
      init.body = new URLSearchParams(all);
    } else {
      for (const [k, v] of Object.entries(all)) url.searchParams.set(k, v);
    }
    const response = await (this.config.fetchImpl ?? fetch)(url, init);
    if (!response.ok) throw new SteamError(`http_${response.status}`, `${method}: HTTP ${response.status}`);
    const body = await response.json().catch(() => null);
    const r = body?.response;
    if (!r || r.result !== "OK" || typeof r.params !== "object") {
      const code = String(r?.error?.errorcode ?? "rejected");
      throw new SteamError(code, `${method}: ${r?.error?.errordesc ?? "rejected"}`);
    }
    return r.params as Record<string, unknown>;
  }

  async userCurrency(steamId: string): Promise<string> {
    const p = await this.call("GetUserInfo", 2, { steamid: steamId }, false);
    if (typeof p.currency !== "string") throw new SteamError("no_currency", "GetUserInfo: no currency");
    return p.currency;
  }

  async initTxn(orderId: string, steamId: string, currency: string, item: InitItem) {
    const p = await this.call("InitTxn", 3, {
      orderid: orderId,
      steamid: steamId,
      itemcount: "1",
      language: "en",
      currency,
      usersession: "client",
      "itemid[0]": String(item.itemId),
      "qty[0]": "1",
      "amount[0]": String(item.amountCents),
      "description[0]": item.description,
    }, true);
    return { transId: String(p.transid) };
  }

  async finalizeTxn(orderId: string) {
    const p = await this.call("FinalizeTxn", 2, { orderid: orderId }, true);
    return { transId: String(p.transid) };
  }

  async queryTxn(orderId: string): Promise<SteamOrder> {
    const p = await this.call("QueryTxn", 3, { orderid: orderId }, false);
    return { orderId: String(p.orderid), transId: String(p.transid), status: p.status as SteamTxnStatus };
  }

  async report(since: Date, maxResults: number): Promise<SteamOrder[]> {
    const p = await this.call("GetReport", 5, {
      type: "GAMESALES",
      time: since.toISOString().replace(/\.\d{3}Z$/, "Z"),
      maxresults: String(maxResults),
    }, false);
    const orders = Array.isArray(p.orders) ? p.orders as Record<string, unknown>[] : [];
    return orders.map((o) => ({
      orderId: String(o.orderid),
      transId: String(o.transid),
      status: o.status as SteamTxnStatus,
      time: typeof o.time === "string" ? o.time : undefined,
    }));
  }
}

export function steamStoreFromEnv(): SteamStore | null {
  const publisherKey = Deno.env.get("STEAM_PUBLISHER_KEY");
  const appId = Deno.env.get("STEAM_APP_ID");
  if (!publisherKey || !appId) return null;
  return new SteamWebStore({
    appId,
    publisherKey,
    // Sandbox unless explicitly live: a misconfigured environment must never charge anyone.
    sandbox: Deno.env.get("STEAM_MICROTXN_LIVE") !== "true",
    baseUrl: Deno.env.get("STEAM_API_BASE") || undefined,
  });
}
