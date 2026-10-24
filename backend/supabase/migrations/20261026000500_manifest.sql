-- Phase 2, the Manifest (backend-architecture 7.4, 7.5 and 12.3 step 5; bible/04; menus 9): a
-- per-Claim salvage manifest of crates in two columns, Free and Sponsored, ticked off by the
-- Tonnage a player earns during the Claim. The Sponsored column pays only to players holding
-- the `sponsorship:<claim_id>` entitlement; crates passed before sponsoring are paid when it is
-- granted. Crates hold cosmetics and Ledger entries only: never a Line, Dog or Cutter. Every
-- paid crate is recorded, so a crate added mid-Claim below a player's total is paid at their
-- next match.

create table game.entitlements (
  account_id uuid not null references game.accounts (account_id) on delete cascade,
  entitlement text not null check (entitlement ~ '^[a-z]+:[a-z0-9-]+$'),
  granted_at timestamptz not null default now(),
  source_ref text,
  primary key (account_id, entitlement)
);

create table game.manifest_crates (
  claim_id text not null references game.claims (claim_id),
  track text not null check (track in ('free', 'sponsored')),
  tier smallint not null check (tier >= 1),
  -- Tonnage earned during the Claim that opens the crate.
  tonnage_t integer not null check (tonnage_t > 0),
  -- {"items": [item ids], "entries": [Ledger entry ids]}
  reward jsonb not null default '{}',
  primary key (claim_id, track, tier)
);

create table game.manifest_progress (
  account_id uuid not null references game.accounts (account_id) on delete cascade,
  claim_id text not null references game.claims (claim_id),
  tonnage_t bigint not null default 0 check (tonnage_t >= 0),
  primary key (account_id, claim_id)
);

create table game.manifest_paid (
  account_id uuid not null references game.accounts (account_id) on delete cascade,
  claim_id text not null,
  track text not null,
  tier smallint not null,
  paid_at timestamptz not null default now(),
  primary key (account_id, claim_id, track, tier),
  foreign key (claim_id, track, tier) references game.manifest_crates (claim_id, track, tier)
);

