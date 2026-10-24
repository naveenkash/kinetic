# KINETIC: Backend Runbooks

What to do when the backend needs a person. Each runbook names the signal, the check and the fix. Commands are run from `backend/`. `deno task admin --staff <you> ...` is the admin console ([backend-architecture.md](backend-architecture.md) section 17). It records its changes in `admin.audit` under your name, which must be listed in `admin.staff`. Changes made with raw SQL that names no staff member are not audited on the per-match tables, so make changes through the console.

## 1. Match reports are not being rewarded

**Signal:** `match_reports` rows stay in `stored`, or players say their Ledger did not move after a match.

**Check:**

```sql
select status, count(*), max(failures), min(received_at) from game.match_reports group by status;
select match_id, failures, process_attempts, next_attempt_at, last_error
  from game.match_reports where status = 'stored' and failures > 0 order by received_at limit 20;
```

- **Many failures with one `last_error`:** this is systemic: a bad deploy, a missing migration, or a ruleset mismatch.
  - A message saying "ruleset … is not the copy clients read" means `deno task content` was not run after a ruleset change. Run it.
  - A message saying "the ruleset config key must be a version number" means the `ruleset` row in `game.config` is missing or malformed.
  - Fix the cause. Stored reports resume on their own when their backoff ends (at most an hour).
- **One report failing alone:** read its `last_error`. Five report-caused failures, or twenty of any kind, quarantine it.

**Fix a quarantined report** once the cause is fixed:

```bash
deno task admin --staff <you> report:requeue --match <match_id>
```

Never requeue a report quarantined for `fact_ceiling` or `malformed …` without reading its payload. Those are the reports a hostile or broken game server would send.

## 2. Quarantined reports (review queue)

`quarantine_reason` says why:

| Reason | Meaning | Action |
|---|---|---|
| `roster_mismatch`, `bot_mismatch`, `duplicate_player`, `malformed_player` | The server's roster disagrees with the match | Investigate the server build; do not requeue |
| `fact_ceiling: …` | A fact is over the ruleset's per-match ceiling | Look for a physics or reporting bug, or cheating. Raise the ceiling in a **new** ruleset version only if the fact is genuine |
| `match_aborted`, `match_not_started` | The match never ran properly | None; kept for audit |
| `processing_failed` | Repeated failures (see section 1) | Fix the cause, then requeue |

## 3. The worker is not leading

**Signal:** no matches form, and no reports are processed. The worker logs `waiting for leadership` on every instance, or nothing at all.

- Two instances run, and the one holding the advisory lock leads. Check the lock with `select * from pg_locks where locktype = 'advisory';`.
- A connection pooler in transaction mode drops session locks. `DATABASE_URL` must be a direct connection or a session-mode one.
- Restarting an instance is safe. Leadership passes within one tick of the old leader's connection closing.

## 4. A Claim (season) flip

1. **Before the Claim:** create it and its Contracts and crates from the console.

   ```bash
   deno task admin --staff <you> claim:create --id claim-15 --name "Claim 15" --arena BLS --starts 2027-01-04T00:00:00Z --ends 2027-02-01T00:00:00Z
   ```

   Then `contract:add` and `crate:add` for each Contract and crate. Claims may not overlap.
2. **At `starts_at`:** the worker activates it. At `ends_at` the worker freezes it, and tallies it once every report stored during it is processed, or 24 hours after it ended.
3. **Review:**

   ```sql
   select outcome, review from game.claims where claim_id = 'claim-15';
   ```

   Look at `review.anomalies`:
   - `capped_every_week`: accounts at the weekly cap in every week;
   - `quarantined_reports`;
   - `unprocessed_reports`.

   Then sign off:

   ```bash
   deno task admin --staff <you> claim:review --id claim-15
   ```

   For a tie with no holder, add `--holder <faction> --variant <variant>`.
4. **Resolve:** `claim:resolve --id claim-15` writes `variant:<arena>` into `game.config`. The patch carrying both outcomes must already be live on every platform ([backend-architecture.md](backend-architecture.md) section 14).
5. Rehearse the whole flip on staging every season.

