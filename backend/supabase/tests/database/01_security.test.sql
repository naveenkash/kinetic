-- Grants, RLS and helpers. Run with `supabase test db`.
begin;
create extension if not exists pgtap with schema extensions;
select plan(14);

insert into auth.users (id, email, aud, role, raw_app_meta_data) values
  ('11111111-1111-4111-8111-111111111111', 'dev-alice@users.kinetic.invalid', 'authenticated', 'authenticated', '{"kinetic_provider":"dev","kinetic_provider_user_id":"alice"}'),
  ('22222222-2222-4222-8222-222222222222', 'dev-bob@users.kinetic.invalid', 'authenticated', 'authenticated', '{"kinetic_provider":"dev","kinetic_provider_user_id":"bob"}'),
  ('33333333-3333-4333-8333-333333333333', 'dev-carol@users.kinetic.invalid', 'authenticated', 'authenticated', '{"kinetic_provider":"dev","kinetic_provider_user_id":"carol"}');
select api.srv_ensure_account('11111111-1111-4111-8111-111111111111', 'dev', 'alice', 'alice');
select api.srv_ensure_account('22222222-2222-4222-8222-222222222222', 'dev', 'bob', 'bob');
select api.srv_ensure_account('33333333-3333-4333-8333-333333333333', 'dev', 'carol', 'carol');

-- Nobody but service_role can call srv_* functions.
set local role anon;
select throws_ok($$ select api.srv_build_ok('0.1.0') $$, '42501', null, 'anon cannot call srv functions');
select throws_ok($$ select * from api.my_account $$, '42501', null, 'anon cannot read account views');
select lives_ok($$ select api.get_config() $$, 'anon can read config');

set local role authenticated;
set local request.jwt.claims = '{"sub":"11111111-1111-4111-8111-111111111111","role":"authenticated"}';
select throws_ok($$ select api.srv_store_report(gen_random_uuid(), 'x', '{}') $$, '42501', null,
  'players cannot call srv functions');
select throws_ok($$ select private.hit_rate_limit('k', 1, interval '1 minute') $$, '42501', null,
  'players cannot call private helpers');
select throws_ok($$ select * from game.match_reports $$, '42501', null, 'players cannot read game tables directly');
select is((select count(*) from api.my_account), 1::bigint, 'a player sees only their own account');
select is((select display_name from api.my_account), 'alice', 'and it is theirs');

-- Crewmates become visible through RLS; outsiders do not.
select api.crew_invite('22222222-2222-4222-8222-222222222222');
set local request.jwt.claims = '{"sub":"22222222-2222-4222-8222-222222222222","role":"authenticated"}';
select api.crew_accept((select crew_id from api.my_invites limit 1));
select is((select count(*) from api.my_crew), 2::bigint, 'a crew member sees both members');
select is((select array_agg(display_name order by display_name) from api.my_crew), array['alice', 'bob'],
  'with their display names');
set local request.jwt.claims = '{"sub":"33333333-3333-4333-8333-333333333333","role":"authenticated"}';
select is((select count(*) from api.my_crew), 0::bigint, 'an outsider sees no crew');

reset role;
-- The limiter allows exactly max hits per window.
select is(
  array[private.hit_rate_limit('t', 2, interval '1 minute'),
        private.hit_rate_limit('t', 2, interval '1 minute'),
        private.hit_rate_limit('t', 2, interval '1 minute')],
  array[true, true, false], 'rate limiter allows max hits then refuses');
select is(
  array[private.version_cmp('0.1.0', '0.1.0'), private.version_cmp('0.1.0', '0.2'), private.version_cmp('1.0', '0.9.9')],
  array[0, -1, 1], 'build comparison');
select throws_ok($$ select private.version_cmp('0.1.0-beta', '0.1.0') $$, 'P0001', 'bad_build', 'malformed build');

select * from finish();
rollback;
