# KINETIC: Backend Architecture

_Status: proposal v0.2, 2026-10-24. Phase 1 (section 26) is implemented in [backend/](../../backend/README.md) and passes its unit, database and full-loop tests on the local stack. v0.2 rebuilds the backend on Supabase (Postgres, Auth, Realtime, Storage, Edge Functions, Cron, Queues) and drops Redis at launch. This is everything that runs outside a match: accounts, Steam login, crews, presence, matchmaking, server allocation, the Ledger, the economy, Contracts, Claims, moderation, rate limiting, config, telemetry and live-ops tooling. In-match networking is in [networking.md](networking.md) and is unchanged. Progression rules come from [bible/04-progression.md](../bible/04-progression.md); the screens that read this data are in [systems/menus-and-frontend.md](../systems/menus-and-frontend.md). Numbers design has not set yet are marked **PROPOSED**. Vendor limits and prices read on the date above are marked **CHECK**; Supabase's own pricing page is the authority._

## 1. Principles

1. **The dedicated server is the only source of match truth.** Clients never report results, Tonnage, Contract counts or Claim standing. The backend accepts those only from a game server holding a credential the backend issued for that match.
2. **The backend is the only source of player truth.** The game server never trusts a client's loadout, Grade or entitlements; it receives them in the match manifest.
3. **Use the platform before writing code.** Supabase provides Postgres, auth, realtime messaging, file storage, serverless functions, scheduled jobs and a message queue. KINETIC writes only what Supabase does not do: Steam login, the matchmaker loop, fleet allocation, match-report processing and Steam payments.
4. **Postgres is the only stateful store at launch.** No Redis until a measurement asks for it (section 4).
5. **Clients never write tables.** Clients read through views protected by row-level security (RLS) and write only through named functions that validate, rate-limit and log.
6. **Money and progress are append-only.** Every balance change is a transaction row. Every Ledger change traces to a match report or an audited admin action.
7. **The game keeps working while the meta-game lags.** Report processing can fall behind without stopping matches. Matchmaking is the one service that must stay up, so it runs as two instances.

## 2. System map

A one-page version with Mermaid diagrams and Simplified Technical English text is in [backend-architecture-diagram.md](backend-architecture-diagram.md).

```
 PLAYER CLIENT (UE, KineticOnline)
   | Auth REST         | Data API REST (views, RPCs)   | Edge Functions        | Realtime WebSocket
   | /auth/v1          | /rest/v1                      | /functions/v1         | private channels
   v                   v                               v                       v
 +==================================== SUPABASE PROJECT (one region) ====================================+
 |  Auth (GoTrue)      Data API (PostgREST)       Edge Functions (Deno)         Realtime                |
 |  sessions, refresh  exposes schema `api` only  auth-steam, match-current,    user:<id>, crew:<id>    |
 |  MFA for staff      RLS on every view          store-steam-*, voice-token,   Broadcast + Presence    |
 |  bans               RPCs with rate limits      server-* (game servers)       Authorization via RLS   |
 |        \                   |                            |                        /                  |
 |         +------------------+------------ PostgreSQL ----+-----------------------+                   |
 |                            schemas: api | game | private | admin | auth | storage | realtime          |
 |                            extensions: pg_cron (Cron), pgmq (Queues), pg_net                         |
 |  Storage: replays, match-events (private buckets, signed upload URLs)                                 |
 +=======================================================================================================+
          ^  direct Postgres (pooled)                                  ^
          |                                                            |  HTTPS with a per-match credential
 +--------+-------------------------------+              +-------------+---------------------------+
 | kinetic-worker (TypeScript on Deno)    |              | DEDICATED SERVER (UE Linux)            |
 | 2 instances on a small container host  |   allocate   | boot: manifest -> heartbeat -> report  |
 | matchmaker loop (leader by advisory    |------------->| -> signed upload of replay and events  |
 |   lock), report processor (pgmq),      |   Fleet API  | verifies join tickets locally           |
 |   Claims tally, Steam refunds, cleanup |  (Edgegap /  +----------------------------------------+
 +----------------------------------------+  GameLift / Agones / local)

 Outside Supabase: Steam Web API (login tickets, microtransactions, refund reports),
 event store for gameplay telemetry (Axiom, ClickHouse or BigQuery), crash service,
 voice vendor, admin console (web app using Supabase Auth with MFA).
```

## 3. What Supabase provides, and what KINETIC adds

| Need | Supabase feature | Used for | Free plan (**CHECK**) | When to upgrade |
|---|---|---|---|---|
| Database | Postgres | Everything durable | 500 MB, shared CPU, 500 MB RAM; paused after 1 week of inactivity; no backups | Before any data you cannot lose: Pro is $25 a month with 8 GB, daily backups and a $10 compute credit |
| Accounts and sessions | Auth | Session and refresh tokens for every player; MFA for staff; account bans | 50,000 monthly active users | Pro includes 100,000 |
| Steam login | Not provided: Steam uses OpenID 2.0 and web API tickets, not a Supabase provider | Edge Function `auth-steam` (section 8) | | |
| Data access | Data API (PostgREST) with RLS | Reads through `api` views; writes through `api` functions | Unlimited API requests | |
| Push messages, crews, presence | Realtime Broadcast and Presence with Realtime Authorization | `user:<id>` and `crew:<id>` channels | Concurrent connections limited (reported 200 on Free, 500 on Pro; sources disagree) | Every online player holds one connection, so 1,000 concurrent players needs Pro plus extra connections |
| Server-side logic | Edge Functions (Deno, TypeScript) | Steam login, purchases, game-server endpoints, join tickets, voice tokens | Invocation quota per month | Pro raises it |
| Scheduled jobs | Cron (pg_cron), down to seconds | Ticket expiry, Claim freeze, cleanups, presence sweeps | Included | |
| Job queue | Queues (pgmq) | Match reports, exports, deletions | Included | |
| Files | Storage, signed upload URLs | Replays and raw match events | 1 GB, 5 GB egress | Pro has 100 GB; move replays to an S3-compatible bucket if volume grows |
| Secrets | Edge Function secrets, Vault | Steam keys, signing keys | Included | |
| Local development | Supabase CLI (`supabase start`) | The whole stack in Docker on the Mac | Free | |
| Long-running matchmaker | **Not provided**: Edge Functions are request-scoped | `kinetic-worker` on a small container host | | |
| Fleet allocation, Steam payments | **Not provided** | Worker and Edge Functions call those APIs | | |

**Two Supabase projects on the free plan: `kinetic-dev` and `kinetic-staging`.** Production is a Pro project from the first public test that keeps player data (section 22).

