-- Phase 2, inventory and economy (backend-architecture sections 7.4 and 13): the item catalog,
-- what each player owns, loadouts, wallets of Tonnage credit and sponsor credit, the shop
-- rotation and the purchase function. Rated power never moves (bible/04): sidegrades (Lines,
-- Dogs, Cutters) are unlocked by Grade or a Contract only, never sold or shipped in the
-- Manifest, and the database refuses a catalog that says otherwise.

create table game.items (
  item_id text primary key check (item_id ~ '^[a-z_]+\.[a-z0-9_]+$'),
  slot text not null check (slot in (
    'line', 'dog', 'cutter',
    'livery', 'tape', 'dogs', 'cutter_skin', 'rig_call', 'drop', 'scrap_card', 'voice', 'reserve_livery')),
  source_rule text not null check (source_rule in ('grade', 'contract', 'manifest', 'shop', 'admin')),
  -- The Grade that unlocks it (grade items), or the minimum Grade to equip it.
  grade_gated smallint check (grade_gated between 1 and 6),
  metadata jsonb not null default '{}',
  retired boolean not null default false,
  check (source_rule <> 'grade' or grade_gated is not null),
  -- The sidegrade pool is identical for every player at the same Grade (bible/04): a
  -- sidegrade is unlocked by its Grade or earned from a Contract, nothing else.
  check (slot not in ('line', 'dog', 'cutter') or (source_rule in ('grade', 'contract') and grade_gated is not null))
);

-- The baseline Spool, which every loadout falls back to. The rest of the catalog is loaded
-- from backend/catalog/ by `deno task catalog`.
insert into game.items (item_id, slot, source_rule, grade_gated) values
  ('line.standard', 'line', 'grade', 1), ('dog.standard', 'dog', 'grade', 1), ('cutter.standard', 'cutter', 'grade', 1);

create table game.inventory (
  account_id uuid not null references game.accounts (account_id) on delete cascade,
  item_id text not null references game.items (item_id),
  acquired_at timestamptz not null default now(),
  source text not null check (source in ('contract', 'manifest', 'shop', 'admin')),
  source_ref text,
  primary key (account_id, item_id)
);

-- One loadout per harness. Missing or no-longer-owned slots fall back to the baseline.
create table game.loadouts (
  account_id uuid not null references game.accounts (account_id) on delete cascade,
  harness text not null check (harness in ('Climber', 'Scrapper', 'Rigger', 'Doubler', 'Breaker')),
  line text not null default 'line.standard' references game.items (item_id),
  dog text not null default 'dog.standard' references game.items (item_id),
  cutter text not null default 'cutter.standard' references game.items (item_id),
  cosmetics jsonb not null default '{}',
  updated_at timestamptz not null default now(),
  primary key (account_id, harness)
);

create table game.wallets (
  account_id uuid not null references game.accounts (account_id) on delete cascade,
  currency text not null check (currency in ('tonnage_credit', 'sponsor_credit')),
  balance bigint not null default 0 check (balance >= 0),
  primary key (account_id, currency)
);

create table game.wallet_txns (
  txn_id uuid primary key default gen_random_uuid(),
  account_id uuid not null references game.accounts (account_id) on delete cascade,
  currency text not null check (currency in ('tonnage_credit', 'sponsor_credit')),
  delta bigint not null check (delta <> 0),
  reason text not null check (reason in ('match', 'shop', 'purchase', 'refund', 'admin')),
  ref text,
  idempotency_key text not null,
  created_at timestamptz not null default now(),
  unique (account_id, idempotency_key)
);

create table game.shop_rotation (
  rotation_id uuid primary key default gen_random_uuid(),
  -- The Claim the stock rotates with; null until Claims exist.
  claim_id text,
  item_id text not null references game.items (item_id),
  currency text not null check (currency in ('tonnage_credit', 'sponsor_credit')),
  price integer not null check (price > 0),
  starts_at timestamptz not null,
  ends_at timestamptz not null,
  check (ends_at > starts_at)
);
create index shop_rotation_live_idx on game.shop_rotation (item_id, starts_at, ends_at);

