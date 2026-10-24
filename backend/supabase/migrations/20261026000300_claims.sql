-- Phase 2, Claims (backend-architecture sections 7.5 and 14; game-modes 7; bible/04): seasons,
-- Allegiance, weekly Contracts and their progress, Standing points and faction standing.
-- The worker's rewards pass writes Standing and Contract progress per match; its claims pass
-- moves each Claim through scheduled, active, frozen and tallied, and refreshes standing
-- hourly. A person reviews a tallied Claim and resolves it from the admin console.

create table game.claims (
  claim_id text primary key check (claim_id ~ '^[a-z0-9-]{1,40}$'),
  name text not null,
  -- The arena whose next season the outcome changes (PROPOSED codes: YRD, HGT, BLS, ANC).
  arena text not null check (arena in ('YRD', 'HGT', 'BLS', 'ANC')),
  state text not null default 'scheduled'
    check (state in ('scheduled', 'active', 'frozen', 'tallied', 'reviewed', 'resolved', 'archived')),
  starts_at timestamptz not null,
  ends_at timestamptz not null,
  -- Public once resolved: the holder, the variant and the standings.
  outcome jsonb,
  -- The tally's notes for the reviewer (anomalies, leftover reports). Never shown to players.
  review jsonb,
  reviewed_by text,
  check (ends_at > starts_at),
  -- Seasons never overlap, so every report belongs to at most one Claim.
  exclude using gist (tstzrange(starts_at, ends_at) with &&)
);
-- At most one Claim is active at a time.
create unique index claims_one_active on game.claims ((true)) where state = 'active';

create table game.allegiances (
  account_id uuid not null references game.accounts (account_id) on delete cascade,
  claim_id text not null references game.claims (claim_id),
  faction text not null check (faction in ('guild', 'htr', 'ascendancy', 'hangers', 'fray')),
  declared_at timestamptz not null default now(),
  primary key (account_id, claim_id)
);

-- Every declaration, so a match counts for the faction the player held when it was stored.
create table game.allegiance_history (
  account_id uuid not null references game.accounts (account_id) on delete cascade,
  claim_id text not null references game.claims (claim_id),
  faction text not null check (faction in ('guild', 'htr', 'ascendancy', 'hangers', 'fray')),
  declared_at timestamptz not null default clock_timestamp(),
  primary key (account_id, claim_id, declared_at)
);

