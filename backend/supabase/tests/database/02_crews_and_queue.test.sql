-- Crew and queue rules (backend-architecture sections 9.2 and 10.1, game-modes 6).
begin;
create extension if not exists pgtap with schema extensions;
select plan(18);

insert into auth.users (id, email, aud, role, raw_app_meta_data) values
  ('a1111111-1111-4111-8111-111111111111', 'dev-lead@users.kinetic.invalid', 'authenticated', 'authenticated', '{"kinetic_provider":"dev","kinetic_provider_user_id":"lead"}'),
  ('a2222222-2222-4222-8222-222222222222', 'dev-hand@users.kinetic.invalid', 'authenticated', 'authenticated', '{"kinetic_provider":"dev","kinetic_provider_user_id":"hand"}'),
  ('a3333333-3333-4333-8333-333333333333', 'dev-spot@users.kinetic.invalid', 'authenticated', 'authenticated', '{"kinetic_provider":"dev","kinetic_provider_user_id":"spot"}'),
  ('a4444444-4444-4444-8444-444444444444', 'dev-extra@users.kinetic.invalid', 'authenticated', 'authenticated', '{"kinetic_provider":"dev","kinetic_provider_user_id":"extra"}');
select api.srv_ensure_account('a1111111-1111-4111-8111-111111111111', 'dev', 'lead', 'lead');
select api.srv_ensure_account('a2222222-2222-4222-8222-222222222222', 'dev', 'hand', 'hand');
select api.srv_ensure_account('a3333333-3333-4333-8333-333333333333', 'dev', 'spot', 'spot');
select api.srv_ensure_account('a4444444-4444-4444-8444-444444444444', 'dev', 'extra', 'extra');
update game.ledgers set grade = 3 where account_id = 'a2222222-2222-4222-8222-222222222222';

set local role authenticated;
set local request.jwt.claims = '{"sub":"a1111111-1111-4111-8111-111111111111","role":"authenticated"}';
select api.crew_invite('a2222222-2222-4222-8222-222222222222');
select api.crew_invite('a3333333-3333-4333-8333-333333333333');

set local request.jwt.claims = '{"sub":"a2222222-2222-4222-8222-222222222222","role":"authenticated"}';
select lives_ok($$ select api.crew_accept((select crew_id from api.my_invites limit 1)) $$, 'invitee accepts');
select throws_ok($$ select api.crew_invite('a4444444-4444-4444-8444-444444444444') $$, 'P0001', 'not_leader',
  'only the leader invites');
set local request.jwt.claims = '{"sub":"a3333333-3333-4333-8333-333333333333","role":"authenticated"}';
select lives_ok($$ select api.crew_accept((select crew_id from api.my_invites limit 1)) $$, 'second invitee accepts');

set local request.jwt.claims = '{"sub":"a1111111-1111-4111-8111-111111111111","role":"authenticated"}';
select throws_ok($$ select api.crew_invite('a4444444-4444-4444-8444-444444444444') $$, 'P0001', 'crew_full',
  'a crew holds three');
select throws_ok($$ select api.queue_join('scrap_3v3_casual', '{"local": 20}', '0.1.0') $$, 'P0001', 'crew_not_ready',
  'members must be ready');

set local request.jwt.claims = '{"sub":"a2222222-2222-4222-8222-222222222222","role":"authenticated"}';
select api.crew_set_pick('Breaker', 'rook-adeyemi', true);
set local request.jwt.claims = '{"sub":"a3333333-3333-4333-8333-333333333333","role":"authenticated"}';
select api.crew_set_pick('Climber', null, true);
select throws_ok($$ select api.crew_set_pick('Wizard', null, true) $$, 'P0001', 'unknown_harness', 'harness list enforced');

set local request.jwt.claims = '{"sub":"a1111111-1111-4111-8111-111111111111","role":"authenticated"}';
select throws_ok($$ select api.queue_join('scrap_3v3_casual', '{"local": 200}', '0.1.0') $$, 'P0001', 'no_region',
  'the 150 ms cap applies at queue time');
select throws_ok($$ select api.queue_join('scrap_3v3_casual', '{"local": 20}', '0.0.9') $$, 'P0001', 'build_too_old',
  'old builds cannot queue');
select throws_ok($$ select api.queue_join('nope', '{"local": 20}', '0.1.0') $$, 'P0001', 'unknown_playlist',
  'unknown playlists are refused');
select is(api.queue_join('scrap_3v3_casual', '{"local": 20, "na-east": "fast", "mars": 1}', '0.1.0') ->> 'grade_pool',
  'hand', 'a crew queues in its highest Grade pool');

reset role;
select is((select pings_ms from game.mm_tickets), '{"local": 20}'::jsonb, 'only numeric pings for known regions are kept');
select is((select jsonb_array_length(members) from game.mm_tickets), 3, 'the ticket carries all members');

set local role authenticated;
set local request.jwt.claims = '{"sub":"a1111111-1111-4111-8111-111111111111","role":"authenticated"}';
select throws_ok($$ select api.queue_join('scrap_3v3_casual', '{"local": 20}', '0.1.0') $$, 'P0001', 'already_queued',
  'one ticket per crew');
set local request.jwt.claims = '{"sub":"a3333333-3333-4333-8333-333333333333","role":"authenticated"}';
select throws_ok($$ select api.crew_set_pick('Rigger', null, true) $$, 'P0001', 'crew_queued',
  'picks are locked while queued');
select lives_ok($$ select api.crew_leave() $$, 'a member can leave while queued');

reset role;
select is((select count(*) from game.mm_tickets), 0::bigint, 'leaving cancels the crew ticket');

set local role authenticated;
set local request.jwt.claims = '{"sub":"a1111111-1111-4111-8111-111111111111","role":"authenticated"}';
select api.crew_leave();
reset role;
select is((select leader_id from game.crews c join game.crew_members m using (crew_id)
           where m.account_id = 'a2222222-2222-4222-8222-222222222222'),
          'a2222222-2222-4222-8222-222222222222'::uuid, 'the oldest remaining member becomes leader');

insert into game.sanctions (account_id, type, reason)
values ('a2222222-2222-4222-8222-222222222222', 'queue_lock', 'test dodge');
set local role authenticated;
set local request.jwt.claims = '{"sub":"a2222222-2222-4222-8222-222222222222","role":"authenticated"}';
select throws_ok($$ select api.queue_join('scrap_3v3_casual', '{"local": 20}', '0.1.0') $$, 'P0001', 'queue_locked',
  'a queue lock blocks queueing');

select * from finish();
rollback;
