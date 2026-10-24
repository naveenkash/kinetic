# KINETIC backend

Phase 1 of [docs/tech/backend-architecture.md](../docs/tech/backend-architecture.md): Supabase (Postgres, Auth, Realtime, Storage, Edge Functions) plus a TypeScript worker for matchmaking and server allocation. Match reports are stored and validated; rewards (Ledger, economy, Contracts, Claims) arrive in Phase 2.

## What is here

| Path | What it does |
|---|---|
| `supabase/migrations/` | Schemas (`api`, `game`, `private`, `admin`), tables, RLS, the rate limiter, every client and server function, Realtime Authorization, Storage buckets |
| `supabase/functions/auth-steam` | Steam ticket (or dev name, local only) to a Supabase session |
| `supabase/functions/match-current` | Reconnect: a fresh join ticket for the player's running match, inside the reconnect window |
| `supabase/functions/server-*` | Game-server endpoints: manifest, heartbeat, player events, upload URLs, match report. Authenticated with a per-match credential, never a Supabase key, and rate-limited per match |
| `supabase/functions/_shared/` | Tokens (Ed25519 join tickets and server credentials), Steam client, HTTP and Supabase helpers |
| `worker/` | Matchmaker (pure rules in `matchmaker.ts`), the tick loop (allocation, `match_found`, timeouts, requeue, deallocation, cleanup), leader election, fleet adapters |
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
deno task test:integration
```

The integration test needs `supabase start` and `deno task functions` running, and `deno task worker` stopped (the test takes the leader lock itself). It checks that public signup is refused, logs in six dev players (including two simultaneous logins), forms two crews, sends crew chat, queues, runs the worker tick, boots the fake server through the local fleet, receives `match_found` over Realtime, joins with tickets, and checks the stored report and uploads.

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
- Phase 2: Ledger, Tonnage, Grades, inventory, economy, Contracts, Claims, ratings, admin console.
