-- Phase 3, real money through Steam (backend-architecture 7.4 and 13.4): store SKUs, the
-- purchase record and its states, the grant at finalize and its reversal on a refund or a
-- chargeback. The edge functions store-steam-init and store-steam-finalize and the worker's
-- purchases pass call these service functions; Steam is called from TypeScript, never here.
--
-- What money buys (bible/04): sponsor credit for shop cosmetics, and Sponsorship of a Claim
-- (the Sponsored Manifest column). Never power, never randomized items. Each order keeps what
-- it was sold as (kind, credit, Claim) from the moment it is created, so a later SKU edit
-- changes neither its grant nor its reversal. A balance never goes negative: a refund of
-- credit already spent takes what is left and flags the account for support.

create table game.skus (
  sku text primary key check (sku ~ '^[a-z0-9_.-]{1,40}$'),
  kind text not null check (kind in ('sponsor_credit', 'sponsorship')),
  -- Steam's numeric item id for the sale.
  steam_item_id integer not null unique check (steam_item_id > 0),
  description text not null check (length(description) between 1 and 128),
  -- Price in the currency's smallest unit, as Steam bills the player's wallet: {"USD": 499}.
  prices jsonb not null check (jsonb_typeof(prices) = 'object'),
  credit integer check (credit > 0),
  claim_id text references game.claims (claim_id),
  enabled boolean not null default true,
  check ((kind = 'sponsor_credit') = (credit is not null)),
  check ((kind = 'sponsorship') = (claim_id is not null))
);

create sequence game.purchase_order_seq start 100000001;

create table game.purchases (
  purchase_id uuid primary key default gen_random_uuid(),
  platform text not null default 'steam' check (platform = 'steam'),
  platform_order_id bigint not null unique default nextval('game.purchase_order_seq'),
  platform_trans_id text,
  account_id uuid references game.accounts (account_id) on delete set null,
  sku text not null references game.skus (sku),
  -- What the order sells, as it was when the order was created.
  kind text not null check (kind in ('sponsor_credit', 'sponsorship')),
  credit integer check (credit > 0),
  claim_id text,
  amount integer not null check (amount > 0),
  currency_code text not null check (currency_code ~ '^[A-Z]{3}$'),
  status text not null default 'initiated'
    check (status in ('initiated', 'failed', 'cancelled', 'finalized', 'refunded', 'charged_back')),
  -- Whether finalize granted anything (a Sponsorship already held grants nothing).
  granted boolean not null default false,
  failure_reason text,
  -- The worker's checks with Steam on an order the client never finalized, and its backoff.
  checks integer not null default 0,
  next_check_at timestamptz,
  created_at timestamptz not null default now(),
  initiated_at timestamptz,
  finalized_at timestamptz,
  reversed_at timestamptz
);
create index purchases_open_idx on game.purchases (next_check_at, created_at) where status = 'initiated';
create index purchases_account_idx on game.purchases (account_id, created_at desc);

-- Accounts support should look at: a refund of credit already spent, a reversed or duplicate
-- Sponsorship, a partial refund.
create table game.support_flags (
  flag_id bigint generated always as identity primary key,
  account_id uuid references game.accounts (account_id) on delete cascade,
  reason text not null,
  ref text,
  created_at timestamptz not null default now(),
  resolved_at timestamptz
);

alter table game.skus enable row level security;
alter table game.purchases enable row level security;
alter table game.support_flags enable row level security;
create policy skus_enabled on game.skus for select to authenticated using (enabled);
create policy purchases_own on game.purchases for select to authenticated using (account_id = (select auth.uid()));
grant select on game.skus, game.purchases to authenticated;

create trigger skus_audit after insert or update or delete on game.skus
  for each row execute function private.audit_change('staff', 'sku');
create trigger support_flags_audit after update on game.support_flags
  for each row execute function private.audit_change('staff', 'flag_id');

create view api.store with (security_invoker = true) as
  select s.sku, s.kind, s.description, s.prices, s.credit, s.claim_id from game.skus s
   where s.claim_id is null or exists (select 1 from game.claims c where c.claim_id = s.claim_id and c.state = 'active');

create view api.my_purchases with (security_invoker = true) as
  select p.platform_order_id::text as order_id, p.sku, p.amount, p.currency_code, p.status, p.created_at, p.finalized_at
    from game.purchases p;

grant select on api.store, api.my_purchases to authenticated;

-- Service functions -------------------------------------------------------------------

-- The player's SteamID, for the store calls; null for accounts without a Steam identity.
create function api.srv_steam_id(p_account uuid)
returns text
language sql stable security definer set search_path = ''
as $$
  select provider_user_id from game.identities where account_id = p_account and provider = 'steam'
$$;

-- A new order for a SKU in the player's Steam wallet currency. Returns what InitTxn needs.
create function api.srv_purchase_create(p_account uuid, p_sku text, p_currency text)
returns jsonb
language plpgsql security definer set search_path = ''
as $$
declare
  v_sku game.skus;
  v_amount integer;
  v_order bigint;
