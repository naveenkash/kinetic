-- Phase 2, rewards: the Ledger, Rope Marks, Ledger entries, career totals and match summaries
-- (backend-architecture sections 7.3 and 12.3). The worker's rewards pass writes them from
-- stored match reports, one report per transaction, with the ruleset in force (`ruleset` in
-- game.config; the coefficients live in backend/ruleset/). Clients read their own rows only.

alter table game.ledgers
  add column tonnage_total bigint not null default 0 check (tonnage_total >= 0),
  -- Null until the first rewarded match: the player still owes the full lease.
  add column weight_balance bigint check (weight_balance >= 0),
  add column cleared_at timestamptz,
  add column cutter_serial text unique,
  add column cutter_issued_at timestamptz;

-- Cutter serials, issued at the Cutter ceremony (Greenhand to Hand, bible/04).
create sequence game.cutter_serial_seq start 1001;

create table game.rope_marks (
  account_id uuid not null references game.accounts (account_id) on delete cascade,
  rig text not null,
  count integer not null check (count >= 0),
  tier text not null check (tier in ('none', 'tape', 'band', 'mark')),
  updated_at timestamptz not null default now(),
  primary key (account_id, rig)
);

create table game.ledger_entries (
  account_id uuid not null references game.accounts (account_id) on delete cascade,
  entry_id text not null,
  unlocked_at timestamptz not null default now(),
  match_id uuid references game.matches (match_id) on delete set null,
  primary key (account_id, entry_id)
);

create table game.career_stats (
  account_id uuid not null references game.accounts (account_id) on delete cascade,
  harness text not null check (harness in ('Climber', 'Scrapper', 'Rigger', 'Doubler', 'Breaker')),
  matches integer not null default 0,
  wins integer not null default 0,
  scraps integer not null default 0,
  assists integer not null default 0,
  spotter_credits integer not null default 0,
  ringouts integer not null default 0,
  mass_distance_kg_m numeric not null default 0,
  rated_tons_moved numeric not null default 0,
  floor_seconds numeric not null default 0,
  airborne_seconds numeric not null default 0,
  hoisted_seconds numeric not null default 0,
  primary key (account_id, harness)
);

create table game.match_summaries (
  match_id uuid not null references game.matches (match_id) on delete cascade,
  account_id uuid not null references game.accounts (account_id) on delete cascade,
  ruleset smallint not null,
  win boolean not null,
  tonnage integer not null check (tonnage >= 0),
  -- Tonnage credit paid into the wallet (section 13.3).
  credit integer not null default 0 check (credit >= 0),
  breakdown jsonb not null,
  grade_before smallint not null,
  grade_after smallint not null,
  weight_before bigint not null,
  weight_after bigint not null,
  rope_marks jsonb not null default '[]',
  unlocks jsonb not null default '[]',
  -- Filled when Contracts arrive (section 12.3 step 4).
  contracts jsonb not null default '[]',
  created_at timestamptz not null default now(),
  primary key (match_id, account_id)
);
create index match_summaries_account_idx on game.match_summaries (account_id, created_at desc);

alter table game.match_reports
  add column ruleset smallint,
  add column processed_at timestamptz,
  -- Attempts count failures the report causes; failures count every failure, for the backoff.
  add column process_attempts smallint not null default 0,
  add column failures integer not null default 0,
  add column next_attempt_at timestamptz,
  add column last_error text;
create index match_reports_stored_idx on game.match_reports (received_at) where status = 'stored';

-- The economy version in force. Required: the worker rewards nothing without it, so a new
-- ruleset file never switches the economy by itself.
insert into game.config (key, value) values ('ruleset', '1') on conflict (key) do nothing;

