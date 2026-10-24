import { assertEquals, assertRejects } from "jsr:@std/assert@1.0.13";
import { STEAM_IDENTITY, verifySteamTicket } from "../../supabase/functions/_shared/steam.ts";

const TICKET = "14000000a1b2c3d4e5f60718293a4b5c";

function fakeFetch(body: unknown, status = 200, seen?: URL[]): typeof fetch {
  return ((input: string | URL | Request) => {
    seen?.push(new URL(input instanceof Request ? input.url : input));
    return Promise.resolve(new Response(JSON.stringify(body), { status }));
  }) as typeof fetch;
}

Deno.test("accepts an OK response and sends the identity", async () => {
  const seen: URL[] = [];
  const user = await verifySteamTicket(TICKET, {
    appId: "480",
    publisherKey: "key",
    fetchImpl: fakeFetch(
      { response: { params: { result: "OK", steamid: "76561198000000001", vacbanned: false, publisherbanned: true } } },
      200,
      seen,
    ),
  });
  assertEquals(user, { steamId: "76561198000000001", vacBanned: false, publisherBanned: true });
  assertEquals(seen[0].searchParams.get("identity"), STEAM_IDENTITY);
  assertEquals(seen[0].searchParams.get("appid"), "480");
});

Deno.test("rejects an error response", async () => {
  await assertRejects(() =>
    verifySteamTicket(TICKET, {
      appId: "480",
      publisherKey: "key",
      fetchImpl: fakeFetch({ response: { error: { errorcode: 101, errordesc: "Invalid ticket" } } }),
    })
  );
});

Deno.test("rejects an HTTP failure", async () => {
  await assertRejects(() =>
    verifySteamTicket(TICKET, { appId: "480", publisherKey: "key", fetchImpl: fakeFetch({}, 503) })
  );
});

Deno.test("rejects a malformed ticket without calling Steam", async () => {
  const seen: URL[] = [];
  await assertRejects(() =>
    verifySteamTicket("not hex!", { appId: "480", publisherKey: "key", fetchImpl: fakeFetch({}, 200, seen) })
  );
  assertEquals(seen.length, 0);
});