## 5. Purchases, refunds and chargebacks

- **A player paid but got nothing:** check their purchases.

  ```sql
  select * from game.purchases where account_id = '<id>' order by created_at desc;
  ```

  An order still `initiated` is checked by the worker from 10 minutes after it opened, on a backoff of a minute, doubling to hourly (`checks`, `next_check_at`). It finalizes and grants if Steam took the money, and closes the order after a day if not, or if Steam does not know it. To check one now, set its `next_check_at` to null.
- **Support flags:** a refund or chargeback of credit already spent takes what is left and flags the account. A reversed Sponsorship removes the entitlement and flags the account. Work the queue:

  ```sql
  select * from game.support_flags where resolved_at is null order by created_at;
  ```

  Close each flag with `update game.support_flags set resolved_at = now() where flag_id = …`.
- **Steam is down:** `store-steam-init` returns 502 `steam_*`, and players can retry. Nothing is charged without a finalize.
- **Live versus sandbox:** the store calls Steam's sandbox unless `STEAM_MICROTXN_LIVE=true`. Set it only on production, and only after a sandbox purchase, refund and chargeback have passed there.
- **Before real money flows:** buy Supabase point-in-time recovery ([backend-architecture.md](backend-architecture.md) section 22).

## 6. Moderation

- **Report queue:** `deno task admin --staff <you> reports` lists open reports with their replay path. Close each one with `report:handle --id <report> --status actioned|dismissed`. Act on the player with `sanction --account <id> --type <type> --reason <text> [--ends <iso>]`.
- **A name that slipped past the filter** (a word not yet on the list): add the word with `word:block --word <word>`, then run `name:reset --account <id>`. A reset gives a neutral name and starts the 30 days before the next change. A word the filter wrongly refuses goes on the allow-list with `word:allow --word <word>`.
- Outcomes are never told to the reporter.

## 7. Account deletion and export

- **Deletion:** a request waits 14 days, and the player can cancel it in that time. Then the worker deletes the account, its exports and personal rows. Match rows stay, anonymized, and purchases stay for tax records. Requests still pending:

  ```sql
  select * from game.account_jobs where kind = 'deletion' and status = 'queued' order by run_after;
  ```
- **Retries:** failed jobs retry on their own with backoff (`attempts`, `run_after`). After six attempts a job is `failed`.
- **A failed job:** read `error`, fix the cause, then requeue it with `update game.account_jobs set status = 'queued', error = null, attempts = 0, run_after = now() where job_id = …`.
- **A deleted player's ban:** active sanctions are kept against a keyed hash of the identity and come back if it signs up again. Nothing to do. Never rotate `private.secrets.identity_hmac_key`: retained sanctions would stop matching.
- **Auth audit entries:** deletion removes the user's `auth.audit_log_entries`. On a hosted project where the database role may not, the deletion still completes and logs a notice. Check this once per environment.
- **Before the moderation migration on an existing environment:** names that differ only in case must be reset first, or the case-insensitive unique index cannot be built.
- **Exports:** written to the private `exports` bucket under the player's folder. Only that player can read it.

## 8. Rotating keys

- **KINETIC signing keys** (join tickets and server credentials): generate a new key with `deno task keys` in a scratch directory. Add the new public key to `KINETIC_SIGNING_PUBLIC_JWKS` next to the old one, then switch `KINETIC_SIGNING_PRIVATE_JWK`. Remove the old public key after the longest credential lifetime (4 hours). Rotate quarterly.
- **Steam publisher key:** change it in the function secrets and the worker environment together.

## 9. Before each phase exit: the load test

Run the simulator against staging, with dev login on, the login IP limit raised for the test machine, and the fleet scaled:

```bash
deno task load-sim --players 600 --timeout-s 300
```

It prints p50, p95 and the maximum for each step (login, crew, queue, queue to match, join, match to rewards), plus a failure count per step. Watch database CPU and Realtime connections in the Supabase dashboard while it runs. A failure or a p95 queue-to-match over the matchmaking targets blocks the exit.