-- Copies of backend/ruleset/*.json, loaded by `deno task content`, so clients can show Grade
-- progress, the Weight owed and the next Rope Mark threshold. The worker rewards from the files.
create table game.rulesets (
  version smallint primary key,
  rules jsonb not null,
  loaded_at timestamptz not null default now()
);
alter table game.rulesets enable row level security;
create policy rulesets_all on game.rulesets for select to authenticated using (true);
grant select on game.rulesets to authenticated;

-- game.config has no client policy; the view reads the version through this function.
create function private.ruleset_version()
returns integer
language sql stable security definer set search_path = ''
as $$
  select case when jsonb_typeof(value) = 'number' then (value #>> '{}')::int end
    from game.config where key = 'ruleset'
$$;
revoke execute on function private.ruleset_version() from public, anon;
grant execute on function private.ruleset_version() to authenticated;

create view api.current_ruleset with (security_invoker = true) as
  select r.version, r.rules -> 'tonnage' as tonnage, r.rules -> 'grade_thresholds_t' as grade_thresholds_t,
         (r.rules ->> 'weight_start_t')::bigint as weight_start_t, r.rules -> 'rope_mark_tiers' as rope_mark_tiers
    from game.rulesets r
   where r.version = (select private.ruleset_version());
grant select on api.current_ruleset to authenticated;


alter table game.rope_marks enable row level security;
alter table game.ledger_entries enable row level security;
alter table game.career_stats enable row level security;
alter table game.match_summaries enable row level security;

create policy rope_marks_own on game.rope_marks
  for select to authenticated using (account_id = (select auth.uid()));
create policy ledger_entries_own on game.ledger_entries
  for select to authenticated using (account_id = (select auth.uid()));
create policy career_stats_own on game.career_stats
  for select to authenticated using (account_id = (select auth.uid()));
create policy match_summaries_own on game.match_summaries
  for select to authenticated using (account_id = (select auth.uid()));

grant select on game.rope_marks, game.ledger_entries, game.career_stats, game.match_summaries to authenticated;

-- Read views for clients ------------------------------------------------------------

create view api.my_ledger with (security_invoker = true) as
  select l.account_id, l.grade, l.tonnage_total, l.weight_balance, l.cleared_at,
         l.cutter_serial, l.cutter_issued_at, l.updated_at
  from game.ledgers l;

create view api.my_rope_marks with (security_invoker = true) as
  select r.rig, r.count, r.tier, r.updated_at from game.rope_marks r;

create view api.my_ledger_entries with (security_invoker = true) as
  select e.entry_id, e.unlocked_at, e.match_id from game.ledger_entries e;

create view api.my_career_stats with (security_invoker = true) as
  select c.harness, c.matches, c.wins, c.scraps, c.assists, c.spotter_credits, c.ringouts,
         c.mass_distance_kg_m, c.rated_tons_moved, c.floor_seconds, c.airborne_seconds, c.hoisted_seconds
  from game.career_stats c;

create view api.my_match_summaries with (security_invoker = true) as
  select s.match_id, s.ruleset, s.win, s.tonnage, s.credit, s.breakdown, s.grade_before, s.grade_after,
         s.weight_before, s.weight_after, s.rope_marks, s.unlocks, s.contracts, s.created_at
  from game.match_summaries s;

grant select on api.my_ledger, api.my_rope_marks, api.my_ledger_entries, api.my_career_stats,
  api.my_match_summaries to authenticated;

-- The manifest tells the game server the economy version in force (it was fixed at 1).
create or replace function api.srv_manifest(p_match uuid, p_allocation text)
returns jsonb
language plpgsql stable security definer set search_path = ''
as $$
declare
  v_match game.matches;
  v_playlist game.playlists;
begin
  select * into v_match from game.matches where match_id = p_match;
  if not found or v_match.allocation_id is distinct from p_allocation then
    raise exception 'unknown_match';
  end if;
  if v_match.state in ('completed', 'aborted') then
    raise exception 'match_closed';
  end if;
  select * into v_playlist from game.playlists where playlist_id = v_match.playlist_id;
  return jsonb_build_object(
    'match_id', v_match.match_id,
    'playlist_id', v_playlist.playlist_id,
    'mode', v_playlist.mode,
    'rated', v_playlist.rated,
    'arena', v_match.arena,
    'variant', v_match.variant,
    'region', v_match.region,
    'build', v_match.build,
    'team_size', v_playlist.team_size,
    'teams', v_playlist.teams,
    'reconnect_window_s', case when v_playlist.rated then 90 else 60 end,
    'event_schema', 1,
    'ruleset', private.ruleset_version(),
    'players', (
      select jsonb_agg(jsonb_build_object(
               'slot', mp.slot, 'team', mp.team, 'account_id', mp.account_id,
               'is_bot', mp.is_bot,
               'display_name', coalesce(a.display_name, 'Office crew ' || mp.slot),
               'harness', mp.harness, 'face', mp.face, 'loadout', mp.loadout)
             order by mp.slot)
      from game.match_players mp
      left join game.accounts a on a.account_id = mp.account_id
      where mp.match_id = p_match));
end
$$;


-- Puts a quarantined or failed report back on the queue, for the admin console once a cause
-- is fixed: attempts reset, rewarded on the next pass.
create function api.srv_requeue_report(p_match uuid)
returns jsonb
language plpgsql security definer set search_path = ''
as $$
begin
  update game.match_reports
     set status = 'stored', quarantine_reason = null, process_attempts = 0, failures = 0, next_attempt_at = null,
         last_error = null
   where match_id = p_match and status = 'quarantined';
  if not found then
    raise exception 'not_quarantined';
  end if;
  return jsonb_build_object('status', 'stored');
end
$$;
revoke execute on function api.srv_requeue_report(uuid) from public, anon, authenticated;
grant execute on function api.srv_requeue_report(uuid) to service_role;