begin
  -- One order at a time per player, even for two calls at once: a double tap is refused.
  perform pg_advisory_xact_lock(hashtext('purchase:' || p_account::text));
  if not exists (select 1 from game.accounts where account_id = p_account and status = 'active')
     or private.active_sanction(p_account, array['account_ban']) then
    raise exception 'account_inactive';
  end if;
  select * into v_sku from game.skus where sku = p_sku and enabled;
  if not found then
    raise exception 'unknown_sku';
  end if;
  if jsonb_typeof(v_sku.prices -> p_currency) is distinct from 'number' then
    raise exception 'currency_unsupported' using hint = p_currency;
  end if;
  v_amount := (v_sku.prices ->> p_currency)::integer;
  if v_sku.kind = 'sponsorship' then
    if not exists (select 1 from game.claims where claim_id = v_sku.claim_id and state = 'active') then
      raise exception 'claim_not_active';
    end if;
    if exists (select 1 from game.entitlements
                where account_id = p_account and entitlement = 'sponsorship:' || v_sku.claim_id) then
      raise exception 'already_sponsored';
    end if;
  end if;
  if exists (select 1 from game.purchases where account_id = p_account and status = 'initiated'
               and created_at > now() - interval '15 minutes') then
    raise exception 'purchase_open';
  end if;
  insert into game.purchases (account_id, sku, kind, credit, claim_id, amount, currency_code)
  values (p_account, p_sku, v_sku.kind, v_sku.credit, v_sku.claim_id, v_amount, p_currency)
  returning platform_order_id into v_order;
  return jsonb_build_object('order_id', v_order::text, 'amount', v_amount, 'currency', p_currency,
                            'item_id', v_sku.steam_item_id, 'description', v_sku.description);
end
$$;

create function api.srv_purchase_initiated(p_order bigint, p_trans text)
returns void
language sql security definer set search_path = ''
as $$
  update game.purchases set platform_trans_id = p_trans, initiated_at = now()
   where platform_order_id = p_order and status = 'initiated'
$$;

-- An order that never completes: Steam refused it, the player declined, or it went stale.
create function api.srv_purchase_failed(p_order bigint, p_status text, p_reason text)
returns void
language plpgsql security definer set search_path = ''
as $$
begin
  if p_status not in ('failed', 'cancelled') then
    raise exception 'bad_status';
  end if;
  update game.purchases set status = p_status, failure_reason = p_reason
   where platform_order_id = p_order and status = 'initiated';
end
$$;

-- The worker asked Steam about an open order and it is still open: check again later, from a
-- minute, doubling, at most hourly.
create function api.srv_purchase_checked(p_order bigint)
returns void
language sql security definer set search_path = ''
as $$
  update game.purchases
     set checks = checks + 1,
         next_check_at = now() + make_interval(secs => least(60 * power(2, least(checks, 10)), 3600))
   where platform_order_id = p_order and status = 'initiated'
$$;

-- The order as its owner sees it; the finalize endpoint acts only on the caller's own order.
create function api.srv_purchase_of(p_order bigint, p_account uuid)
returns jsonb
language sql stable security definer set search_path = ''
as $$
  select jsonb_build_object('order_id', platform_order_id::text, 'status', status, 'sku', sku)
    from game.purchases where platform_order_id = p_order and account_id = p_account
$$;

-- Steam has taken the money: mark the order finalized and grant, in one transaction. A
-- retried call grants nothing more.
create function api.srv_purchase_finalize(p_order bigint, p_trans text)
returns jsonb
language plpgsql security definer set search_path = ''
as $$
declare
  v_purchase game.purchases;
  v_granted boolean := false;
begin
  select * into v_purchase from game.purchases where platform_order_id = p_order for update;
  if not found then
    raise exception 'unknown_order';
  end if;
  if v_purchase.status = 'finalized' then
    return jsonb_build_object('status', 'finalized', 'duplicate', true, 'granted', v_purchase.granted);
  end if;
  if v_purchase.status <> 'initiated' then
    raise exception 'order_not_open' using hint = v_purchase.status;
  end if;
  if v_purchase.account_id is not null and v_purchase.kind = 'sponsor_credit' then
    insert into game.wallet_txns (account_id, currency, delta, reason, ref, idempotency_key)
    values (v_purchase.account_id, 'sponsor_credit', v_purchase.credit, 'purchase', p_order::text,
            'purchase:' || p_order);
    insert into game.wallets (account_id, currency, balance)
    values (v_purchase.account_id, 'sponsor_credit', v_purchase.credit)
    on conflict (account_id, currency) do update set balance = wallets.balance + excluded.balance;
    v_granted := true;
  elsif v_purchase.account_id is not null then
    -- The entitlement's trigger pays the Sponsored crates already passed. Already held (a second
    -- paid order): nothing more to grant, and support refunds it.
    insert into game.entitlements (account_id, entitlement, source_ref)
    values (v_purchase.account_id, 'sponsorship:' || v_purchase.claim_id, 'purchase:' || p_order)
    on conflict do nothing;
    v_granted := found;
    if not v_granted then
      insert into game.support_flags (account_id, reason, ref)
      values (v_purchase.account_id, 'already_sponsored', 'order:' || p_order);
    end if;
  end if;
  update game.purchases set status = 'finalized', finalized_at = now(), granted = v_granted,
         platform_trans_id = coalesce(p_trans, platform_trans_id)
   where purchase_id = v_purchase.purchase_id;
  return jsonb_build_object('status', 'finalized', 'granted', v_granted, 'kind', v_purchase.kind);
