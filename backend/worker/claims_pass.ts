// The worker's claims pass (backend-architecture 14): moves each Claim along
// scheduled -> active -> frozen -> tallied, and refreshes faction standing at most hourly while
// a Claim is active. A report counts for the Claim running when it was stored, so a frozen
// Claim waits for the rewards pass to process those before it is tallied. Reviewed and resolved are a person's steps (api.srv_claim_review and
// api.srv_claim_resolve, from the admin console).
//
// The tally's anomaly list (PROPOSED) names accounts at the weekly cap in every week they
// played and counts the reports quarantined during the Claim, for the reviewer to look at.
import type postgres from "npm:postgres@3.4.5";
import type { WorkerContext } from "./loop.ts";
import { arenaVariant, type Faction, type FactionTotals, holder, standings, WEEKLY_CAP_P } from "./claims.ts";

export const STANDING_REFRESH_S = 3600;
// A frozen Claim waits this long for its last reports, then is tallied anyway; reports still
// unprocessed are listed for the reviewer.
export const TALLY_GRACE_S = 24 * 3600;

type Sql = postgres.Sql | postgres.TransactionSql;

async function factionTotals(sql: Sql, claimId: string): Promise<FactionTotals[]> {
  const rows = await sql<{ faction: Faction; points: string; active_players: number; rated_wins: number }[]>`
    with per_player as (
      select faction, account_id, sum(points) as points, count(*) filter (where rated_win) as rated_wins
        from game.claim_standing_events where claim_id = ${claimId}
       group by faction, account_id)
    select faction, coalesce(sum(points), 0)::text as points,
           (count(*) filter (where account_id is not null and points >= 1))::int as active_players,
           coalesce(sum(rated_wins), 0)::int as rated_wins
      from per_player group by faction`;
  return rows.map((r) => ({ ...r, points: Number(r.points) }));
}

export async function refreshStanding(sql: Sql, claimId: string) {
  const result = standings(await factionTotals(sql, claimId));
  for (const s of result) {
    await sql`
      insert into game.claim_standing (claim_id, faction, points_total, active_players, standing, rated_wins,
                                       refreshed_at)
      values (${claimId}, ${s.faction}, ${s.points}, ${s.active_players}, ${s.standing}, ${s.rated_wins}, now())
      on conflict (claim_id, faction) do update set
        points_total = excluded.points_total, active_players = excluded.active_players,
        standing = excluded.standing, rated_wins = excluded.rated_wins, refreshed_at = now()`;
  }
  return result;
}

async function anomalies(sql: Sql, claimId: string) {
  const leftover = await sql<{ match_id: string }[]>`
    select r.match_id from game.match_reports r, game.claims c
     where c.claim_id = ${claimId} and r.status = 'stored' and r.received_at >= c.starts_at
       and r.received_at < c.ends_at order by r.received_at`;
  const capped = await sql<{ account_id: string }[]>`
    with weekly as (
      select account_id, week, sum(points) as points from game.claim_standing_events
       where claim_id = ${claimId} and account_id is not null group by account_id, week)
    select account_id from weekly group by account_id
    having count(*) >= 2 and bool_and(points >= ${WEEKLY_CAP_P}) order by account_id`;
  const [quarantined] = await sql<{ n: number }[]>`
    select count(*)::int as n from game.match_reports r, game.claims c
     where c.claim_id = ${claimId} and r.status = 'quarantined' and r.received_at >= c.starts_at
       and r.received_at < c.ends_at + interval '1 hour'`;
  return {
    capped_every_week: capped.map((r) => r.account_id),
    quarantined_reports: quarantined.n,
    unprocessed_reports: leftover.map((r) => r.match_id),
  };
}

export async function tallyClaim(sql: postgres.Sql, claim: { claim_id: string; arena: string }) {
  await sql.begin(async (tx) => {
    const final = await refreshStanding(tx, claim.claim_id);
    const winner = holder(final);
    const outcome = {
      holder: winner,
      variant: winner ? arenaVariant(claim.arena, winner) : null,
      standings: final.map((s) => ({ faction: s.faction, standing: s.standing, rated_wins: s.rated_wins })),
    };
    // The reviewer's notes, kept out of the outcome players read.
    const review = { anomalies: await anomalies(tx, claim.claim_id) };
    await tx`
      update game.claims
         set state = 'tallied', outcome = ${tx.json(outcome as unknown as postgres.JSONValue)},
             review = ${tx.json(review as unknown as postgres.JSONValue)}
       where claim_id = ${claim.claim_id} and state = 'frozen'`;
  });
}

export async function claimsPass(ctx: WorkerContext): Promise<void> {
  // Freeze first, so a Claim ending as the next starts never leaves two active.
  await ctx.sql`update game.claims set state = 'frozen' where state = 'active' and now() >= ends_at`;
  await ctx.sql`
    update game.claims set state = 'active'
     where claim_id = (select claim_id from game.claims
                        where state = 'scheduled' and now() >= starts_at and now() < ends_at
                        order by starts_at limit 1)
       and not exists (select 1 from game.claims where state = 'active')`;
  // A frozen Claim is tallied once every report stored before it ended has been processed.
  const frozen = await ctx.sql<{ claim_id: string; arena: string }[]>`
    select c.claim_id, c.arena from game.claims c
     where c.state = 'frozen'
       and (now() >= c.ends_at + make_interval(secs => ${TALLY_GRACE_S})
            or not exists (select 1 from game.match_reports r
                            where r.status = 'stored' and r.received_at >= c.starts_at and r.received_at < c.ends_at))
     order by c.ends_at`;
  for (const claim of frozen) await tallyClaim(ctx.sql, claim);
  const stale = await ctx.sql<{ claim_id: string }[]>`
    select c.claim_id from game.claims c
     where c.state = 'active'
       and coalesce((select min(s.refreshed_at) from game.claim_standing s where s.claim_id = c.claim_id), '-infinity')
           < now() - make_interval(secs => ${STANDING_REFRESH_S})`;
  for (const claim of stale) await refreshStanding(ctx.sql, claim.claim_id);
}
