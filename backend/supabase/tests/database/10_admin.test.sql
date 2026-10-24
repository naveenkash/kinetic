-- The admin audit (backend-architecture 7.6 and 17): written by trigger, append-only, and out of
-- every player's reach.
begin;
create extension if not exists pgtap with schema extensions;
select plan(7);

insert into admin.staff (staff_id) values ('staff-pgtap');
select set_config('kinetic.staff', 'staff-pgtap', true);
update game.claims set state = 'archived' where state in ('active', 'scheduled');
insert into game.claims (claim_id, name, arena, starts_at, ends_at)
values ('a-claim', 'Audit', 'ANC', '2390-01-01', '2390-02-01');
select is((select staff_id from admin.audit where target = 'claims:a-claim' and action = 'insert'), 'staff-pgtap',
  'a staff change is audited under the staff name');
select set_config('kinetic.staff', '', true);
update game.claims set state = 'active' where claim_id = 'a-claim';
select is((select staff_id from admin.audit where target = 'claims:a-claim' and action = 'update'
            order by audit_id desc limit 1), 'system', 'the worker''s lifecycle steps are audited as the system');
insert into game.config (key, value) values ('a-flag', 'true');
select is((select count(*) from admin.audit where target = 'config:a-flag'), 0::bigint,
  'system writes to per-match tables are not audited');
select throws_ok($$ delete from admin.audit $$, 'P0001', 'audit_is_append_only', 'the audit is append-only');
select set_config('kinetic.staff', 'staff-pgtap', true);
insert into game.manifest_crates (claim_id, track, tier, tonnage_t) values ('a-claim', 'free', 2, 100);
select is((select target from admin.audit where target like 'manifest_crates:%' order by audit_id desc limit 1),
  'manifest_crates:a-claim:free:2', 'targets name the row''s key');
select set_config('kinetic.staff', 'mallory', true);
select throws_ok($$ update game.claims set name = 'x' where claim_id = 'a-claim' $$, 'P0001', 'unknown_staff',
  'only listed staff may act');

set local role authenticated;
select throws_ok($$ select * from admin.audit $$, '42501', null, 'players cannot read the audit');

select * from finish();
rollback;