create table game.contracts (
  contract_id text primary key check (contract_id ~ '^[a-z0-9-]{1,60}$'),
  claim_id text not null references game.claims (claim_id),
  faction text not null check (faction in ('guild', 'htr', 'ascendancy', 'hangers', 'fray')),
  week smallint not null check (week >= 1),
  -- A Contract counter from the match report (bible/04's objectives).
  counter_key text not null,
  target numeric not null check (target > 0),
  title text not null,
  -- {"items": [item ids], "entries": [Ledger entry ids]}
  reward jsonb not null default '{}'
);
create index contracts_week_idx on game.contracts (claim_id, faction, week);

-- A reward is {"items": [...], "entries": [...]}: item ids that exist, are not retired and may
-- be granted by a Contract; entry ids as strings. A bad reward is refused here, never at the
-- moment a player completes the Contract.
create function private.check_reward(p_reward jsonb, p_rule text)
returns void
language plpgsql stable security definer set search_path = ''
as $$
declare
  v_key text;
  v_item text;
begin
  if jsonb_typeof(p_reward) <> 'object' then
    raise exception 'bad_reward';
  end if;
  for v_key in select jsonb_object_keys(p_reward) loop
    if v_key not in ('items', 'entries') or jsonb_typeof(p_reward -> v_key) <> 'array'
       or exists (select 1 from jsonb_array_elements(p_reward -> v_key) e where jsonb_typeof(e) <> 'string') then
      raise exception 'bad_reward' using hint = v_key;
    end if;
  end loop;
  for v_item in select jsonb_array_elements_text(coalesce(p_reward -> 'items', '[]')) loop
    if not exists (select 1 from game.items i where i.item_id = v_item and not i.retired and i.source_rule = p_rule) then
      raise exception 'not_a_reward_item' using hint = v_item;
    end if;
  end loop;
end
$$;

create function private.check_contract()
returns trigger
language plpgsql security definer set search_path = ''
as $$
begin
  perform private.check_reward(new.reward, 'contract');
  return new;
end
$$;
create trigger contracts_reward
  before insert or update of reward on game.contracts
  for each row execute function private.check_contract();

create table game.contract_progress (
  account_id uuid not null references game.accounts (account_id) on delete cascade,
  contract_id text not null references game.contracts (contract_id),
  progress numeric not null default 0 check (progress >= 0),
  completed_at timestamptz,
  primary key (account_id, contract_id)
);

-- Append-only. The partial unique indexes make every grant idempotent.
create table game.claim_standing_events (
  event_id bigint generated always as identity primary key,
  claim_id text not null references game.claims (claim_id),
  faction text not null check (faction in ('guild', 'htr', 'ascendancy', 'hangers', 'fray')),
  points numeric not null check (points >= 0),
  reason text not null check (reason in ('match', 'contract', 'evidence')),
  match_id uuid references game.matches (match_id) on delete set null,
  contract_id text references game.contracts (contract_id),
  -- Null once the account is deleted: the faction keeps the points.
  account_id uuid references game.accounts (account_id) on delete set null,
  week smallint not null check (week >= 1),
  rated_win boolean not null default false,
  created_at timestamptz not null default now()
);
create unique index standing_match_once on game.claim_standing_events (claim_id, account_id, match_id)
  where reason = 'match';
create unique index standing_contract_once on game.claim_standing_events (claim_id, account_id, contract_id)
  where reason = 'contract';
create unique index standing_evidence_once on game.claim_standing_events (claim_id, account_id)
  where reason = 'evidence';
create index standing_week_idx on game.claim_standing_events (claim_id, account_id, week);

create table game.claim_standing (
  claim_id text not null references game.claims (claim_id),
  faction text not null check (faction in ('guild', 'htr', 'ascendancy', 'hangers', 'fray')),
  points_total numeric not null default 0,
  active_players integer not null default 0,
  standing numeric not null default 0,
  rated_wins integer not null default 0,
  refreshed_at timestamptz not null default now(),
  primary key (claim_id, faction)
);

-- The append-only rule. The only change allowed is the foreign keys' own cleanup: an account
-- or match deleted sets its id to null, and the points stay with the faction.
create function private.standing_events_append_only()
returns trigger
language plpgsql security definer set search_path = ''
as $$
begin
  if tg_op = 'UPDATE'
     and (to_jsonb(old) - 'account_id' - 'match_id') = (to_jsonb(new) - 'account_id' - 'match_id')
     and (new.account_id is null or new.account_id = old.account_id)
     and (new.match_id is null or new.match_id = old.match_id) then
    return new;
  end if;
  raise exception 'standing_events_are_append_only';
end
$$;
create trigger standing_events_append_only
  before update or delete on game.claim_standing_events
  for each row execute function private.standing_events_append_only();

alter table game.claims enable row level security;
alter table game.allegiances enable row level security;
alter table game.allegiance_history enable row level security;
alter table game.contracts enable row level security;
alter table game.contract_progress enable row level security;
alter table game.claim_standing_events enable row level security;
alter table game.claim_standing enable row level security;

create policy claims_visible on game.claims for select to authenticated using (state <> 'scheduled');
create policy claim_standing_all on game.claim_standing for select to authenticated using (true);
create policy allegiances_own on game.allegiances
  for select to authenticated using (account_id = (select auth.uid()));
create policy contract_progress_own on game.contract_progress
  for select to authenticated using (account_id = (select auth.uid()));
create policy standing_events_own on game.claim_standing_events
  for select to authenticated using (account_id = (select auth.uid()));
-- Contracts are visible for the player's declared faction, up to the current week only.
create function private.my_open_contract(p_claim text, p_faction text, p_week smallint)
returns boolean
language sql stable security definer set search_path = ''
as $$
  select exists (
    select 1 from game.allegiances a join game.claims c using (claim_id)
     where a.account_id = (select auth.uid()) and a.claim_id = p_claim and a.faction = p_faction
       and c.state <> 'scheduled'
       and p_week <= floor(extract(epoch from (least(now(), c.ends_at) - c.starts_at)) / 604800) + 1)
$$;
create policy contracts_mine on game.contracts
  for select to authenticated using ((select private.my_open_contract(claim_id, faction, week)));

-- The review notes stay with the service role.
revoke select on game.claims from authenticated;
grant select (claim_id, name, arena, state, starts_at, ends_at, outcome) on game.claims to authenticated;
grant select on game.claim_standing, game.allegiances, game.contracts, game.contract_progress,
  game.claim_standing_events to authenticated;

-- Helpers -----------------------------------------------------------------------------

create function private.active_claim()
returns game.claims
language sql stable security definer set search_path = ''
as $$
  select * from game.claims where state = 'active' and now() >= starts_at and now() < ends_at
$$;

-- Client functions --------------------------------------------------------------------

-- One faction per Claim. Switching keeps what was earned and resets Contract progress that is
-- not complete (bible/04: "The Guild remembers").
create function api.declare_allegiance(p_faction text)
returns jsonb
language plpgsql security definer set search_path = ''
as $$
declare
  v_uid uuid := private.require_uid();
  v_claim game.claims := private.active_claim();
  v_before text;
begin
  perform private.require_rate('allegiance:' || v_uid::text, 5, interval '1 minute');
  if p_faction is null or p_faction not in ('guild', 'htr', 'ascendancy', 'hangers', 'fray') then
    raise exception 'unknown_faction';
  end if;
  if v_claim.claim_id is null then
    raise exception 'no_active_claim';
  end if;
  select faction into v_before from game.allegiances
   where account_id = v_uid and claim_id = v_claim.claim_id for update;
  if v_before = p_faction then
    return jsonb_build_object('claim_id', v_claim.claim_id, 'faction', p_faction, 'switched', false);
  end if;
  insert into game.allegiances (account_id, claim_id, faction) values (v_uid, v_claim.claim_id, p_faction)
  on conflict (account_id, claim_id) do update set faction = excluded.faction, declared_at = now();
  insert into game.allegiance_history (account_id, claim_id, faction) values (v_uid, v_claim.claim_id, p_faction);
  if v_before is not null then
    delete from game.contract_progress p
     using game.contracts c
     where p.account_id = v_uid and p.contract_id = c.contract_id and c.claim_id = v_claim.claim_id
       and p.completed_at is null;
  end if;
  return jsonb_build_object('claim_id', v_claim.claim_id, 'faction', p_faction, 'switched', v_before is not null);
end
$$;

-- Service functions (admin console) ---------------------------------------------------------

-- The variants an arena can take next season (bible/04); mirrors worker/claims.ts arenaVariant.
create function private.arena_variant_ok(p_arena text, p_variant text)
returns boolean
language sql immutable
set search_path = ''
as $$
  select case p_arena
    when 'HGT' then p_variant in ('base', 'district_removed')
    when 'BLS' then p_variant in ('base', 'crown_and_spine', 'stabilized')
    when 'YRD' then p_variant in ('base', 'haulers_doubled')
    when 'ANC' then p_variant in ('colors_guild', 'colors_htr', 'colors_ascendancy', 'colors_hangers', 'colors_fray')
    else false end
$$;

-- A person signs off a tallied Claim (14: reviewed). For a tie the tally could not break, the
-- reviewer names the holder and the variant it gives the arena; the override is recorded.
create function api.srv_claim_review(p_claim text, p_staff text, p_holder text default null,
                                     p_variant text default null)
returns jsonb
language plpgsql security definer set search_path = ''
as $$
begin
  if p_staff is null or length(p_staff) = 0 then
    raise exception 'staff_required';
  end if;
  if (p_holder is null) <> (p_variant is null)
     or (p_holder is not null and p_holder not in ('guild', 'htr', 'ascendancy', 'hangers', 'fray'))
     or (p_variant is not null
         and not private.arena_variant_ok((select arena from game.claims where claim_id = p_claim), p_variant)) then
    raise exception 'bad_override';
  end if;
  update game.claims
     set state = 'reviewed', reviewed_by = p_staff,
         outcome = case when p_holder is null then outcome
                        else coalesce(outcome, '{}') || jsonb_build_object('holder', p_holder, 'variant', p_variant) end,
         review = case when p_holder is null then review
                       else coalesce(review, '{}') || jsonb_build_object('holder_set_by', p_staff) end
   where claim_id = p_claim and state = 'tallied';
  if not found then
    raise exception 'not_tallied';
  end if;
  return jsonb_build_object('claim_id', p_claim, 'state', 'reviewed');
end
$$;

-- Applies a reviewed Claim's outcome: the arena variant flag for the next season goes into
-- config (`variant:<arena>`), and the Claim is resolved. A Claim with no holder changes nothing.
create function api.srv_claim_resolve(p_claim text)
returns jsonb
language plpgsql security definer set search_path = ''
as $$
declare
  v_claim game.claims;
begin
  select * into v_claim from game.claims where claim_id = p_claim and state = 'reviewed' for update;
  if not found then
    raise exception 'not_reviewed';
  end if;
  if v_claim.outcome ->> 'variant' is not null then
    insert into game.config (key, value) values ('variant:' || v_claim.arena, to_jsonb(v_claim.outcome ->> 'variant'))
    on conflict (key) do update set value = excluded.value;
  end if;
  update game.claims set state = 'resolved' where claim_id = p_claim;
  return jsonb_build_object('claim_id', p_claim, 'state', 'resolved', 'variant', v_claim.outcome ->> 'variant');
end
$$;

-- A match's Standing for the player: {faction, points}, null outside a Claim or Allegiance.
alter table game.match_summaries add column standing jsonb;
create or replace view api.my_match_summaries with (security_invoker = true) as
  select s.match_id, s.ruleset, s.win, s.tonnage, s.credit, s.breakdown, s.grade_before, s.grade_after,
         s.weight_before, s.weight_after, s.rope_marks, s.unlocks, s.contracts, s.created_at, s.standing
  from game.match_summaries s;

-- Read views for clients ----------------------------------------------------------------

create view api.claims with (security_invoker = true) as
  select c.claim_id, c.name, c.arena, c.state, c.starts_at, c.ends_at,
         case when c.state in ('resolved', 'archived')
              then jsonb_build_object('holder', c.outcome -> 'holder', 'variant', c.outcome -> 'variant',
                                      'standings', c.outcome -> 'standings') end as outcome
    from game.claims c;

create view api.claim_standing with (security_invoker = true) as
  select s.claim_id, s.faction, round(s.standing, 1) as standing, s.active_players, s.refreshed_at
    from game.claim_standing s;

create view api.my_allegiance with (security_invoker = true) as
  select a.claim_id, a.faction, a.declared_at from game.allegiances a;

create view api.my_contracts with (security_invoker = true) as
  select c.contract_id, c.claim_id, c.faction, c.week, c.title, c.counter_key, c.target, c.reward,
         coalesce(p.progress, 0) as progress, p.completed_at
    from game.contracts c
    left join game.contract_progress p on p.contract_id = c.contract_id and p.account_id = (select auth.uid());

create view api.my_standing with (security_invoker = true) as
  select e.claim_id, e.week, e.reason, sum(e.points) as points
    from game.claim_standing_events e
   group by e.claim_id, e.week, e.reason;

grant select on api.claims, api.claim_standing, api.my_allegiance, api.my_contracts, api.my_standing
  to authenticated;

-- Grants ------------------------------------------------------------------------------
revoke execute on function
  private.check_reward(jsonb, text), private.check_contract(), private.arena_variant_ok(text, text),
  private.standing_events_append_only(), private.my_open_contract(text, text, smallint), private.active_claim(),
  api.declare_allegiance(text), api.srv_claim_review(text, text, text, text),
  api.srv_claim_resolve(text)
  from public, anon, authenticated;
grant execute on function private.my_open_contract(text, text, smallint) to authenticated;
grant execute on function api.declare_allegiance(text) to authenticated;
grant execute on function api.srv_claim_review(text, text, text, text), api.srv_claim_resolve(text) to service_role;