end
$$;

-- A refund or chargeback on a finalized order (13.4 step 4): reverse what this order granted.
-- Credit is taken back as far as the balance allows; any shortfall, and any reversed
-- Sponsorship, flags the account for support. Manifest crates already paid are kept (PROPOSED).
create function api.srv_purchase_reverse(p_order bigint, p_status text)
returns jsonb
language plpgsql security definer set search_path = ''
as $$
declare
  v_purchase game.purchases;
  v_balance bigint;
  v_taken bigint := 0;
begin
  if p_status not in ('refunded', 'charged_back') then
    raise exception 'bad_status';
  end if;
  select * into v_purchase from game.purchases where platform_order_id = p_order for update;
  if not found then
    raise exception 'unknown_order';
  end if;
  if v_purchase.status in ('refunded', 'charged_back') then
    return jsonb_build_object('status', v_purchase.status, 'duplicate', true);
  end if;
  if v_purchase.status <> 'finalized' then
    raise exception 'order_not_finalized' using hint = v_purchase.status;
  end if;
  update game.purchases set status = p_status, reversed_at = now() where purchase_id = v_purchase.purchase_id;
  if v_purchase.account_id is null or not v_purchase.granted then
    return jsonb_build_object('status', p_status, 'taken', 0);
  end if;
  if v_purchase.kind = 'sponsor_credit' then
    select balance into v_balance from game.wallets
     where account_id = v_purchase.account_id and currency = 'sponsor_credit' for update;
    v_taken := least(coalesce(v_balance, 0), v_purchase.credit);
    if v_taken > 0 then
      update game.wallets set balance = balance - v_taken
       where account_id = v_purchase.account_id and currency = 'sponsor_credit';
      insert into game.wallet_txns (account_id, currency, delta, reason, ref, idempotency_key)
      values (v_purchase.account_id, 'sponsor_credit', -v_taken, 'refund', p_order::text, 'refund:' || p_order);
    end if;
    if v_taken < v_purchase.credit then
      insert into game.support_flags (account_id, reason, ref)
      values (v_purchase.account_id, p_status || '_after_spend',
              'order:' || p_order || ' short:' || (v_purchase.credit - v_taken));
    end if;
  else
    delete from game.entitlements
     where account_id = v_purchase.account_id and entitlement = 'sponsorship:' || v_purchase.claim_id
       and source_ref = 'purchase:' || p_order;
    insert into game.support_flags (account_id, reason, ref)
    values (v_purchase.account_id, p_status || '_sponsorship', 'order:' || p_order);
  end if;
  return jsonb_build_object('status', p_status, 'taken', v_taken);
end
$$;

-- Something support must decide by hand: a partial refund, an order Steam does not know.
create function api.srv_purchase_flag(p_order bigint, p_reason text)
returns void
language sql security definer set search_path = ''
as $$
  insert into game.support_flags (account_id, reason, ref)
  select account_id, p_reason, 'order:' || p_order from game.purchases where platform_order_id = p_order
   and not exists (select 1 from game.support_flags f
                    where f.reason = p_reason and f.ref = 'order:' || p_order)
$$;

-- Grants ------------------------------------------------------------------------------
revoke execute on function
  api.srv_steam_id(uuid), api.srv_purchase_create(uuid, text, text), api.srv_purchase_initiated(bigint, text),
  api.srv_purchase_failed(bigint, text, text), api.srv_purchase_checked(bigint), api.srv_purchase_of(bigint, uuid),
  api.srv_purchase_finalize(bigint, text), api.srv_purchase_reverse(bigint, text), api.srv_purchase_flag(bigint, text)
  from public, anon, authenticated;
grant execute on function
  api.srv_steam_id(uuid), api.srv_purchase_create(uuid, text, text), api.srv_purchase_initiated(bigint, text),
  api.srv_purchase_failed(bigint, text, text), api.srv_purchase_checked(bigint), api.srv_purchase_of(bigint, uuid),
  api.srv_purchase_finalize(bigint, text), api.srv_purchase_reverse(bigint, text), api.srv_purchase_flag(bigint, text)
  to service_role;