-- Crate items must be Manifest cosmetics (bible/04: "No Line, Dog, or Cutter ever lives in the
-- Manifest"), and tiers must open in order.
create function private.check_manifest_crate()
returns trigger
language plpgsql security definer set search_path = ''
as $$
begin
  perform private.check_reward(new.reward, 'manifest');
  if exists (select 1 from game.manifest_crates c
              where c.claim_id = new.claim_id and c.track = new.track and c.tier <> new.tier
                and (c.tier < new.tier) <> (c.tonnage_t < new.tonnage_t)) then
    raise exception 'crates_out_of_order';
  end if;
  return new;
end
$$;
create trigger manifest_crates_check
  before insert or update on game.manifest_crates
  for each row execute function private.check_manifest_crate();

alter table game.entitlements enable row level security;
alter table game.manifest_crates enable row level security;
alter table game.manifest_progress enable row level security;
alter table game.manifest_paid enable row level security;

create policy entitlements_own on game.entitlements
  for select to authenticated using (account_id = (select auth.uid()));
create policy manifest_crates_visible on game.manifest_crates
  for select to authenticated using (exists (
    select 1 from game.claims c where c.claim_id = manifest_crates.claim_id and c.state <> 'scheduled'));
create policy manifest_progress_own on game.manifest_progress
  for select to authenticated using (account_id = (select auth.uid()));
create policy manifest_paid_own on game.manifest_paid
  for select to authenticated using (account_id = (select auth.uid()));
grant select on game.entitlements, game.manifest_crates, game.manifest_progress, game.manifest_paid to authenticated;

-- Pays every crate the player has reached and not been paid: Free always, Sponsored with the
-- Claim's sponsorship. Returns the tiers paid now, per column.
create function private.pay_manifest(p_account uuid, p_claim text)
returns jsonb
language plpgsql security definer set search_path = ''
as $$
declare
  v_total bigint;
  v_sponsored boolean;
  v_crate game.manifest_crates;
  v_ref text;
  v_free integer[] := '{}';
  v_paid_sponsored integer[] := '{}';
begin
  select tonnage_t into v_total from game.manifest_progress
   where account_id = p_account and claim_id = p_claim for update;
  v_total := coalesce(v_total, 0);
  v_sponsored := exists (select 1 from game.entitlements
                          where account_id = p_account and entitlement = 'sponsorship:' || p_claim);
  for v_crate in
    select c.* from game.manifest_crates c
     where c.claim_id = p_claim and c.tonnage_t <= v_total and (c.track = 'free' or v_sponsored)
       and not exists (select 1 from game.manifest_paid p
                        where p.account_id = p_account and p.claim_id = c.claim_id and p.track = c.track
                          and p.tier = c.tier)
     order by c.track, c.tier
  loop
    v_ref := 'manifest:' || p_claim || ':' || v_crate.track || ':' || v_crate.tier;
    insert into game.inventory (account_id, item_id, source, source_ref)
    select p_account, i, 'manifest', v_ref from jsonb_array_elements_text(coalesce(v_crate.reward -> 'items', '[]')) i
    on conflict do nothing;
    insert into game.ledger_entries (account_id, entry_id)
    select p_account, e from jsonb_array_elements_text(coalesce(v_crate.reward -> 'entries', '[]')) e
    on conflict do nothing;
    insert into game.manifest_paid (account_id, claim_id, track, tier)
    values (p_account, p_claim, v_crate.track, v_crate.tier);
    if v_crate.track = 'free' then
      v_free := v_free || v_crate.tier::integer;
    else
      v_paid_sponsored := v_paid_sponsored || v_crate.tier::integer;
    end if;
  end loop;
  return jsonb_build_object('tonnage_t', v_total, 'free', to_jsonb(v_free), 'sponsored', to_jsonb(v_paid_sponsored));
end
$$;

-- Sponsoring pays the Sponsored crates already passed at once, even with no match to come.
create function private.pay_manifest_on_sponsorship()
returns trigger
language plpgsql security definer set search_path = ''
as $$
begin
  if new.entitlement like 'sponsorship:%' then
    perform private.pay_manifest(new.account_id, substr(new.entitlement, length('sponsorship:') + 1));
  end if;
  return new;
end
$$;
create trigger entitlements_pay_manifest
  after insert on game.entitlements
  for each row execute function private.pay_manifest_on_sponsorship();

create view api.my_entitlements with (security_invoker = true) as
  select e.entitlement, e.granted_at from game.entitlements e;

-- The Manifest as the player sees it: every crate of each Claim, ticked or not.
create view api.my_manifest with (security_invoker = true) as
  select c.claim_id, c.track, c.tier, c.tonnage_t, c.reward,
         exists (select 1 from game.manifest_paid d
                  where d.account_id = (select auth.uid()) and d.claim_id = c.claim_id and d.track = c.track
                    and d.tier = c.tier) as earned,
         coalesce(p.tonnage_t, 0) as tonnage_earned
    from game.manifest_crates c
    left join game.manifest_progress p on p.claim_id = c.claim_id and p.account_id = (select auth.uid());

grant select on api.my_entitlements, api.my_manifest to authenticated;

-- The Manifest a match paid, on its summary.
alter table game.match_summaries add column manifest jsonb;
create or replace view api.my_match_summaries with (security_invoker = true) as
  select s.match_id, s.ruleset, s.win, s.tonnage, s.credit, s.breakdown, s.grade_before, s.grade_after,
         s.weight_before, s.weight_after, s.rope_marks, s.unlocks, s.contracts, s.created_at, s.standing, s.rating,
         s.manifest
  from game.match_summaries s;

revoke execute on function private.check_manifest_crate(), private.pay_manifest(uuid, text),
  private.pay_manifest_on_sponsorship() from public, anon, authenticated;
