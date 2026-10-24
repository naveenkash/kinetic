-- KINETIC backend, Phase 1 schema. See docs/tech/backend-architecture.md sections 6 and 7.
-- Only the `api` schema is exposed through the Data API. Game tables live in `game`.

create schema if not exists api;
create schema if not exists game;
create schema if not exists private;
create schema if not exists admin;

revoke all on schema game, private, admin from public, anon, authenticated;
grant usage on schema api to anon, authenticated, service_role;
-- authenticated needs usage on `game` so security_invoker views in `api` can read
-- through RLS, and on `private` for the RLS helper functions.
grant usage on schema game to authenticated, service_role;
grant usage on schema private to authenticated, service_role;

-- Postgres grants EXECUTE on new functions to PUBLIC, and a per-schema default cannot
-- take that away. Revoke it globally for functions this role creates from now on; the
-- functions migration also revokes explicitly and re-grants per role.
alter default privileges revoke execute on functions from public;

-- Identity -------------------------------------------------------------------

create table game.accounts (
  account_id uuid primary key references auth.users (id) on delete cascade,
  display_name text not null unique check (display_name ~ '^[A-Za-z0-9_-]{3,24}$'),
  name_changed_at timestamptz,
  status text not null default 'active'
    check (status in ('active', 'suspended', 'deleting', 'deleted')),
  created_at timestamptz not null default now()
);

create table game.identities (
  provider text not null check (provider in ('steam', 'psn', 'xbox', 'nintendo', 'dev')),
  provider_user_id text not null,
  account_id uuid not null references game.accounts (account_id) on delete cascade,
  created_at timestamptz not null default now(),
  primary key (provider, provider_user_id)
);
create index identities_account_idx on game.identities (account_id);

-- Phase 1 keeps only Grade, which the matchmaker needs for Grade pools.
-- Tonnage, the Weight and the rest arrive with Phase 2.
create table game.ledgers (
  account_id uuid primary key references game.accounts (account_id) on delete cascade,
  grade smallint not null default 1 check (grade between 1 and 6),
  updated_at timestamptz not null default now()
);

create table game.presence (
  account_id uuid primary key references game.accounts (account_id) on delete cascade,
  online boolean not null default true,
  updated_at timestamptz not null default now()
);

-- Crews ------------------------------------------------------------------------

create table game.crews (
  crew_id uuid primary key default gen_random_uuid(),
  leader_id uuid not null references game.accounts (account_id) on delete cascade,
  join_code text not null unique
    default upper(substr(replace(gen_random_uuid()::text, '-', ''), 1, 8)),
  -- Part of the crew-chat topic name. Rotated whenever a member leaves, so a removed member
  -- whose cached channel access outlives their membership is left talking to nobody.
  chat_key text not null default replace(gen_random_uuid()::text, '-', ''),
  created_at timestamptz not null default now()
);

create table game.crew_members (
  crew_id uuid not null references game.crews (crew_id) on delete cascade,
  account_id uuid not null unique references game.accounts (account_id) on delete cascade,
  harness text not null default 'Rigger'
    check (harness in ('Climber', 'Scrapper', 'Rigger', 'Doubler', 'Breaker')),
  face text,
  ready boolean not null default false,
  joined_at timestamptz not null default clock_timestamp(),
  primary key (crew_id, account_id)
);

-- A kicked player cannot rejoin by join code until this passes (PROPOSED: 10 minutes).
-- An invite from the crew still lets them back in.
create table game.crew_kicks (
  crew_id uuid not null references game.crews (crew_id) on delete cascade,
  account_id uuid not null references game.accounts (account_id) on delete cascade,
  until timestamptz not null,
  primary key (crew_id, account_id)
);

create table game.crew_invites (
  crew_id uuid not null references game.crews (crew_id) on delete cascade,
  from_id uuid not null references game.accounts (account_id) on delete cascade,
  to_id uuid not null references game.accounts (account_id) on delete cascade,
  expires_at timestamptz not null,
  primary key (crew_id, to_id)
);
create index crew_invites_to_idx on game.crew_invites (to_id);

