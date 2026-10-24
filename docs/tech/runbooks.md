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
5. **Rehearse the whole flip on staging every season**, a week before the real one:
   1. Restore staging from a recent production copy, so the rehearsal flips this season's real data. In the shell you rehearse from, list staging's Supabase project ref:

      ```bash
      export KINETIC_STAGING_DB_REFS=<staging project ref>
      ```

      The rehearsal reads the project from `DATABASE_URL` (`db.<ref>.supabase.co`, or the pooler's `postgres.<ref>` user) and refuses any project not listed, so a URL to production is refused even through the shared pooler host. `--target local` refuses any database not on this machine. Never list production's ref. The staging worker can keep running: the rehearsal's steps are guarded by each Claim's state, so they and the worker never take the same step twice.
   2. Run the rehearsal:

      ```bash
      deno task flip-rehearsal --target staging --staff <you> --claim claim-15
      ```

      It also refuses `KINETIC_ENV=production`, and any `DATABASE_URL` whose host is `KINETIC_PROD_DB_HOST`. It pulls the Claim's end forward to now, then reports each step: checklist, preview, frozen, tallied, the tally against the preview (a change is noted, not failed, since reports processed in between can move it), reviewed, resolved (the variant flag in config), and the next Claim.
   3. A tie with no holder stops at "reviewed". Rehearse the reviewer's call by running it again with `--holder <faction> --variant <variant>`: it resumes from tallied, and the checklist then only needs a next Claim scheduled, not the 24-hour gap (the first run moved the end). `--force-tally true` tallies without waiting for unprocessed reports, which are then listed as anomalies. `--ignore-checks true` runs past a failed checklist, for a re-run once you know why it failed.
   4. Fix whatever failed, then restore staging again before the next rehearsal.
6. **Before the real freeze**, on production, read-only:
   - `claim:checklist --id claim-15`. Blocking checks:
     - the next Claim is scheduled within 24 hours, with week-1 Contracts for every faction and crates on both tracks;
     - the live build ships every variant the arena can take (`variants:shipped --variants a,b`, set at release);
     - the worker is refreshing standing.

     Unprocessed and quarantined reports are warnings for the reviewer.
   - `claim:preview --id claim-15`: the standings, holder and variant the tally would give now.

## 4a. Live ops from the console

Every command runs under your staff name, and your changes to config, playlists, regions and identities are written to `admin.audit`. A player's own console link is recorded there too, as `player:`. Identity rows in the audit name the platform and account only, never the platform id: the audit outlives deleted accounts.

- **What is wrong right now:** `health`. It shows:
  - queue depth and the oldest wait per pool;
  - matches by state, those with no heartbeat for 60 s, and those allocating for over 2 minutes;
  - live matches per build;
  - the report backlog;
  - account jobs;
  - overdue purchase checks and open support flags;
  - the active Claim's standing age.
- **A player:** `player --name <display name>` or `player --account <uuid>` shows their identities, Ledger, wallets, active sanctions, last five matches, and open reports against them.
- **Message of the day:** `motd:set --text "..."`. An empty text clears it.
- **Feature flags:** `flag:set --flag <name> --value <json>`. Clients read `flags` from `api.get_config`.
- **A playlist or region:** `playlist:set --id <id> --enabled false` takes a playlist out of rotation; `--regions a,b` and `--bot_fill_after_s n` change the rest. `region:set --id <id> --enabled false` takes a region out.
- **Other config:** `config:get [--key]` and `config:set --key --value <json>`. Builds, flags, the message of the day and variants have their own commands, which check them.
- **A patch** (backend-architecture 18):
  1. Push the server image.
  2. Raise `build:latest --build x.y.z`.
  3. Once old matches drain, `build:min --build x.y.z`. It refuses while matches on older builds are still running. `--force true` overrides that for an emergency, and those matches' players can no longer sign in to rejoin.
- **Console identities:** `identity:unlink --account <uuid> --provider psn` removes a linked console platform, never an account's last identity. The identity an account was made for (Steam, say) cannot be unlinked: its next sign-in would attach it again. A player links PlayStation, Xbox or Nintendo through `auth-console` with `link: true`, their session, and fresh proof of an identity the account already has (backend-architecture 8.4).

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