## 4. Redis and rate limiting: what is needed

**Rate limiting is needed from Phase 1.** Login, queueing, purchases, reports, name changes, crew invites and friend lookups are all abusable. Supabase Auth rate-limits its own endpoints; everything else gets KINETIC's limiter (section 15).

**Redis is not needed at launch.** What v0.1 used Redis for now lives in Supabase:

| v0.1 use of Redis | v0.2 replacement |
|---|---|
| Crews | `game.crews` and `game.crew_members` tables, pushed over Realtime |
| Presence | Realtime Presence on crew channels; a `game.presence` row refreshed by heartbeat for friend lists |
| Matchmaking tickets | `game.mm_tickets`, an unlogged table read with `FOR UPDATE SKIP LOCKED` |
| Rate-limit counters | `private.rate_limits`, an unlogged table |
| Matchmaker leader lock | Postgres advisory lock |
| WebSocket fan-out | Realtime |

**Add Redis (Upstash, serverless) when any of these is measured:** rate-limit and ticket writes reach a noticeable share of database CPU; matchmaking tickets churn faster than a few hundred updates a second; or Edge Functions need a limiter that does not touch the database. Upstash's free tier is 256 MB and 500,000 commands a month, which covers testing only; pay-as-you-go is about $0.20 per 100,000 commands (**CHECK**). The limiter in section 15 has the same interface either way, so the switch is one module.

## 5. Projects, environments and repository

| Environment | Supabase project | Fleet | Worker |
|---|---|---|---|
| Local | Supabase CLI in Docker on the Mac | `local` adapter launching the Mac server binary | Runs from the terminal |
| Dev | `kinetic-dev` (Free) | Edgegap test account | One instance |
| Staging | `kinetic-staging` (Free, or Pro for the season-flip rehearsal) | Edgegap | One instance |
| Prod | `kinetic-prod` (Pro, with compute sized to load) | Fleet chosen by the beacon test | Two instances |

The backend is its own Git repository:

```
kinetic-backend/
  supabase/
    config.toml                 auth settings, rate limits, function settings
    migrations/                 SQL, one file per change, applied with `supabase db push`
    seed.sql                    local and dev seed: catalog, playlists, a test Claim
    functions/
      _shared/                  tokens, Steam client, rate limiter, validation, types
      auth-steam/  match-current/  voice-token/
      store-steam-init/  store-steam-finalize/
      server-manifest/  server-heartbeat/  server-player-event/  server-report/  server-upload-url/
    tests/                      pgTAP database tests (`supabase test db`)
  worker/                       Deno: matchmaker, report processor, jobs; imports supabase/functions/_shared
  ruleset/                      economy and Claim scoring versions (JSON), owned by the game designer
  catalog/                      item catalog exported from the content build
  admin/                        admin console web app
  openapi/                      contract for the UE client, checked in CI
```

One language, TypeScript, for Edge Functions, the worker and the admin console. CI applies migrations to a throwaway local stack, runs pgTAP and function tests, then deploys to dev on merge and to prod on a tagged release.

## 6. Schemas, keys and exposure

| Schema | Holds | Exposed to the Data API | Who writes |
|---|---|---|---|
| `api` | Views (with `security_invoker = true`) and `security definer` functions that clients call | **Yes, the only exposed schema** | Nobody; it holds no tables |
| `game` | All game tables | No | `api` functions, Edge Functions, the worker |
| `private` | Rate limits, signing-key metadata, internal helpers | No | Functions only |
| `admin` | Audit log, staff roles, quarantined reports | No | Admin console through its own functions |

- In project settings, the exposed schema list is `api` alone; `public` is removed.
- The **publishable key** (formerly anon key) ships in the game client. It is designed to be public; RLS and the `api` functions are the protection.
- The **secret key** (formerly service role key) exists only in Edge Function secrets and the worker's environment. It never ships in the client or the game server build.
- Game servers hold no Supabase key at all. They call `server-*` Edge Functions with a per-match credential signed by KINETIC (section 11). Those functions are deployed with Supabase's JWT check off and verify KINETIC's credential themselves.

Example: a client reading its own inventory.

```sql
create view api.my_inventory with (security_invoker = true) as
  select item_id, acquired_at, source from game.inventory;

alter table game.inventory enable row level security;
create policy inventory_own on game.inventory
  for select to authenticated
  using (account_id = (select auth.uid()));

grant select on api.my_inventory to authenticated;
```

## 7. Data model

All tables live in `game` unless noted. `account_id` is the Supabase Auth user ID (`auth.users.id`).

### 7.1 Identity and social

| Table | Columns |
|---|---|
| `accounts` | `account_id` (PK, references `auth.users`), `display_name` (unique, filtered), `name_changed_at`, `status` (active, suspended, deleting, deleted), `birth_year`, `country`, `consent_flags`, `created_at` |
| `identities` | `provider` (steam, psn, xbox, nintendo, dev), `provider_user_id`, `account_id`; PK `(provider, provider_user_id)` |
| `presence` | `account_id` PK, `state` (menus, crew, queuing, in_match), `match_id`, `updated_at` |
| `crews` | `crew_id`, `leader_id`, `join_code` (for Steam rich presence), `created_at` |
| `crew_members` | `crew_id`, `account_id` (unique: one crew at a time), `harness`, `face`, `ready`, `joined_at` |
| `crew_invites` | `crew_id`, `from_id`, `to_id`, `expires_at` |

### 7.2 Matchmaking and matches

| Table | Columns |
|---|---|
| `playlists` | `playlist_id`, `mode`, `team_size`, `teams`, `rated`, `bot_backfill_after_s`, `regions` (array), `enabled` |
| `regions` | `region_id`, `beacon_host`, `beacon_port`, `enabled` |
| `mm_tickets` (unlogged) | `ticket_id`, `crew_id`, `playlist_id`, `grade_pool`, `build`, `members` (jsonb: account, rating, harness), `pings_ms` (jsonb), `created_at`, `state` |
| `matches` | `match_id`, `playlist_id`, `arena`, `variant`, `region`, `build`, `fleet`, `allocation_id`, `address`, `state` (allocating, waiting_for_players, in_progress, reporting, completed, aborted), `created_at`, `started_at`, `ended_at`, `last_heartbeat_at` |
| `match_players` | `match_id`, `account_id` (null for bots), `team`, `slot`, `harness`, `face`, `loadout` (jsonb, validated), `connected_seconds`, `abandoned`, `result` |
| `match_reports` | `match_id` PK, `received_at`, `payload` (jsonb), `raw_events_path`, `replay_path`, `status` (queued, processed, quarantined), `processed_at`, `ruleset` |
| `ratings` | `account_id`, `mode`, `mu`, `sigma`, `games`, `updated_at` |
| `abandons` | `account_id`, `match_id`, `at` |

