-- Phase 2, admin console v1 (backend-architecture sections 7.6, 14 and 17): every change staff
-- make to live-ops tables is written to admin.audit by trigger, append-only. The console
-- (backend/tools/admin.ts) works over the service connection and names the staff member for
-- each transaction with `set local kinetic.staff = '<name>'`, which must be a name in
-- admin.staff. The audit covers changes made that way and the worker's season steps; a direct
-- SQL session that names no staff member is not audited on the per-match tables, so only the
-- console and the worker should hold the service connection.
--
-- Season tables (claims, contracts, Manifest crates) are audited on every change, the worker's
-- lifecycle steps included, as 'system' when no staff member is named. Tables the worker writes
-- per match (inventory, entitlements, report status) and config and sanctions are audited when
-- a staff member makes the change.

create table admin.audit (
  audit_id bigint generated always as identity primary key,
  at timestamptz not null default now(),
  staff_id text not null,
  action text not null,
  target text not null,
  before jsonb,
  after jsonb
);
alter table admin.audit enable row level security;
revoke all on admin.audit from public, anon, authenticated;

-- Staff the console may act as. Added by the database owner; MFA arrives with the web console.
create table admin.staff (
  staff_id text primary key check (staff_id ~ '^[a-z0-9._-]{2,40}$'),
  added_at timestamptz not null default now()
);
alter table admin.staff enable row level security;
revoke all on admin.staff from public, anon, authenticated;

create function private.audit_append_only()
returns trigger
language plpgsql security definer set search_path = ''
as $$
begin
  raise exception 'audit_is_append_only';
end
$$;
create trigger audit_append_only
  before update or delete on admin.audit
  for each row execute function private.audit_append_only();

-- The staff member behind the current transaction, or null.
create function private.current_staff()
returns text
language sql stable
set search_path = ''
as $$
  select nullif(current_setting('kinetic.staff', true), '')
$$;

create function private.audit_change()
returns trigger
language plpgsql security definer set search_path = ''
as $$
declare
  v_staff text := private.current_staff();
  v_row jsonb := to_jsonb(coalesce(new, old));
  v_target text;
begin
  if v_staff is null and tg_argv[0] = 'staff' then
    return coalesce(new, old);
  end if;
  if v_staff is not null and not exists (select 1 from admin.staff where staff_id = v_staff) then
    raise exception 'unknown_staff' using hint = v_staff;
  end if;
  -- The trigger's arguments after the first name the row's key columns.
  select tg_table_name || ':' || string_agg(coalesce(v_row ->> k, '?'), ':' order by n)
    into v_target
    from unnest(tg_argv[1:]) with ordinality as a(k, n);
  insert into admin.audit (staff_id, action, target, before, after)
  values (coalesce(v_staff, 'system'), lower(tg_op), v_target,
          case when tg_op <> 'INSERT' then to_jsonb(old) end,
          case when tg_op <> 'DELETE' then to_jsonb(new) end);
  return coalesce(new, old);
end
$$;

create trigger claims_audit after insert or update or delete on game.claims
  for each row execute function private.audit_change('always', 'claim_id');
create trigger contracts_audit after insert or update or delete on game.contracts
  for each row execute function private.audit_change('always', 'contract_id');
create trigger manifest_crates_audit after insert or update or delete on game.manifest_crates
  for each row execute function private.audit_change('always', 'claim_id', 'track', 'tier');
create trigger sanctions_audit after insert or update or delete on game.sanctions
  for each row execute function private.audit_change('staff', 'account_id', 'type');
create trigger entitlements_audit after insert or update or delete on game.entitlements
  for each row execute function private.audit_change('staff', 'account_id', 'entitlement');
create trigger inventory_audit after insert or update or delete on game.inventory
  for each row execute function private.audit_change('staff', 'account_id', 'item_id');
create trigger config_audit after insert or update or delete on game.config
  for each row execute function private.audit_change('staff', 'key');
create trigger match_reports_audit after update on game.match_reports
  for each row execute function private.audit_change('staff', 'match_id');

revoke execute on function private.audit_append_only(), private.current_staff(), private.audit_change()
  from public, anon, authenticated;
