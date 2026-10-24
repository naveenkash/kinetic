// Per Rated match, inside the rewards transaction (backend-architecture 12.3 step 7): every
// human on the roster with an account, leavers included, gets a new Office Rating for the
// mode. Bots are not rated and do not count toward their crew's skill. A no-contest changes
// nothing. A penalty-free forfeit (a short crew after 60 s, game-modes 8) leaves the
// forfeiting crew's Ratings alone (PROPOSED: "without penalty" read as no change at all),
// except the leaver's, which takes the full loss; the other crew is rated as winners.
import type postgres from "npm:postgres@3.4.5";
import { type Band, band, MU0, rate, type RatedPlayer, SIGMA0 } from "./rating.ts";
import type { Report } from "./rewards.ts";

type Tx = postgres.TransactionSql;

export interface RatingChange {
  band_before: Band;
  band: Band;
  games: number;
}

function ranks(report: Report): Record<number, number> {
  const teams = [...new Set(report.players.map((p) => p.team))];
  // No winner and no no-contest is a draw: every crew shares first place.
  return Object.fromEntries(
    teams.map((t) => [t, report.winner_team === null || report.winner_team === t ? 1 : 2]),
  );
}

export async function rateMatch(tx: Tx, mode: string, report: Report): Promise<Map<string, RatingChange>> {
  const humans = report.players.filter((p) => !p.is_bot && p.account_id !== null);
  if (report.no_contest || humans.length === 0) return new Map();
  const accounts = humans.map((p) => p.account_id!);
  // Key-share every rated account first, leavers included, so a deletion waits for this pass.
  await tx`select account_id from game.accounts where account_id in ${tx(accounts)}
           order by account_id for key share`;
  const rows = await tx<{ account_id: string; mu: number; sigma: number; games: number }[]>`
    select r.account_id, r.mu, r.sigma, r.games from game.ratings r
      join game.accounts a using (account_id)
     where r.mode = ${mode} and r.account_id in ${tx(accounts)}
     order by r.account_id for update of r`;
  const live = await tx<{ account_id: string }[]>`
    select account_id from game.accounts where account_id in ${tx(accounts)}`;
  const known = new Set(live.map((r) => r.account_id));
  const players: RatedPlayer[] = humans.filter((p) => known.has(p.account_id!)).map((p) => {
    const row = rows.find((r) => r.account_id === p.account_id);
    return { key: p.account_id!, team: p.team, abandoned: p.abandoned, rating: row ?? { mu: MU0, sigma: SIGMA0 } };
  });
  const next = rate(players, { rank: ranks(report), forfeit_team: report.forfeit_team });
  const changes = new Map<string, RatingChange>();
  // The leaver who made the crew short still takes the full loss (game-modes 8).
  const spared = (p: RatedPlayer) => report.forfeit_penalty_free && p.team === report.forfeit_team && !p.abandoned;
  for (const p of players.filter((x) => !spared(x))) {
    const before = rows.find((r) => r.account_id === p.key);
    const games = (before?.games ?? 0) + 1;
    const r = next.get(p.key)!;
    const change = { band_before: band(p.rating, games - 1), band: band(r, games), games };
    await tx`
      insert into game.ratings (account_id, mode, mu, sigma, games, band, updated_at)
      values (${p.key}, ${mode}, ${r.mu}, ${r.sigma}, ${games}, ${change.band}, now())
      on conflict (account_id, mode) do update
        set mu = excluded.mu, sigma = excluded.sigma, games = excluded.games, band = excluded.band, updated_at = now()`;
    changes.set(p.key, change);
  }
  return changes;
}