### 7.3 Ledger and progression

| Table | Columns |
|---|---|
| `ledgers` | `account_id` PK, `tonnage_total`, `grade`, `weight_balance`, `cleared_at`, `cutter_serial`, `updated_at` |
| `ledger_entries` | `account_id`, `entry_id`, `unlocked_at`, `match_id` |
| `rope_marks` | `account_id`, `rig`, `count`, `tier` (none, tape, band, mark) |
| `tallies` | `account_id`, `tally_id`, `earned_at` |
| `career_stats` | `account_id`, `harness`, `matches`, `wins`, `scraps`, `ringouts`, `tons_moved`, `floor_seconds`, `airborne_seconds` |
| `match_summaries` | `match_id`, `account_id`, `tonnage`, `breakdown` (jsonb), `grade_before`, `grade_after`, `unlocks` (jsonb), `contracts` (jsonb) |

### 7.4 Inventory and economy

| Table | Columns |
|---|---|
| `items` | `item_id` (stable forever), `slot`, `source_rule` (grade, contract, manifest, shop, admin), `grade_gated`, `metadata` (jsonb); loaded from the catalog at deploy |
| `inventory` | `account_id`, `item_id`, `acquired_at`, `source`, `source_ref`; PK `(account_id, item_id)` |
| `loadouts` | `account_id`, `harness`, `line`, `dog`, `cutter`, `cosmetics` (jsonb by slot) |
| `wallets` | `account_id`, `currency` (tonnage_credit, sponsor_credit), `balance` with `check (balance >= 0)` |
| `wallet_txns` | `txn_id`, `account_id`, `currency`, `delta`, `reason`, `ref`, `idempotency_key`, `created_at`; unique `(account_id, idempotency_key)` |
| `shop_rotation` | `claim_id`, `item_id`, `currency`, `price`, `starts_at`, `ends_at` |
| `purchases` | `platform`, `platform_order_id`, `account_id`, `sku`, `amount`, `currency_code`, `status` (initiated, authorized, finalized, refunded, charged_back), timestamps |
| `entitlements` | `account_id`, `entitlement` (for example `sponsorship:<claim_id>`), `granted_at`, `source_ref` |

### 7.5 Allegiance, Contracts and Claims

| Table | Columns |
|---|---|
| `claims` | `claim_id`, `name`, `state` (scheduled, active, frozen, tallied, reviewed, resolved, archived), `starts_at`, `ends_at`, `outcome` (jsonb: per-arena variant) |
| `allegiances` | `account_id`, `claim_id`, `faction`, `declared_at` |
| `contracts` | `contract_id`, `claim_id`, `faction`, `week`, `counter_key`, `target`, `reward` (jsonb) |
| `contract_progress` | `account_id`, `contract_id`, `progress`, `completed_at` |
| `manifest_progress` | `account_id`, `claim_id`, `free_tier`, `sponsored_tier` |
| `claim_standing_events` | `claim_id`, `faction`, `points` (P), `reason` (match, contract, evidence), `match_id`, `account_id`, `week`, `created_at` (append-only) |
| `claim_standing` | `claim_id`, `faction`, `points_total`, `active_players` (A_f), `standing` (S_f), `rated_wins`, `refreshed_at` (materialized hourly) |

### 7.6 Moderation, config and audit

| Table | Columns |
|---|---|
| `reports` | `report_id`, `reporter_id`, `reported_id`, `match_id`, `reason`, `text`, `replay_path`, `status`, `created_at` |
| `sanctions` | `account_id`, `type` (chat_mute, voice_mute, queue_lock, rated_lock, matchmaking_ban, account_ban), `reason`, `starts_at`, `ends_at`, `issued_by` |
| `config` | `key`, `value` (jsonb): `min_build`, `latest_build`, MOTD, feature flags, active variant flags |
| `admin.audit` | `at`, `staff_id`, `action`, `target`, `before`, `after` (append-only, written by trigger) |

## 8. Identity and Steam login

Supabase Auth owns sessions. Supabase has no Steam provider, so an Edge Function bridges Steam to a Supabase session.

### 8.1 Flow

1. The client requests a Steam web API ticket for the identity string `kinetic-backend` and calls `POST /functions/v1/auth-steam` with the ticket and its build ID.
2. `auth-steam` rate-limits by IP, then validates the ticket with the Steam Web API user-authentication endpoint using the publisher key held in function secrets. That returns the SteamID and ban flags (**CHECK** endpoint and fields in Steamworks docs).
3. It looks up `identities` for `(steam, steamid)`. If none, it creates a Supabase Auth user through the admin API with a synthetic, never-mailed address (`steam-<steamid>@users.kinetic.invalid`) and `app_metadata` naming the provider and provider user id, then the game rows. Public signup is off on every project, and `srv_ensure_account` adopts an Auth user only if its `app_metadata` (writable only through the admin API) names exactly this identity, so nobody can pre-register a player's address and take over the account. If `createUser` fails because a simultaneous first login won the race, the function finds that user with a read-only lookup and continues; with no user at all it returns 503 and creates nothing.
4. It checks `config.min_build` and account sanctions.
5. It mints a Supabase session for that user server-side (section 8.2) and returns the access and refresh tokens.
6. From then on the client refreshes through Supabase Auth's own refresh endpoint. The session works for the Data API, Edge Functions and Realtime alike.

### 8.2 Minting the session

The pattern: the admin API generates a one-time magic-link token for the user, and the function immediately exchanges it for a session with `verifyOtp`, without sending any email. One user reports the same token being returned on repeated calls for the same user (a reported Supabase Auth issue), so this was the first thing prototyped. On the local stack (Supabase CLI 2.109) back-to-back logins for the same user work and the integration test covers it; repeat the check on the hosted project, including logins from two machines.

If it proves unreliable, the fallback is to sign Supabase-compatible access tokens in `auth-steam` with the project's JWT signing key and manage KINETIC's own refresh tokens. That costs Supabase Auth's refresh and revocation handling, so it is the second choice.

### 8.3 Custom claims and bans

- A Postgres **custom access token hook** adds `kinetic_status` and, for staff, `app_role` to every access token.
- Account bans use Supabase Auth's user ban (sessions stop refreshing) plus a `sanctions` row for the reason and expiry. Matchmaking bans and mutes are `sanctions` rows checked by the relevant functions.

### 8.4 Other providers