-- Matchmaking --------------------------------------------------------------------

create table game.regions (
  region_id text primary key,
  beacon_host text not null,
  beacon_port integer not null,
  enabled boolean not null default true
);

create table game.playlists (
  playlist_id text primary key,
  mode text not null check (mode in ('scrap', 'claim', 'haul', 'free_scrap')),
  team_size smallint not null check (team_size between 1 and 3),
  teams smallint not null check (teams between 2 and 9),
  rated boolean not null default false,
  arenas text[] not null,
  regions text[] not null,
  -- Grade pools where bots may fill slots at match start (game-modes 6.1, 9).
  bot_fill_pools text[] not null default '{apprentice}',
  bot_fill_after_s integer not null default 60,
  enabled boolean not null default true
);

-- Unlogged: tickets are cheap to lose; clients re-queue on `queue_reset`.
create unlogged table game.mm_tickets (
  ticket_id uuid primary key default gen_random_uuid(),
  crew_id uuid not null unique,
  playlist_id text not null,
  grade_pool text not null check (grade_pool in ('apprentice', 'hand', 'journeyman', 'rated')),
  build text not null,
  members jsonb not null,
  pings_ms jsonb not null,
  created_at timestamptz not null default now()
);
create index mm_tickets_pool_idx on game.mm_tickets (playlist_id, build, created_at);

create table game.matches (
  match_id uuid primary key default gen_random_uuid(),
  playlist_id text not null references game.playlists (playlist_id),
  arena text not null,
  variant text not null default 'base',
  region text not null,
  build text not null,
  fleet text not null,
  -- allocation_id is ours (minted before the fleet call, carried in the server credential);
  -- fleet_ref is the provider's own id, needed to deallocate.
  allocation_id text,
  fleet_ref text,
  address text,
  state text not null default 'allocating'
    check (state in ('allocating', 'waiting_for_players', 'in_progress', 'reporting', 'completed', 'aborted')),
  abort_reason text,
  allocation_attempts smallint not null default 0,
  created_at timestamptz not null default now(),
  ready_at timestamptz,
  notified_at timestamptz,
  started_at timestamptz,
  ended_at timestamptz,
  last_heartbeat_at timestamptz,
  deallocated_at timestamptz,
  -- Tickets that formed this match, kept so a failed allocation can requeue them.
  source_tickets jsonb not null default '[]'
);
create index matches_state_idx on game.matches (state);

create table game.match_players (
  match_id uuid not null references game.matches (match_id) on delete cascade,
  slot smallint not null,
  team smallint not null,
  -- Null for bots, and for humans whose account was deleted (rows are kept, anonymized).
  account_id uuid references game.accounts (account_id) on delete set null,
  is_bot boolean not null default false,
  -- Mirrors whether the match is live, so "is this player in a match" is one index probe.
  live boolean not null default true,
  harness text not null,
  face text,
  loadout jsonb not null default '{}',
  connected boolean not null default false,
  joined_at timestamptz,
  left_at timestamptz,
  abandoned boolean not null default false,
  primary key (match_id, slot),
  unique (match_id, account_id),
  check (not (is_bot and account_id is not null))
);
create index match_players_account_idx on game.match_players (account_id);
create index match_players_live_idx on game.match_players (account_id) where live and not abandoned;

create function game.sync_match_live()
returns trigger
language plpgsql security definer set search_path = ''
as $$
begin
  update game.match_players
     set live = new.state in ('allocating', 'waiting_for_players', 'in_progress')
   where match_id = new.match_id;
  return null;
end
$$;
create trigger matches_sync_live
  after update of state on game.matches
  for each row when (old.state is distinct from new.state)
  execute function game.sync_match_live();

create table game.match_reports (
  match_id uuid primary key references game.matches (match_id),
  received_at timestamptz not null default now(),
  payload jsonb not null,
  raw_events_path text,
  replay_path text,
  status text not null default 'stored' check (status in ('stored', 'quarantined', 'processed')),
  quarantine_reason text
);

