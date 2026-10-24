// Per match, inside the rewards transaction (backend-architecture 12.3 steps 4 and 6): each
// earner with an Allegiance in the active Claim advances this week's Contracts for their
// faction, collects completed Contracts' rewards, and earns Standing points under the weekly
// cap. Bots and players who left never reach here (rewards.ts earners), and a voided match
// (forfeit, leaver, no-contest) pays 0 P.
import type postgres from "npm:postgres@3.4.5";
import {
  advanceContracts,
  capped,
  claimWeek,
  CONTRACT_P,
  type ContractUpdate,
  type Faction,
  matchPoints,
  type MatchResult,
  rewardLists,
} from "./claims.ts";

type Tx = postgres.TransactionSql;

export interface ClaimContext {
  claim_id: string;
  starts_at: Date;
  week: number;
  // When the server stored the report: the Claim, the week and the faction are as of then.
  received_at: Date;
}

export interface ClaimOutcome {
  faction: Faction;
  points: number;
  contracts: ContractUpdate[];
}

// The Claim a report counts for: the one running when the server stored it, still open to
// late processing while frozen (the tally waits for those reports).
export async function claimFor(tx: Tx, receivedAt: Date): Promise<ClaimContext | null> {
  const [claim] = await tx<{ claim_id: string; starts_at: Date }[]>`
    select claim_id, starts_at from game.claims
     where state in ('active', 'frozen') and ${receivedAt} >= starts_at and ${receivedAt} < ends_at
     order by starts_at limit 1`;
  return claim ? { ...claim, week: claimWeek(claim.starts_at, receivedAt), received_at: receivedAt } : null;
}

async function grantContract(tx: Tx, account: string, contractId: string): Promise<void> {
  const [contract] = await tx<{ reward: unknown }[]>`
    select reward from game.contracts where contract_id = ${contractId}`;
  const { items, entries } = rewardLists(contract?.reward);
  if (items.length > 0) {
    await tx`
      insert into game.inventory ${
      tx(items.map((item_id) => ({ account_id: account, item_id, source: "contract", source_ref: contractId })))
    } on conflict do nothing`;
  }
  if (entries.length > 0) {
    await tx`
      insert into game.ledger_entries ${tx(entries.map((entry_id) => ({ account_id: account, entry_id })))}
      on conflict do nothing`;
  }
}

async function earnedThisWeek(tx: Tx, account: string, claim: ClaimContext): Promise<number> {
  const [row] = await tx<{ points: string }[]>`
    select coalesce(sum(points), 0)::text as points from game.claim_standing_events
     where claim_id = ${claim.claim_id} and account_id = ${account} and week = ${claim.week}`;
  return Number(row.points);
}

async function addStanding(
  tx: Tx,
  account: string,
  claim: ClaimContext,
  faction: Faction,
  points: number,
  reason: { match_id?: string; contract_id?: string; rated_win?: boolean },
): Promise<number> {
  const granted = capped(points, await earnedThisWeek(tx, account, claim));
  const kind = reason.contract_id ? "contract" : "match";
  // Recorded even at 0 P, so a Rated win past the cap still counts for the tie-break.
  await tx`
    insert into game.claim_standing_events (claim_id, faction, points, reason, match_id, contract_id, account_id,
                                            week, rated_win)
    values (${claim.claim_id}, ${faction}, ${granted}, ${kind}, ${reason.match_id ?? null},
            ${reason.contract_id ?? null}, ${account}, ${claim.week}, ${reason.rated_win ?? false})
    on conflict do nothing`;
  return granted;
}

async function advance(
  tx: Tx,
  account: string,
  claim: ClaimContext,
  faction: Faction,
  counters: Record<string, number>,
) {
  const contracts = await tx<{ contract_id: string; counter_key: string; target: string }[]>`
    select contract_id, counter_key, target::text from game.contracts
     where claim_id = ${claim.claim_id} and faction = ${faction} and week = ${claim.week}`;
  if (contracts.length === 0) return [];
  const rows = await tx<{ contract_id: string; progress: string; completed: boolean }[]>`
    select contract_id, progress::text, completed_at is not null as completed from game.contract_progress
     where account_id = ${account} and contract_id in ${tx(contracts.map((c) => c.contract_id))}
     for update`;
  const progress = new Map(rows.map((r) => [r.contract_id, { progress: Number(r.progress), completed: r.completed }]));
  const updates = advanceContracts(contracts.map((c) => ({ ...c, target: Number(c.target) })), progress, counters);
  for (const u of updates) {
    await tx`
      insert into game.contract_progress (account_id, contract_id, progress, completed_at)
      values (${account}, ${u.contract_id}, ${u.progress}, ${u.completed ? new Date() : null})
      on conflict (account_id, contract_id) do update
        set progress = excluded.progress, completed_at = excluded.completed_at`;
  }
  return updates;
}

export async function applyClaimProgress(
  tx: Tx,
  claim: ClaimContext,
  account: string,
  matchId: string,
  result: MatchResult,
  counters: Record<string, number>,
): Promise<ClaimOutcome | null> {
  // The faction the player held when the report was stored, not the one they hold now.
  const [held] = await tx<{ faction: Faction; current: Faction | null }[]>`
    select h.faction, (select a.faction from game.allegiances a
                        where a.account_id = h.account_id and a.claim_id = h.claim_id) as current
      from game.allegiance_history h
     where h.account_id = ${account} and h.claim_id = ${claim.claim_id} and h.declared_at <= ${claim.received_at}
     order by h.declared_at desc limit 1`;
  if (!held) return null;
  const faction = held.faction;
  // A voided match does not count: no Standing and no Contract progress.
  if (result.voided) return { faction, points: 0, contracts: [] };
  // Contracts only for the faction still held: switching reset the old faction's progress.
  const contracts = held.current === faction ? await advance(tx, account, claim, faction, counters) : [];
  const ratedWin = result.rated && result.win;
  let points = await addStanding(tx, account, claim, faction, matchPoints(result), {
    match_id: matchId,
    rated_win: ratedWin,
  });
  for (const done of contracts.filter((c) => c.completed)) {
    await grantContract(tx, account, done.contract_id);
    points += await addStanding(tx, account, claim, faction, CONTRACT_P, { contract_id: done.contract_id });
  }
  return { faction, points, contracts };
}