- `dev` login (username only) exists in local and dev and is disabled in staging and prod by config.
- Console identities later go through the same `auth-steam`-style bridge, one Edge Function per platform.
- Discord, Google or Apple sign-in, which Supabase supports natively, can be linked for the web Ledger and admin console without touching the game client.

## 9. Crews, presence, friends and Realtime

### 9.1 Channels

| Topic | Type | Members | Carries |
|---|---|---|---|
| `user:<account_id>` | Private, Broadcast | That player | `crew_invite`, `match_found` (address plus join ticket), `match_processed`, `queue_reset`, notices |
| `crew:<crew_id>` | Private, receive only | Crew members | `crew_state` (members, picks, ready flags, the current `chat_topic`) and `queue_state`, sent by the server. Never the join code, which members read from `api.my_crew` |
| `crew-chat:<crew_id>:<chat_key>` | Private, Broadcast and Presence | Crew members | Crew text chat and who is online. The chat key rotates whenever a member leaves |

Realtime Authorization restricts who can join each topic:

```sql
create policy realtime_user_and_crew on realtime.messages
  for select to authenticated
  using (
    realtime.topic() = 'user:' || (select auth.uid())::text
    or exists (
      select 1 from game.crew_members m
      where realtime.topic() = 'crew:' || m.crew_id::text
        and m.account_id = (select auth.uid())
    )
  );
```

Realtime checks access once per channel join, without the event name, so a topic is either writable by clients or not. The only client-writable topic is the crew-chat topic; server messages (`crew_state`, `queue_state`, `match_found`, `match_processed`) go on receive-only topics, sent from database functions and the worker with `realtime.send(...)`, so a client can never forge them. Because access is cached per join, removal is enforced by rotation: when a member leaves or is kicked the join code and chat key change, and a kicked player cannot rejoin by code for 10 minutes (**PROPOSED**; an invite from the crew overrides it).

### 9.2 Crews

Crews are rows, so a crew survives a client reconnect and every change is transactional. Functions: `api.crew_create`, `api.crew_invite`, `api.crew_accept`, `api.crew_decline`, `api.crew_leave`, `api.crew_kick`, `api.crew_promote`, `api.crew_set_pick` (harness, Face, ready). Each one validates, rate-limits, writes and broadcasts the new crew state on `crew:<id>`. A crew is one to three members; the leader queues.

### 9.3 Friends and presence

- Friends come from Steam. The client reads its Steam friends list and calls `api.friends_resolve(steam_ids)` (rate-limited, capped at 500 IDs per call), which returns the matching KINETIC accounts and their `presence` state.
- `api.presence_heartbeat` every 30 seconds updates `presence`; a Cron job marks rows older than 90 seconds offline.
- The client sets Steam rich presence with the crew `join_code`, so Steam's "Join Game" and overlay invites work.
- A KINETIC friends table is added when a second platform ships, so crossplay friends work.

## 10. Matchmaking and match lifecycle

### 10.1 Tickets

The crew leader calls `api.queue_join(playlist_id, pings_ms)`. The function checks sanctions, abandon cooldowns, build, crew size and readiness, and inserts one `mm_tickets` row per crew with each member's rating and harness. `api.queue_cancel` removes it. Pings come from the client measuring UDP round trips to each region's beacon (median of five, at login and every 10 minutes, per [networking.md](networking.md) section 9); the beacon list comes from `api.config`.

### 10.2 The loop

`kinetic-worker` runs the matchmaker. Two instances; the one holding `pg_try_advisory_lock(<matchmaker>)` is the leader, and the other takes over if the leader's connection drops. Every second, per pool `(playlist, Grade pool, build)`. Pools, crew formation and Rated rules are canonical in [systems/game-modes-and-match-flow.md](../systems/game-modes-and-match-flow.md) section 6; the loop implements them:

1. Read waiting tickets with `select ... for update skip locked`.
2. **Grade pools** (game-modes 6.1): Apprentice (Grade 1), Hand (Grades 2 to 3), Journeyman (Grades 4 to 6), and Rated (Grade 2 and up, by Office Rating). A casual ticket widens to the adjacent pool after 90 s and to all pools after 180 s. A crew queues in the pool of its highest Grade member (**PROPOSED**).
3. **Eligible regions** per ticket: ping at or under 150 ms. The cap never widens; a crew waits longer instead (networking section 9).
4. **Build teams of three** (game-modes 6.2). A 3-stack is a team and meets other stacks first, then mixed crews after 60 s. A duo takes its third from solo queue. Three solos form a crew, and the lobby shows the role suggestion from their Ledgers.
5. **Pair teams** within a rating gap that widens with wait time (**PROPOSED**: from 3 to 10 points of `mu` over 120 s).
6. **Pick the region** that minimizes the highest ping among all players, from regions every ticket shares.
7. **Bots** (game-modes 9): in the Apprentice pool bots may fill empty slots at match start; in other casual pools a bot replaces a leaver after the 10 s withdrawal (section 10.5); Rated never uses bots. A crew of three bots never exists. **PROPOSED**: if population is too low to fill casual matches in a region, `bot_backfill_after_s` lets bots fill one or two slots per crew at match start, never a whole crew.
8. Free Scrap (8 players) uses the same loop with team size one.

**Office Rating** (game-modes 6.3) is implemented with OpenSkill (team-aware, freely licensed): one hidden rating per Rated mode (Scrap, Claim), shown on the Ledger only as a band (Provisional, Inspected, Certified, Rated, Sanctioned, Championship). Casual pools use OpenSkill quietly inside a Grade pool. Grades gate experience pools, never power.

### 10.3 Allocation

For each pairing the worker, in one transaction, deletes the tickets, inserts `matches` (state `allocating`) and `match_players` with each player's validated loadout, and commits. It then calls the fleet adapter:

```ts
interface Fleet {
  allocate(req: { region: string; image: string; env: Record<string, string> }): Promise<{ allocationId: string; host: string; port: number }>;
  status(allocationId: string): Promise<"starting" | "ready" | "stopped" | "failed">;
  deallocate(allocationId: string): Promise<void>;
}
```

