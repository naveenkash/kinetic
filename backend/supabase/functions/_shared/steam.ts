// Steam web API ticket validation (backend-architecture section 8.1).
// The client gets the ticket from ISteamUser::GetAuthTicketForWebApi with the identity
// below. CHECK the endpoint and response fields against Steamworks docs before launch.

export const STEAM_IDENTITY = "kinetic-backend";
const STEAM_TIMEOUT_MS = 5000;

export interface SteamUser {
  steamId: string;
  vacBanned: boolean;
  publisherBanned: boolean;
}

export interface SteamConfig {
  appId: string;
  publisherKey: string;
  fetchImpl?: typeof fetch;
}

export async function verifySteamTicket(ticket: string, config: SteamConfig): Promise<SteamUser> {
  if (!/^[0-9a-fA-F]{16,4096}$/.test(ticket)) {
    throw new Error("malformed_ticket");
  }
  const url = new URL("https://partner.steam-api.com/ISteamUserAuth/AuthenticateUserTicket/v1/");
  url.searchParams.set("key", config.publisherKey);
  url.searchParams.set("appid", config.appId);
  url.searchParams.set("ticket", ticket);
  url.searchParams.set("identity", STEAM_IDENTITY);

  const response = await (config.fetchImpl ?? fetch)(url, { signal: AbortSignal.timeout(STEAM_TIMEOUT_MS) });
  if (!response.ok) {
    throw new Error(`steam_http_${response.status}`);
  }
  const body = await response.json();
  const params = body?.response?.params;
  if (!params || params.result !== "OK" || typeof params.steamid !== "string") {
    throw new Error("steam_rejected");
  }
  return {
    steamId: params.steamid,
    vacBanned: params.vacbanned === true,
    publisherBanned: params.publisherbanned === true,
  };
}