-- A sidegrade reaches an inventory only as a Contract reward.
create function private.check_inventory_item()
returns trigger
language plpgsql security definer set search_path = ''
as $$
begin
  if new.source <> 'contract'
     and exists (select 1 from game.items i where i.item_id = new.item_id and i.slot in ('line', 'dog', 'cutter')) then
    raise exception 'sidegrade_not_grantable' using hint = new.item_id;
  end if;
  return new;
end
$$;
create trigger inventory_items
  before insert or update on game.inventory
  for each row execute function private.check_inventory_item();

-- Only shop cosmetics are ever sold.
create function private.check_shop_item()
returns trigger
language plpgsql security definer set search_path = ''
as $$
begin
  if not exists (select 1 from game.items i
                  where i.item_id = new.item_id and i.source_rule = 'shop'
                    and i.slot not in ('line', 'dog', 'cutter')) then
    raise exception 'not_a_shop_item' using hint = new.item_id;
  end if;
  return new;
end
$$;
create trigger shop_rotation_items
  before insert or update of item_id on game.shop_rotation
  for each row execute function private.check_shop_item();

-- A catalog change may not turn a listed item into something the shop must not sell, or
-- retire it while it is listed.
create function private.check_listed_item()
returns trigger
language plpgsql security definer set search_path = ''
as $$
begin
  if (new.source_rule <> 'shop' or new.slot in ('line', 'dog', 'cutter') or new.retired)
     and exists (select 1 from game.shop_rotation r where r.item_id = new.item_id and r.ends_at > now()) then
    raise exception 'item_listed_in_shop' using hint = new.item_id;
  end if;
  return new;
end
$$;
create trigger items_listed
  before update of source_rule, slot, retired on game.items
  for each row execute function private.check_listed_item();

alter table game.items enable row level security;
alter table game.inventory enable row level security;
alter table game.loadouts enable row level security;
alter table game.wallets enable row level security;
alter table game.wallet_txns enable row level security;
alter table game.shop_rotation enable row level security;

create policy items_all on game.items for select to authenticated using (true);
create policy shop_rotation_all on game.shop_rotation for select to authenticated using (true);
create policy inventory_own on game.inventory
  for select to authenticated using (account_id = (select auth.uid()));
create policy loadouts_own on game.loadouts
  for select to authenticated using (account_id = (select auth.uid()));
create policy wallets_own on game.wallets
  for select to authenticated using (account_id = (select auth.uid()));
create policy wallet_txns_own on game.wallet_txns
  for select to authenticated using (account_id = (select auth.uid()));

grant select on game.items, game.shop_rotation, game.inventory, game.loadouts, game.wallets, game.wallet_txns
  to authenticated;

-- Ownership -------------------------------------------------------------------------

-- Grade items are owned once the Grade is reached; everything else needs an inventory row.
-- Either way the item must not be retired and the player must meet its Grade.
create function private.owns_item(p_account uuid, p_item text)
returns boolean
language sql stable security definer set search_path = ''
as $$
  select exists (
    select 1
      from game.items i
      join game.ledgers l on l.account_id = p_account
     where i.item_id = p_item and not i.retired
       and l.grade >= coalesce(i.grade_gated, 1)
       and (i.source_rule = 'grade'
            or exists (select 1 from game.inventory v where v.account_id = p_account and v.item_id = i.item_id)))
$$;

create function private.owned_or(p_account uuid, p_item text, p_slot text, p_default text)
returns text
language sql stable security definer set search_path = ''
as $$
  select case when p_item is not null and private.owns_item(p_account, p_item)
                   and (select slot from game.items where item_id = p_item) = p_slot
              then p_item else p_default end
$$;

