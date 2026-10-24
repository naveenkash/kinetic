-- Export and deletion requests (backend-architecture 19).
begin;
create extension if not exists pgtap with schema extensions;
select plan(11);

insert into auth.users (id, email, aud, role, raw_app_meta_data) values
  ('ae111111-1111-4111-8111-111111111111', 'dev-pia@users.kinetic.invalid', 'authenticated', 'authenticated', '{"kinetic_provider":"dev","kinetic_provider_user_id":"pia"}'),
  ('ae222222-2222-4222-8222-222222222222', 'dev-quin@users.kinetic.invalid', 'authenticated', 'authenticated', '{"kinetic_provider":"dev","kinetic_provider_user_id":"quin"}');
select api.srv_ensure_account('ae111111-1111-4111-8111-111111111111', 'dev', 'pia', 'pia_rigger');
select api.srv_ensure_account('ae222222-2222-4222-8222-222222222222', 'dev', 'quin', 'quin_rigger');

set local role authenticated;
set local request.jwt.claims = '{"sub":"ae111111-1111-4111-8111-111111111111","role":"authenticated"}';
select is(api.request_export() ->> 'status', 'queued', 'an export is queued');
select throws_ok($$ select api.request_export() $$, 'P0001', 'rate_limited', 'one a day');
reset role;
delete from private.rate_limits where key like 'export:%';
set local role authenticated;
select throws_ok($$ select api.request_export() $$, 'P0001', 'export_pending', 'and one at a time');
create temp table first_request as select api.request_deletion() as r;
select ok(((select r from first_request) ->> 'deletes_at')::timestamptz > now() + interval '13 days',
  'a deletion waits fourteen days');
reset role;
delete from private.rate_limits where key like 'deletion:%';
set local role authenticated;
select is(api.request_deletion() ->> 'job_id', (select r ->> 'job_id' from first_request),
  'asking again returns the same request');
select is((select count(*) from api.my_account_jobs where kind = 'deletion'), 1::bigint, 'one deletion pending');
select is(api.cancel_deletion() ->> 'status', 'cancelled', 'and it can be cancelled in the grace');
select throws_ok($$ select api.cancel_deletion() $$, 'P0001', 'no_pending_deletion', 'only while pending');

set local request.jwt.claims = '{"sub":"ae222222-2222-4222-8222-222222222222","role":"authenticated"}';
select is((select count(*) from api.my_account_jobs), 0::bigint, 'another player sees none of it');

-- The exports bucket: each player reads their own folder only.
reset role;
insert into storage.objects (bucket_id, name, owner_id) values
  ('exports', 'ae111111-1111-4111-8111-111111111111/a.json', null),
  ('exports', 'ae222222-2222-4222-8222-222222222222/b.json', null);
set local role authenticated;
select is((select array_agg(name) from storage.objects where bucket_id = 'exports'),
  array['ae222222-2222-4222-8222-222222222222/b.json'], 'a player sees only their own exports');
select throws_ok($$ insert into storage.objects (bucket_id, name) values ('exports', 'ae222222-2222-4222-8222-222222222222/x.json') $$,
  '42501', null, 'and cannot write there');

select * from finish();
rollback;