create table game.abandons (
  account_id uuid not null references game.accounts (account_id) on delete cascade,
  match_id uuid not null references game.matches (match_id) on delete cascade,
  at timestamptz not null default now(),
  primary key (account_id, match_id)
);

-- Moderation and config ------------------------------------------------------------

create table game.sanctions (
  sanction_id uuid primary key default gen_random_uuid(),
  account_id uuid not null references game.accounts (account_id) on delete cascade,
  type text not null check (type in
    ('chat_mute', 'voice_mute', 'queue_lock', 'rated_lock', 'matchmaking_ban', 'account_ban')),
  reason text not null,
  starts_at timestamptz not null default now(),
  ends_at timestamptz,
  issued_by text not null default 'system'
);
create index sanctions_account_idx on game.sanctions (account_id);

create table game.config (
  key text primary key,
  value jsonb not null
);

-- Rate limiting (section 15) --------------------------------------------------------

create unlogged table private.rate_limits (
  key text primary key,
  window_start timestamptz not null,
  hits integer not null
);

-- Row-level security --------------------------------------------------------------
-- Every game table has RLS on. Clients only reach them through `api` views and
-- functions; the policies below are what the security_invoker views read through.

alter table game.accounts enable row level security;
alter table game.identities enable row level security;
alter table game.ledgers enable row level security;
alter table game.presence enable row level security;
alter table game.crews enable row level security;
alter table game.crew_members enable row level security;
alter table game.crew_invites enable row level security;
alter table game.crew_kicks enable row level security;
alter table game.regions enable row level security;
alter table game.playlists enable row level security;
alter table game.mm_tickets enable row level security;
alter table game.matches enable row level security;
alter table game.match_players enable row level security;
alter table game.match_reports enable row level security;
alter table game.abandons enable row level security;
alter table game.sanctions enable row level security;
alter table game.config enable row level security;
alter table private.rate_limits enable row level security;

-- Bypasses RLS so crew policies do not recurse into crew_members.
create function private.my_crew_id()
returns uuid
language sql stable security definer set search_path = ''
as $$
  select crew_id from game.crew_members where account_id = (select auth.uid())
$$;
grant execute on function private.my_crew_id() to authenticated;

create policy accounts_own on game.accounts
  for select to authenticated using (account_id = (select auth.uid()));
create policy accounts_crewmates on game.accounts
  for select to authenticated using (
    account_id in (select m.account_id from game.crew_members m
                   where m.crew_id = (select private.my_crew_id()))
  );
create policy ledgers_own on game.ledgers
  for select to authenticated using (account_id = (select auth.uid()));
create policy crews_member on game.crews
  for select to authenticated using (crew_id = (select private.my_crew_id()));
create policy crew_members_same_crew on game.crew_members
  for select to authenticated using (crew_id = (select private.my_crew_id()));
create policy crew_invites_mine on game.crew_invites
  for select to authenticated
  using (to_id = (select auth.uid()) or from_id = (select auth.uid()));

grant select on game.accounts, game.ledgers, game.crews, game.crew_members, game.crew_invites
  to authenticated;

-- Read views for clients ------------------------------------------------------------

create view api.my_account with (security_invoker = true) as
  select a.account_id, a.display_name, a.status, a.created_at, l.grade
  from game.accounts a
  join game.ledgers l using (account_id);

create view api.my_crew with (security_invoker = true) as
  select c.crew_id, c.leader_id, c.join_code, 'crew-chat:' || c.crew_id || ':' || c.chat_key as chat_topic,
         m.account_id, a.display_name,
         m.harness, m.face, m.ready, m.joined_at
  from game.crews c
  join game.crew_members m using (crew_id)
  join game.accounts a on a.account_id = m.account_id;

create view api.my_invites with (security_invoker = true) as
  select i.crew_id, i.from_id, i.expires_at
  from game.crew_invites i
  where i.to_id = (select auth.uid()) and i.expires_at > now();

grant select on api.my_account, api.my_crew, api.my_invites to authenticated;

-- Storage buckets for replays and raw match events (section 19).
insert into storage.buckets (id, name, public)
values ('replays', 'replays', false), ('match-events', 'match-events', false)
on conflict (id) do nothing;
