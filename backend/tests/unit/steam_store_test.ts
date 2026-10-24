import { assertEquals, assertRejects } from "jsr:@std/assert@1.0.13";
import { SteamError, SteamWebStore } from "../../supabase/functions/_shared/steam_store.ts";

interface Seen {
  url: URL;
  method: string;
  body: URLSearchParams | null;
}

function fake(responses: unknown[], seen: Seen[], status = 200): typeof fetch {
  return ((input: string | URL | Request, init?: RequestInit) => {
    const url = new URL(input instanceof Request ? input.url : input);
    const body = init?.body instanceof URLSearchParams ? init.body : null;
    seen.push({ url, method: init?.method ?? "GET", body });
    return Promise.resolve(new Response(JSON.stringify(responses.shift()), { status }));
  }) as typeof fetch;
}

const ok = (params: unknown) => ({ response: { result: "OK", params } });

Deno.test("the sandbox interface unless live, and InitTxn's form", async () => {
  const seen: Seen[] = [];
  const store = new SteamWebStore({
    appId: "480",
    publisherKey: "k",
    sandbox: true,
    fetchImpl: fake([ok({ orderid: "100000001", transid: "777" })], seen),
  });
  const r = await store.initTxn("100000001", "76561198000000001", "EUR", {
    itemId: 2,
    amountCents: 499,
    description: "Sponsor credit, 500",
  });
  assertEquals(r, { transId: "777" });
  assertEquals(seen[0].url.pathname, "/ISteamMicroTxnSandbox/InitTxn/v3/");
  assertEquals(seen[0].method, "POST");
  assertEquals(seen[0].body?.get("amount[0]"), "499");
  assertEquals(seen[0].body?.get("currency"), "EUR");
  assertEquals(seen[0].body?.get("usersession"), "client");
  const live = new SteamWebStore({
    appId: "480",
    publisherKey: "k",
    sandbox: false,
    fetchImpl: fake([ok({ transid: "1" })], seen),
  });
  await live.finalizeTxn("1");
  assertEquals(seen[1].url.pathname, "/ISteamMicroTxn/FinalizeTxn/v2/");
});

Deno.test("errors carry Steam's code", async () => {
  const seen: Seen[] = [];
  const store = new SteamWebStore({
    appId: "480",
    publisherKey: "k",
    sandbox: true,
    fetchImpl: fake(
      [{ response: { result: "Failure", error: { errorcode: 100, errordesc: "Insufficient funds" } } }],
      seen,
    ),
  });
  const e = await assertRejects(() => store.finalizeTxn("1"), SteamError);
  assertEquals(e.code, "100");
  const down = new SteamWebStore({ appId: "480", publisherKey: "k", sandbox: true, fetchImpl: fake([{}], seen, 503) });
  assertEquals((await assertRejects(() => down.queryTxn("1"), SteamError)).code, "http_503");
});

Deno.test("the report lists orders with their status and time", async () => {
  const seen: Seen[] = [];
  const store = new SteamWebStore({
    appId: "480",
    publisherKey: "k",
    sandbox: true,
    fetchImpl: fake([
      ok({ count: 1, orders: [{ orderid: 5, transid: 6, status: "Refunded", time: "2026-10-24T10:00:00Z" }] }),
    ], seen),
  });
  const orders = await store.report(new Date("2026-10-24T09:00:00.123Z"), 1000);
  assertEquals(orders, [{ orderId: "5", transId: "6", status: "Refunded", time: "2026-10-24T10:00:00Z" }]);
  assertEquals(seen[0].url.searchParams.get("time"), "2026-10-24T09:00:00Z");
  assertEquals(seen[0].url.searchParams.get("type"), "GAMESALES");
});