`env` carries `MATCH_ID`, `BACKEND_URL`, the per-match server credential and the join-ticket public keys. Adapters: `edgegap` (launch default), `gamelift`, `agones`, and `local` (spawns the Mac or Linux server binary on the developer's machine). The fleet is still chosen by the beacon test (tech/README decision 13). An allocation failure returns the tickets to the front of the queue and broadcasts `queue_reset` only if retries are exhausted.

### 10.4 Join

1. The server boots, calls `server-manifest`, loads arena and variant, and reports ready through `server-heartbeat`.
2. The worker sends `match_found` on each player's `user:` channel: address plus a join ticket.
3. The client connects over UDP with the join ticket in its login options. The server verifies the signature in `PreLogin` against the public keys from its environment, checks the roster and slot, and admits the player. No backend call on join.
4. **Reconnect:** `POST /functions/v1/match-current` returns a fresh join ticket while the player's match is `in_progress`.

### 10.5 States and timeouts

```
allocating --ready--> waiting_for_players --all joined or 60 s--> in_progress --> reporting --> completed
     |                        |                                       |
     +--fail--> requeue       +--nobody joined--> aborted             +--no heartbeat 60 s--> aborted
```

The heartbeat contract for any game server: heartbeat at boot and every 10 seconds until the report is sent. The backend marks a server ready on the first heartbeat after the worker has recorded its address (or when the worker records the address of a server that already heartbeated for the current attempt); each allocation attempt gets a new allocation id, so a server from an earlier attempt is refused. The worker's timeout pass, every tick, aborts matches whose heartbeat is older than 60 seconds and deallocates them. It also aborts a server that never reports ready within 90 seconds (those players go back to the queue in their original position) and a match nobody joins within 60 seconds of ready. Leaver, idle, forfeit and dodge rules are canonical in game-modes section 8, and the backend enforces them:

| Rule (game-modes 8) | Backend action |
|---|---|
| Reconnect window 60 s casual, 90 s Rated | `match-current` issues join tickets inside the window only |
| Leaver after the window | `abandons` row; in casual the server backfills a bot; in Rated the leaver takes the full loss plus the Ledger entry "Walked off a claim" |
| Three Rated leaves in one Claim | `sanctions` row, type `rated_lock`: Rated locked for 7 days |
| Idle 60 s | The server reports the player as a leaver |
| Forfeit vote, 2 of 3 | Reported by the server; Rated forfeit costs Rating as a loss plus 20% |
| Declining a formed Rated lobby twice in an hour | 15-minute queue lock (`sanctions`, type `queue_lock`) |

## 11. Game-server integration

### 11.1 Credentials

| Token | Signed by | Lifetime | Holder | Contents |
|---|---|---|---|---|
| Supabase access token | Supabase Auth | About 1 hour, refreshed | Player client | User ID, KINETIC claims |
| Server credential | KINETIC Ed25519 key in function secrets | Match length plus 1 hour | One game server | `match_id`, `allocation_id`, audience `server` |
| Join ticket | Same key family | 120 seconds | Player client | `account_id`, `match_id`, `team`, `slot`, audience `match:<id>` |

Keys carry a key ID and rotate quarterly; servers receive the current and previous public keys.

### 11.2 Server endpoints (Edge Functions)

| Function | Called | Does |
|---|---|---|
| `server-manifest` | Once at boot | Returns roster, validated loadouts, arena, variant flags, playlist rules, ruleset and event-schema versions |
| `server-heartbeat` | Every 10 s | Updates `last_heartbeat_at`, state, connected players |
| `server-player-event` | On join, leave, abandon | Updates `match_players`; feeds reconnect and abandon rules |
| `server-upload-url` | At match end | Returns signed Storage upload URLs for the replay and raw events |
| `server-report` | At match end | Validates and stores the report and completes the match. Phase 2 adds the queue (pgmq) that the worker reads to pay rewards |

The server writes its report to local disk before sending and retries with backoff for up to 24 hours.

## 12. Match reports and progression

### 12.1 Payload

The server reports **facts**; the backend turns facts into rewards, so the economy can be tuned without patching the game.

```json
{
  "match_id": "...", "build": "...", "ruleset": 3, "event_schema": 2,
  "mode": "scrap", "arena": "YRD", "variant": "base", "winner_team": 1,
  "rounds": [{"winner_team": 1, "seconds": 142, "stalls": 4}],
  "players": [{
    "account_id": "...", "team": 1, "harness": "Rigger", "face": "teodora-quispe",
    "connected_seconds": 712, "abandoned": false, "is_bot": false,
    "facts": {
      "mass_distance_kg_m": 184200, "eliminations": 3, "assists": 2, "spotter_credits": 1,
      "ringout_heights_m": [38.0], "rope_marks": [{"rig": "Gallows"}],
      "counters": {"lines_hung_used_by_others": 2, "rated_tons_moved": 0,
                   "rounds_without_floor": 0, "scraps_with_lighter_object": 1, "snaps_survived": 0},
      "floor_seconds": 401, "airborne_seconds": 66, "hoisted_seconds": 12
    }
  }],
  "raw_events_sha256": "..."
}
```

Facts come from the same `KineticCore` event stream as playtest instrumentation ([playtest-plan.md](../production/playtest-plan.md)). Spotter credit is computed on the server from line ownership at the time of each Stall. `counters` keys are the Contract objectives from bible/04, so a new Contract built on existing counters needs no patch.

The engine-free implementation (`MatchEvents`, `Credit` and `MatchReport` in [game/](../../game/README.md)) writes event_schema 1. The raw events are JSON Lines, one object per event with its `type` and `v`, and `raw_events_sha256` is the SHA-256 of those exact bytes. Credit rules and counter meanings are **PROPOSED**, documented in `Credit.h` and `MatchReport.h`:
- **Killer:** the enemy whose Haul or Jack acted last on the victim, or on the object that hit them, within 5 s; failing that, whose Hang or Set line did.
- **Spotter:** another enemy whose Hang or Set line counted. A bot's Spotter credit goes to the human killer.
- **Assists:** every other enemy line that counted.
- **Counters:** `lines_hung_used_by_others` counts Spotter credits; `rounds_without_floor` counts rounds played without touching the floor; `snaps_survived` counts overspin snaps not followed by the owner's Stall within 5 s. Team numbers are 1 and 2.
- **Result fields:** the report adds `forfeit_team` (null, 1 or 2), `forfeit_penalty_free` (a Rated short crew forfeiting after 60 s, game-modes 8) and `no_contest` (both crews lost their last humans together; `winner_team` is null), so the worker can apply the forfeit rules.
- **Deleted accounts:** a human whose account was deleted before the manifest was fetched has `account_id: null` in the manifest, so the server reports it as null. `srv_store_report` accepts that and matches it per team against the roster's null-account human slots (migration `20261025000100_report_deleted_accounts.sql`).

### 12.2 Validation in `server-report`

Reject or quarantine when the credential is not for this match; the roster differs from `match_players`; `connected_seconds` exceeds match length; a fact exceeds a physical ceiling (**PROPOSED** per-match caps derived from bible/05); or the match is already reported (`match_reports` primary key). Quarantined reports go to the admin console, not the Ledger.

### 12.3 Processing in the worker

The worker reads `match_reports` from pgmq and processes each match in one database transaction:

1. **Tonnage** from the ruleset version in force: mass moved times distance, eliminations, assists, spotter share, ring-out height. Coefficients are **PROPOSED** and live in `ruleset/`, owned by the game designer.
2. **Ledger:** Tonnage total; Grade thresholds (**PROPOSED**); Weight paid down and "Clear" at zero; Ledger entry unlocks; Cutter serial at Greenhand to Hand.
3. **Rope Marks:** counts and Tape, Band, Mark tiers.
4. **Contracts:** add counters to the player's active Contracts; grant completions.
5. **Manifest:** Free column, plus Sponsored if the player holds `sponsorship:<claim>`.
6. **Claim standing:** append `claim_standing_events` (section 14).
7. **Ratings:** OpenSkill on rated playlists.
8. **Summary:** write `match_summaries`, set the match `completed`, send `match_processed` on each player's channel.

Bots earn nothing and never earn Standing for anyone; Tonnage from a bot's Spotter credit goes to the human it set up (game-modes 9). Private matches earn Rope Mark practice only (**PROPOSED**). A failed job stays on the queue and is retried; after five failures it is quarantined.

## 13. Inventory, loadouts and economy

### 13.1 Catalog

The item catalog (cosmetics, sidegrades, Manifest crates, shop rotations per Claim) is data exported from the content build into `catalog/`, loaded into `items` and `shop_rotation` by a deploy step. Sidegrades (Lines, Dogs, Cutters) are `grade_gated` and the loader refuses any catalog that puts one in a shop or the Manifest.

### 13.2 Loadouts

`api.set_loadout(harness, slots)` checks ownership and Grade gates and writes `loadouts`. The match manifest re-validates at allocation time, so a loadout changed mid-queue cannot smuggle an item in.

### 13.3 Spending earned or sponsor credit

```sql
create function api.shop_purchase(p_item text, p_expected_price int, p_idem uuid)
returns jsonb
language plpgsql security definer set search_path = ''
as $$
declare
  v_uid uuid := auth.uid();
  v_price int; v_currency text; v_balance bigint;
begin
  if not private.hit_rate_limit('shop:' || v_uid, 10, interval '1 minute') then
    raise exception 'rate_limited';
  end if;
  if exists (select 1 from game.wallet_txns where account_id = v_uid and idempotency_key = p_idem) then
    return jsonb_build_object('status', 'duplicate');
  end if;
  select r.price, r.currency into v_price, v_currency
    from game.shop_rotation r
   where r.item_id = p_item and now() between r.starts_at and r.ends_at;
  if v_price is null or v_price <> p_expected_price then raise exception 'price_changed'; end if;
  if exists (select 1 from game.inventory where account_id = v_uid and item_id = p_item) then
    raise exception 'already_owned';
  end if;
  update game.wallets set balance = balance - v_price
   where account_id = v_uid and currency = v_currency and balance >= v_price
  returning balance into v_balance;
  if not found then raise exception 'insufficient_funds'; end if;
  insert into game.wallet_txns (account_id, currency, delta, reason, ref, idempotency_key)
    values (v_uid, v_currency, -v_price, 'shop', p_item, p_idem);
  insert into game.inventory (account_id, item_id, source, source_ref)
    values (v_uid, p_item, 'shop', p_idem::text);
  return jsonb_build_object('status', 'ok', 'balance', v_balance);
end $$;
```

The `balance >= v_price` guard in the `update` and the unique `(account_id, idempotency_key)` keep concurrent purchases safe.

### 13.4 Real money through Steam

1. `store-steam-init`: rate-limit, create a `purchases` row, call Steam's microtransaction init endpoint (sandbox endpoint in dev and staging); Steam shows the overlay.
2. The client receives Steam's authorization callback and calls `store-steam-finalize`.
3. `store-steam-finalize` calls Steam's finalize endpoint, then in one transaction marks the purchase finalized and inserts the `wallet_txns` credit, keyed on the order ID.
4. A daily worker job reads Steam's transaction report for refunds and chargebacks and reverses grants. A balance never goes negative; if credit was already spent, the account is flagged for support.

Real-money prices are always shown next to credit amounts. There are no randomized paid items, by design. How Sponsorship and the Weight interact in numbers is still a design decision (section 27, B5).

## 14. Allegiance, Contracts and Claims

- `api.declare_allegiance(faction)`: one per Claim; switching keeps earned rewards and resets Contract progress, per bible/04.
- Contracts: three per faction per week, defined in season data as `{counter_key, target, reward}`.

**Claim standing** is canonical in [systems/game-modes-and-match-flow.md](../systems/game-modes-and-match-flow.md) section 7. The worker writes one `claim_standing_events` row per player per match with Standing points P:

| Result | P |
|---|---|
| Win, casual | 3 |
| Loss, casual | 1 |
| Win, Rated | 4.5 |
| Loss, Rated | 1.5 |
| Free Scrap top 3 | 2; otherwise 1 |
| Contract completed | +2 each |
| Evidence object recovered | +5, once per player per Claim |
| Forfeit or leave | 0, and the match does not count |

The weekly cap is 60 P per player; matches past it still pay Tonnage, never Standing. Bots never earn Standing for anyone.

A Cron job recomputes faction standing hourly:

S_f = (sum of P over the faction's players this Claim) / max(A_f, 0.6 × mean(A))

where A_f is the number of the faction's players with at least 1 P this Claim and mean(A) is the mean across the five factions. `claim_standing` stores S_f, shown to one decimal on the Claims map. At the end of the Claim the highest S_f holds it; a tie goes to the faction with more Rated wins.

**The season flip** is a state machine run from the admin console:

```
scheduled --> active --> frozen --> tallied --> reviewed --> resolved --> archived
```

1. **Frozen** by a Cron job at `ends_at`; later reports add nothing.
2. **Tallied** by the worker; anomalies (accounts far above the distribution, quarantined reports) listed for review.
3. **Reviewed** by a person who signs off in the console.
4. **Resolved:** the per-arena outcome from bible/04 (Hangtown's district, Ballast's cut or stabilization, the Yard's Haulers, the Anchor's colors) is written to `config` as variant flags; the next Claim starts.

Every variant already ships in the client and server build. The patch carrying both outcomes must be live on every platform before the freeze, weeks earlier on consoles because of certification. Rehearse the flip on staging every season.

## 15. Rate limiting

### 15.1 Layers

| Layer | Mechanism | Covers |
|---|---|---|
| Supabase Auth | Built-in, configurable limits | Token refresh, sign-in endpoints |
| Steam login | KINETIC limiter by IP and by SteamID in `auth-steam` | Ticket floods |
| `api` functions | `private.hit_rate_limit` at the top of every write function | Everything a client can change |
| Edge Functions | Same limiter through an RPC | Purchases, reconnect tickets, voice tokens |
| Game-server endpoints | Per-match credential; 60 calls per minute per endpoint per match | A leaked credential cannot flood the backend or touch other matches |
| Fleet | Provider DDoS protection on game servers | UDP floods |

### 15.2 The limiter

```sql
create unlogged table private.rate_limits (
  key text primary key,
  window_start timestamptz not null,
  hits int not null
);

create function private.hit_rate_limit(p_key text, p_max int, p_window interval)
returns boolean
language plpgsql security definer set search_path = ''
as $$
declare v_hits int;
begin
  insert into private.rate_limits as r (key, window_start, hits)
  values (p_key, now(), 1)
  on conflict (key) do update
    set hits = case when r.window_start < now() - p_window then 1 else r.hits + 1 end,
        window_start = case when r.window_start < now() - p_window then now() else r.window_start end
  returning hits into v_hits;
  return v_hits <= p_max;
end $$;
```

A fixed window per key. Unlogged, so counters are cheap and are lost on a database restart, which is acceptable. A Cron job deletes keys idle for an hour. The same function signature is implemented against Upstash if Redis is added (section 4).

### 15.3 Limits (PROPOSED)

| Action | Key | Limit |
|---|---|---|
| Steam login | IP (the gateway's last `x-forwarded-for` hop, or the CDN header when `TRUST_CDN_IP` is set); SteamID | 10 per minute (`KINETIC_LOGIN_IP_LIMIT`); 5 per minute |
| Queue join or cancel | account | 20 per minute |
| Crew create, invite, accept, kick | account | 30 per minute |
| Crew chat | account | 10 messages per 10 seconds |
| Friends resolve | account | 6 per minute |
| Presence heartbeat | account | 4 per minute |
| Loadout change | account | 30 per minute |
| Shop purchase | account | 10 per minute |
| Steam purchase init | account | 5 per minute |
| Player report | account | 5 per hour |
| Display name change | account | 1 per 30 days after the first |
| Match-current (reconnect ticket) | account | 10 per minute |
| Data export or deletion request | account | 1 per day |

## 16. Voice, chat and anti-cheat

| Feature | Recommendation |
|---|---|
| Voice | A games voice service with an Unreal plugin, rooms per crew and per team, tokens minted by the `voice-token` Edge Function (for example Vivox or ODIN by 4Players; **CHECK** pricing and UE 5.8 support). Self-hosted LiveKit is the cost alternative. Crew voice on by default; no match-wide voice. Muted by default for accounts under the age threshold |
| Text chat | Crew chat only, over the `crew:` Realtime channel, filtered by a word list in the insert policy's function and rate-limited. Rig Calls (bible/04) are the in-match communication |
| Client anti-cheat | Server authority is the primary defense ([networking.md](networking.md) section 11, [player-physics.md](player-physics.md) section 10). A commercial client anti-cheat (for example BattlEye; **CHECK** price and Steam Deck support) is evaluated in Phase 3. The worker flags impossible facts and systematic favourable mispredictions |

## 17. Moderation and the admin console

- `api.report_player(match_id, reported_id, reason, text)` attaches the server replay path automatically.
- Name filter on `api.change_display_name`.
- **Admin console:** a web app (the web is the right place for it) using Supabase Auth with email and TOTP multi-factor authentication. Staff get `app_role` through the access-token hook; every admin function checks it and writes `admin.audit` through a trigger. Screens: player lookup, Ledger and inventory, grants and refunds, sanctions, report queue with replay links, quarantined reports, Claims dashboard and flip controls, playlists and regions, matchmaking health.

## 18. Config, versions and playlists

`api.config` (readable without login) returns `min_build`, `latest_build`, open playlists per region, region beacons, the current Claim summary and variant flags, feature flags and the message of the day. Bible numbers never live here; they ship in the build so every client and server agree (project-structure section 4).

**Patch rollout:** push the new server image to the fleet; the matchmaker pools by build, so old and new clients each match on their own servers; raise `min_build` once old matches drain.

## 19. Storage, privacy and retention

| Bucket or data | Retention (**PROPOSED**) |
|---|---|
| `replays` (private) | 30 days; kept while attached to a report |
| `match-events` (private, raw event files) | 90 days; kept while attached to a report |
| Accounts, Ledger, inventory | Life of the account |
| Wallet and purchase history | As long as tax law requires |
| Match records and summaries | Life of the account; anonymized after deletion |
| Gameplay telemetry (outside Supabase) | 13 months |

Free-plan Storage is 1 GB, which holds only a short window of replays; set the retention job before the first playtest.

- `api.request_export` queues a job that writes an archive to a private bucket and returns a signed download URL.
- `api.request_deletion` starts a 14-day grace period; then a job deletes the Auth user, identities, display name and personal fields, chat records and voice records, and anonymizes match rows so other players' history stays intact.

## 20. Security checklist

- Only the `api` schema is exposed; every `game` table has RLS enabled even though it is not exposed.
- Every `security definer` function sets `search_path = ''` and schema-qualifies every name.
- Publishable key in the client; secret key only in function secrets and the worker; no Supabase key on game servers.
- Public signup off on every Supabase project; Auth users are adopted only through the `app_metadata` check (section 8.1).
- Nothing is granted to `PUBLIC`: Postgres grants new functions to `PUBLIC` by default, so every migration that adds a function revokes and re-grants explicitly.
- Game-server processes start with a clean environment: only their match settings, never the signing key, service key or database URL.
- The worker's leader lock is held on its own direct database connection; losing that connection ends leadership at once.
- Steam publisher key and KINETIC signing keys in function secrets; nothing secret in the game build.
- Server and join tokens are Ed25519 with key IDs, rotated quarterly.
- Supabase's database advisors (security and performance) run in CI against staging; any new warning fails the build.
- Staff accounts require MFA; admin actions are audited.
- Dependency scanning on the backend repository.

## 21. Telemetry

- Gameplay events and server health go from game servers straight to the event store (Axiom, ClickHouse or BigQuery), not into Supabase; the free 500 MB database would fill in days.
- Edge Function and database logs are in Supabase's log explorer; the worker sends structured logs and OpenTelemetry traces to the event store.
- First dashboards: queue time by playlist and region, allocation failures, report-processing lag, concurrent players, Realtime connection count against the plan limit, database CPU.

## 22. Cost and limits by phase

| Phase | Supabase | Other backend | Notes |
|---|---|---|---|
| Phase 0 | Local CLI only | None | $0 |
| Phase 1 (Steam Playtest) | `kinetic-dev` and `kinetic-staging` on Free | Worker on a small container host, a few dollars a month (**CHECK**) | Free projects pause after a week idle and have no backups; acceptable for playtest data only |
| Phase 2 | Same, plus a Pro `kinetic-prod` once any data must persist: $25 a month with a $10 compute credit | Same | Pro adds daily backups |
| Phase 3 (1,000-player beta) | Pro with Small or Medium compute ($15 or $60 a month instance; credit applies) plus Realtime connections beyond the included amount (**CHECK**) | Two worker instances; voice vendor | Point-in-time recovery is a paid add-on; buy it before real money flows |
| Live | Compute and Realtime scale with players | Upstash Redis only if section 4's triggers are met | Game-server hosting dominates total cost ([tech-stack-plan.md](tech-stack-plan.md) section 11) |

Rough production backend cost at 1,000 concurrent players: $60 to $250 a month before point-in-time recovery, mostly compute and Realtime connections (**PROPOSED** estimate; confirm on Supabase's calculator).

## 23. Local development on the Mac

1. `supabase start` brings up Postgres, Auth, Data API, Realtime, Storage, Edge Functions runtime and Studio in Docker.
2. `supabase db reset` applies migrations and `seed.sql` (config, regions, playlists).
3. `deno task functions` serves the Edge Functions with the local secrets from `deno task keys`.
4. `deno task worker` starts the worker with the `local` fleet adapter, which launches a game-server process on a free port per match: the fake server in `backend/tools/fake-server.ts` until the Mac `KineticServer` build exists. Local ports are 554xx so they do not clash with other local Supabase projects.
5. The UE client (PIE or a packaged Mac build) points at the local URLs and logs in with the `dev` provider. The full loop (crew, queue, allocate, join, report, Ledger update, `match_processed` push) runs on one machine with no cloud account.

## 24. Unreal-side integration

There is no official Supabase SDK for Unreal. `KineticOnline` talks to Supabase's documented HTTP and WebSocket APIs directly.

| Class | Job |
|---|---|
| `FKineticSupabaseClient` | Base URL, publishable key header, bearer token, JSON in and out for `/auth/v1`, `/rest/v1` and `/functions/v1` |
| `FKineticRealtimeClient` | Realtime WebSocket using the Phoenix channel protocol: join private topics with the access token, heartbeats, token refresh on the socket, Broadcast and Presence events |
| `UKineticSessionSubsystem` | Steam ticket, `auth-steam`, token storage and refresh, `min_build` check |
| `UKineticCrewSubsystem` | Crew functions, `crew:` channel, Steam rich presence join codes |
| `UKineticMatchmakingSubsystem` | Beacon pings, queue functions, `match_found`, travel with the join ticket, `match-current` on reconnect |
| `UKineticProfileSubsystem` | Ledger, inventory, loadouts, Contracts, Claims, shop for the UI |
| `UKineticStoreSubsystem` | Steam overlay purchase flow with `store-steam-*` |

Server side, in `KineticServer` ([project-structure.md](project-structure.md)): manifest fetch, join-ticket verification in `PreLogin`, heartbeats, player events, the report builder over the `KineticCore` event stream, signed uploads, fleet SDK hooks.

## 25. Testing

- **Database:** pgTAP tests for every `api` function, RLS policy and the rate limiter (`supabase test db`), including "another account cannot read this" for every view.
- **Economy:** golden-file tests in the worker: report in, Ledger, wallet and Contract deltas out, per ruleset version.
- **Integration:** the local stack with the `local` fleet; a scripted crew queues, plays a bot match and checks the Ledger.
- **Load:** a Deno simulator plays thousands of clients through login, crew, queue and match against staging with fake servers posting reports; it measures database CPU, Realtime connections and queue time before each phase exit.
- **Contract:** CI checks the UE client wrapper against the OpenAPI document.

## 26. Build order

| Phase | Scope | Size (one backend engineer, **PROPOSED**) |
|---|---|---|
| Phase 0 | None. `dev` login and a JSON profile in the client | 0 |
| Phase 1 (Steam Playtest) | Supabase projects; schemas and RLS; `auth-steam` with the session-minting prototype first; config and `min_build`; crews and Realtime channels; rate limiter; matchmaker without ratings; fleet adapter; join tickets; `server-*` functions; reports stored without rewards; telemetry; ban list | 4 to 6 weeks |
| Phase 2 | Ledger, Tonnage, Grades, Rope Marks, inventory, loadouts, Contracts, Claims standing, OpenSkill, shop with Tonnage credit, admin console v1, Pro production project | 9 to 11 weeks |
| Phase 3 | Steam purchases and refunds, voice, moderation tools, export and deletion, load test, point-in-time recovery, runbooks | 5 to 7 weeks |
| Phase 4 | Season-flip tooling and rehearsal, console identity bridges, live-ops tools | Ongoing |

## 27. Decisions to make

| # | Decision | Default here | Needed by |
|---|---|---|---|
| B1 | Backend platform | Supabase (Postgres, Auth, Realtime, Storage, Edge Functions, Cron, Queues) | Phase 1 start |
| B2 | Database engine | Supabase Postgres. CockroachDB rejected: KINETIC needs one region, its volume is small, and CockroachDB does not run the Postgres extensions Supabase Auth, Realtime, Cron and Queues depend on. Revisit only for multi-region writes at tens of thousands of concurrent players | Phase 1 start |
| B3 | Steam session minting | Magic-link token exchanged server-side; fallback is self-signed Supabase-compatible tokens | Phase 1 week 1 prototype |
| B4 | Redis | None at launch; Upstash when section 4's triggers are measured | Phase 3 load test |
| B5 | Economy numbers: Tonnage coefficients, Grade thresholds, Weight size, credit prices, Sponsorship terms | Design to set | Phase 2 start |
| B6 | Claim scoring | Settled: game-modes section 7, implemented in section 14 | |
| B7 | Crew formation and Grade pools | Settled: game-modes section 6, implemented in section 10.2 | |
| B8 | Low-population bot fill at match start outside the Apprentice pool | Off; turn on per region only if casual queues cannot fill | Phase 3 |
| B9 | Voice vendor or self-hosted | Vendor with an Unreal plugin | Phase 3 |
| B10 | Client anti-cheat | Server authority plus backend statistics; commercial client anti-cheat evaluated in Phase 3 | Phase 3 |
| B11 | Worker host | A small container host with two instances | Phase 1 |
| B12 | Rate limits | Section 15.3 | Phase 1 |
| B13 | Data retention | Section 19 | Phase 3 |
