# KINETIC backend

Phases 1 and 2 (rewards) of [docs/tech/backend-architecture.md](../docs/tech/backend-architecture.md): Supabase (Postgres, Auth, Realtime, Storage, Edge Functions) plus a TypeScript worker for matchmaking, server allocation and match rewards. Match reports are stored, validated and turned into Tonnage, the Ledger (Grade, the Weight, Clear, the Cutter ceremony, entries), Rope Marks, career totals, Tonnage credit and match summaries. Players own items, set loadouts (re-checked when each match forms and at each reset) and buy shop cosmetics with credit. Claim seasons run on their own: Allegiance, weekly Contracts with their rewards, Standing points under the weekly cap, hourly faction standing, the freeze and the tally; a person reviews and resolves each Claim. Rated matches update an OpenSkill Office Rating per mode; players see only its band. Each Claim's Manifest pays crates by Tonnage earned during it, Free to all and Sponsored to sponsors.

Phase 3 adds real money through Steam microtransactions (sponsor credit and Sponsorship, finalize, and refund and chargeback reversal from Steam's report), player reports and filtered name changes, account export and deletion with a 14-day grace, storage retention, a load simulator and runbooks ([docs/tech/runbooks.md](../docs/tech/runbooks.md)).

## What is here

| Path | What it does |
|---|---|
| `supabase/migrations/` | Schemas (`api`, `game`, `private`, `admin`), tables, RLS, the rate limiter, every client and server function, Realtime Authorization, Storage buckets |
| `supabase/functions/auth-steam` | Steam ticket (or dev name, local only) to a Supabase session |
| `supabase/functions/match-current` | Reconnect: a fresh join ticket for the player's running match, inside the reconnect window |
| `supabase/functions/server-*` | Game-server endpoints: manifest, heartbeat, player events, loadouts at each reset, upload URLs, match report. Authenticated with a per-match credential, never a Supabase key, and rate-limited per match |
| `supabase/functions/store-steam-init`, `store-steam-finalize` | A Steam purchase: open the order and InitTxn (Steam shows its overlay), then FinalizeTxn and grant in one transaction. Sandbox unless `STEAM_MICROTXN_LIVE=true` |
| `supabase/functions/_shared/` | Tokens (Ed25519 join tickets and server credentials), Steam client, HTTP and Supabase helpers |
| `worker/` | Matchmaker (pure rules in `matchmaker.ts`), the tick loop (allocation, `match_found`, timeouts, requeue, rewards, deallocation, cleanup), leader election, fleet adapters |
| `worker/rewards.ts`, `worker/rewards_pass.ts` | Facts to rewards (pure, golden-tested) and the pass that applies each stored report in one transaction. It quarantines malformed or over-ceiling facts and sends `match_processed` on the player's `user:` channel. Failures back off; only a failure the report causes (a data or constraint error) counts, and five of those quarantine it. An outage stops the pass without counting. `api.srv_requeue_report` puts a quarantined report back |
| `worker/claims.ts`, `worker/claims_progress.ts`, `worker/claims_pass.ts` | Claim rules (pure: Standing points, the cap, Contracts, S_f, the holder and each arena's next variant), the per-match Standing and Contract step inside the rewards transaction, and the pass that activates, freezes and tallies Claims and refreshes standing hourly. `api.srv_claim_review` and `api.srv_claim_resolve` are the admin console's steps |
| `worker/rating.ts`, `worker/ratings_step.ts` | The Office Rating: OpenSkill's Weng-Lin Bradley-Terry update (pure, checked against OpenSkill's reference values), with a forfeit costing a loss plus 20% and a leaver taking the full loss; and the step that rates each Rated match's roster in the rewards transaction |
| `worker/manifest_step.ts` | The Manifest payout in the rewards transaction: Tonnage toward the Claim's crates, Free and Sponsored |
| `worker/purchases_pass.ts` | Checks orders the client never finalized (asking Steam, each on its own backoff), finalizes paid ones, closes dead ones, and reverses refunds and chargebacks from Steam's report: credit back as far as the balance allows, Sponsorship removed, support flagged. Partial refunds go to support |
| `worker/accounts_pass.ts`, `worker/storage.ts` | Export archives to the private `exports` bucket (kept 7 days), deletions after their grace (match rows and report payloads anonymized, the Auth trail removed, purchases kept, active sanctions retained by identity hash), and retention of replays (30 days) and raw events (90) unless a report still needs them. Jobs left running are reclaimed and failures retried |
| `tools/load-sim.ts` | `deno task load-sim --players N`: N dev clients through login, crews, queue, match, join and rewards against a running stack; prints p50, p95 and max per step |
| `tools/admin.ts` | Admin console v1, `deno task admin --staff <name> <command>`: create Claims, Contracts and Manifest crates, review and resolve Claims (with a tie override), requeue reports, grant items and entitlements, sanction, read the audit. Each command is one transaction under the staff member's name, which must be listed in `admin.staff` (the database owner adds staff: `insert into admin.staff (staff_id) values ('name')`); triggers write every change to the append-only `admin.audit` |
| `ruleset/` | The economy, one JSON file per version (`1.json`; every number **PROPOSED**). The required `ruleset` key in `game.config` picks the version in force; without it the worker rewards nothing. Never edit a version that has rewarded a match: add the next one |
| `catalog/items.json`, `tools/load-content.ts` | The item catalog (placeholder until the content build exports it) and the deploy step, `deno task content`, which loads the catalog, the current shop rotation and copies of the rulesets for clients (`api.current_ruleset`). It refuses a catalog that sells or ships a sidegrade, and a changed copy of a loaded ruleset |
| `tools/fake-server.ts` | Stands in for the Unreal dedicated server until it exists: same backend contract |
| `tests/` | Unit tests, pgTAP database tests (`supabase/tests/database/`), and a full-loop integration test |

## Run it on a Mac

Needs Docker, the Supabase CLI and Deno.

```bash
cd backend
```

```bash
deno task keys
```

```bash
supabase start
```

```bash
deno task functions
```

```bash
deno task content
```

```bash
deno task worker
```

Local ports are 554xx (API on 55421, database on 55422, Studio on 55423) so they do not clash with other local Supabase projects. `supabase db reset` restarts the stack and stops `deno task functions`; start it again afterwards. Auth settings in `config.toml` take effect only after `supabase stop` and `supabase start`.

## Tests

```bash
deno task test:unit
```

```bash
deno task test:db
```

```bash
deno task test:worker-db
```

```bash
deno task test:integration
```

`test:worker-db` needs `supabase start` and `deno task content`: it runs the rewards and claims passes against the local database. The Claims test needs no other Claim active locally. The unit tests include a golden file (`tests/fixtures/sample-report.rewards.v1.json`): the engine-free core's sample match report, rewarded under ruleset 1. Regenerate it with `UPDATE_GOLDEN=1 deno task test:unit` only for a deliberate change, and review the diff.

The integration test needs `supabase start` and `deno task functions` running, and `deno task worker` stopped (the test takes the leader lock itself). It checks that public signup is refused, logs in six dev players (including two simultaneous logins), forms two crews, sends crew chat, queues, runs the worker tick, boots the fake server through the local fleet, receives `match_found` over Realtime, joins with tickets, and checks the uploads and that the report was stored and rewarded.

## Code style

The code follows the `coding-standards` skill in [.claude/skills/coding-standards](../.claude/skills/coding-standards/SKILL.md): no `any` (lint enforces it; the Supabase client is typed from `supabase/functions/_shared/database.types.ts`, regenerated with `deno task types` after any migration), no mutation of shared values, named constants instead of magic numbers, functions under about 50 lines with early returns, and independent async work run in parallel.

## Rules the code keeps

- Only the `api` schema is exposed. Clients read through RLS-protected views and write through `api` functions that check the caller and rate-limit.
- `srv_*` functions are granted to `service_role` only; nothing is granted to `PUBLIC`.
- Public signup is off. An Auth user is adopted for a Steam or dev identity only if `auth-steam` created it for that exact identity (its `app_metadata`).
- Game servers hold no Supabase key. They use a per-match credential signed by the KINETIC key; each allocation attempt gets a new allocation id, so a stale server loses access.
- Game-server processes start with a clean environment: they never see the signing key, the service key or the database URL.
- `crew:<id>` and `user:<id>` Realtime topics are server-only; members talk on `crew-chat:<id>`.
- The Steam publisher key and the signing keys live in function secrets (`supabase/functions/.env` locally, never committed).

## Not done yet

- The Edgegap fleet adapter (`worker/fleet/edgegap.ts`) waits for the fleet decision.
- Production worker database role: locally the worker connects as `postgres`; production needs a dedicated role with only the grants it uses, over a direct or session-mode connection (the leader lock needs a session).
- Banning an account in Supabase Auth itself (so sessions stop refreshing) belongs to the admin console; today an `account_ban` sanction blocks every `api` function and login at once.
- Still to come: matchmaking by Office Rating in the Rated pool, the voice service (`voice-token`, once a vendor is chosen), a client anti-cheat, evidence objects (+5 P once per Claim; the event store holds no evidence fact yet), the forfeit and leaver Ledger lines of game-modes 8 (such as "Walked off a claim"), and a web admin console with staff sign-in and MFA (v1 is the command-line tool over the service connection). Private matches (Rope Mark practice only) wait for a private playlist. Bots do not yet inherit a leaver's loadout.
