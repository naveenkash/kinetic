-- Phase 4: the client config carries flags, variants and the Claim; identity linking is a
-- service step only; one identity per platform per account.
begin;
create extension if not exists pgtap with schema extensions;
select plan(8);

insert into auth.users (id, email, aud, role, raw_app_meta_data) values
  ('af111111-1111-4111-8111-111111111111', 'dev-ria@users.kinetic.invalid', 'authenticated', 'authenticated',
   '{"kinetic_provider":"dev","kinetic_provider_user_id":"ria"}');
select api.srv_ensure_account('af111111-1111-4111-8111-111111111111', 'dev', 'ria', 'ria_rigger');
insert into game.config (key, value) values ('flags', '{"voice": true}'), ('variant:YRD', '"haulers_doubled"')
  on conflict (key) do update set value = excluded.value;

set local role anon;
select is(api.get_config() -> 'flags' ->> 'voice', 'true', 'anyone reads the feature flags');
select is(api.get_config() -> 'variants' ->> 'YRD', 'haulers_doubled', 'and the arena variants');
select ok(api.get_config() ? 'claim', 'and the current Claim, if any');

set local role authenticated;
set local request.jwt.claims = '{"sub":"af111111-1111-4111-8111-111111111111","role":"authenticated"}';
select throws_ok($$ select api.srv_link_identity('af111111-1111-4111-8111-111111111111', 'psn', 'p1') $$,
  '42501', null, 'a player cannot link an identity themselves');
select throws_ok($$ select api.srv_unlink_identity('af111111-1111-4111-8111-111111111111', 'dev') $$,
  '42501', null, 'nor unlink one');
reset role;

select is(api.srv_link_identity('af111111-1111-4111-8111-111111111111', 'nintendo', 'n1') ->> 'linked', 'true',
  'the service links a proved identity');
select throws_ok($$ insert into game.identities (provider, provider_user_id, account_id)
                    values ('nintendo', 'n2', 'af111111-1111-4111-8111-111111111111') $$,
  '23505', null, 'one identity per platform per account');
select throws_ok($$ select api.srv_unlink_identity('af111111-1111-4111-8111-111111111111', 'xbox') $$,
  'P0001', 'not_linked', 'unlinking needs a link');

select * from finish();
rollback;