-- The loadout a player may take into a match on this harness: every slot re-checked, so an
-- item lost or retired since it was equipped is replaced by the baseline (13.2).
create function private.valid_loadout(p_account uuid, p_harness text)
returns jsonb
language sql stable security definer set search_path = ''
as $$
  with l as (
    select line, dog, cutter, cosmetics from game.loadouts where account_id = p_account and harness = p_harness
  )
  select jsonb_build_object(
    'line', private.owned_or(p_account, (select line from l), 'line', 'line.standard'),
    'dog', private.owned_or(p_account, (select dog from l), 'dog', 'dog.standard'),
    'cutter', private.owned_or(p_account, (select cutter from l), 'cutter', 'cutter.standard'),
    'cosmetics', coalesce((
      select jsonb_object_agg(c.key, c.value #>> '{}')
        from l, jsonb_each(l.cosmetics) c
       where jsonb_typeof(c.value) = 'string'
         and private.owned_or(p_account, c.value #>> '{}', c.key, null) is not null), '{}'))
$$;

-- Every human's loadout is validated as the match is formed: the manifest carries only what
-- the player owns now, whatever they changed while queued.
create function private.fill_match_loadout()
returns trigger
language plpgsql security definer set search_path = ''
as $$
begin
  if new.account_id is not null then
    new.loadout := private.valid_loadout(new.account_id, new.harness);
  end if;
  return new;
end
$$;
create trigger match_players_loadout
  before insert on game.match_players
  for each row execute function private.fill_match_loadout();

-- Client functions ------------------------------------------------------------------

create function api.set_loadout(p_harness text, p_line text, p_dog text, p_cutter text,
                                p_cosmetics jsonb default '{}')
returns jsonb
language plpgsql security definer set search_path = ''
as $$
declare
  v_uid uuid := private.require_uid();
  v_slot text;
  v_item jsonb;
begin
  perform private.require_rate('loadout:' || v_uid::text, 30, interval '1 minute');
  if p_harness not in ('Climber', 'Scrapper', 'Rigger', 'Doubler', 'Breaker') then
    raise exception 'unknown_harness';
  end if;
  if jsonb_typeof(coalesce(p_cosmetics, '{}')) <> 'object' then
    raise exception 'bad_cosmetics';
  end if;
  if private.owned_or(v_uid, p_line, 'line', null) is null
     or private.owned_or(v_uid, p_dog, 'dog', null) is null
     or private.owned_or(v_uid, p_cutter, 'cutter', null) is null then
    raise exception 'not_owned';
  end if;
  for v_slot, v_item in select key, value from jsonb_each(coalesce(p_cosmetics, '{}')) loop
    if v_slot in ('line', 'dog', 'cutter') or jsonb_typeof(v_item) <> 'string'
       or private.owned_or(v_uid, v_item #>> '{}', v_slot, null) is null then
      raise exception 'not_owned' using hint = v_slot;
    end if;
  end loop;
  insert into game.loadouts (account_id, harness, line, dog, cutter, cosmetics, updated_at)
  values (v_uid, p_harness, p_line, p_dog, p_cutter, coalesce(p_cosmetics, '{}'), now())
  on conflict (account_id, harness) do update
    set line = excluded.line, dog = excluded.dog, cutter = excluded.cutter,
        cosmetics = excluded.cosmetics, updated_at = now();
  return private.valid_loadout(v_uid, p_harness);
end
$$;

-- Buys a listed shop item with earned or sponsor credit (13.3). The price must be the one the
-- client showed, the balance guard and the idempotency key keep concurrent calls safe.
create function api.shop_purchase(p_item text, p_expected_price integer, p_idem uuid)
returns jsonb
language plpgsql security definer set search_path = ''
as $$
declare
  v_uid uuid := private.require_uid();
  v_key text := 'shop:' || p_idem::text;
  v_price integer;
  v_currency text;
  v_balance bigint;
begin
  perform private.require_rate('shop:' || v_uid::text, 10, interval '1 minute');
  if p_idem is null then
    raise exception 'idempotency_key_required';
  end if;
  -- Serializes this player's purchases, so a retried call sees the first one's row.
  perform 1 from game.ledgers where account_id = v_uid for update;
  if exists (select 1 from game.wallet_txns where account_id = v_uid and idempotency_key = v_key) then
    return jsonb_build_object('status', 'duplicate');
  end if;
  select r.price, r.currency into v_price, v_currency
    from game.shop_rotation r
    join game.items i on i.item_id = r.item_id and not i.retired
   where r.item_id = p_item and now() >= r.starts_at and now() < r.ends_at
   order by r.starts_at desc limit 1;
  if v_price is null then
    raise exception 'not_listed';
  end if;
  if v_price is distinct from p_expected_price then
    raise exception 'price_changed';
  end if;
  if exists (select 1 from game.inventory where account_id = v_uid and item_id = p_item) then
    raise exception 'already_owned';
  end if;
  update game.wallets set balance = balance - v_price
   where account_id = v_uid and currency = v_currency and balance >= v_price
  returning balance into v_balance;
  if not found then
    raise exception 'insufficient_funds';
  end if;
  insert into game.wallet_txns (account_id, currency, delta, reason, ref, idempotency_key)
  values (v_uid, v_currency, -v_price, 'shop', p_item, v_key);
  insert into game.inventory (account_id, item_id, source, source_ref) values (v_uid, p_item, 'shop', v_key);
  return jsonb_build_object('status', 'ok', 'currency', v_currency, 'balance', v_balance);
end
$$;

-- Service functions -----------------------------------------------------------------

-- The game server's loadout for a player at a reset: the lobby and between rounds (menus 6;
-- Rated locks it at the first Live, which the server enforces by when it asks). Re-checked
-- against what the player owns now, and recorded on the match roster.
create function api.srv_player_loadout(p_match uuid, p_allocation text, p_account uuid)
returns jsonb
language plpgsql security definer set search_path = ''
as $$
declare
  v_harness text;
  v_loadout jsonb;
begin
  perform 1 from game.matches
   where match_id = p_match and allocation_id = p_allocation and state in ('waiting_for_players', 'in_progress');
  if not found then
    raise exception 'unknown_match';
  end if;
  select harness into v_harness from game.match_players where match_id = p_match and account_id = p_account;
  if not found then
    raise exception 'unknown_player';
  end if;
  v_loadout := private.valid_loadout(p_account, v_harness);
  update game.match_players set loadout = v_loadout where match_id = p_match and account_id = p_account;
  return v_loadout;
end
$$;

-- Read views for clients ------------------------------------------------------------

create view api.catalog with (security_invoker = true) as
  select i.item_id, i.slot, i.source_rule, i.grade_gated, i.metadata from game.items i where not i.retired;

-- Everything the player may equip: Grade unlocks plus inventory.
create view api.my_items with (security_invoker = true) as
  select i.item_id, i.slot, coalesce(v.source, 'grade') as source, v.acquired_at
    from game.items i
    join game.ledgers l on l.account_id = (select auth.uid())
    left join game.inventory v on v.item_id = i.item_id and v.account_id = l.account_id
   where not i.retired and l.grade >= coalesce(i.grade_gated, 1)
     and (i.source_rule = 'grade' or v.item_id is not null);

create view api.my_loadouts with (security_invoker = true) as
  select o.harness, o.line, o.dog, o.cutter, o.cosmetics, o.updated_at from game.loadouts o;

create view api.my_wallets with (security_invoker = true) as
  select w.currency, w.balance from game.wallets w;

create view api.my_wallet_txns with (security_invoker = true) as
  select t.txn_id, t.currency, t.delta, t.reason, t.ref, t.created_at from game.wallet_txns t;

create view api.shop with (security_invoker = true) as
  select r.item_id, i.slot, r.currency, r.price, r.starts_at, r.ends_at, r.claim_id
    from game.shop_rotation r join game.items i using (item_id)
   where now() >= r.starts_at and now() < r.ends_at and not i.retired;

grant select on api.catalog, api.my_items, api.my_loadouts, api.my_wallets, api.my_wallet_txns, api.shop
  to authenticated;

-- Grants ----------------------------------------------------------------------------
revoke execute on function
  private.check_inventory_item(), api.srv_player_loadout(uuid, text, uuid),
  private.check_shop_item(), private.check_listed_item(), private.owns_item(uuid, text),
  private.owned_or(uuid, text, text, text), private.valid_loadout(uuid, text), private.fill_match_loadout(),
  api.set_loadout(text, text, text, text, jsonb), api.shop_purchase(text, integer, uuid)
  from public, anon, authenticated;
grant execute on function
  api.set_loadout(text, text, text, text, jsonb), api.shop_purchase(text, integer, uuid)
  to authenticated;
grant execute on function api.srv_player_loadout(uuid, text, uuid) to service_role;
