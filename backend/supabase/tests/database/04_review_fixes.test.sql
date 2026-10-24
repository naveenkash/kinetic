-- Regression tests for the round-1 review findings.
begin;
create extension if not exists pgtap with schema extensions;
select plan(32);

insert into auth.users (id, email, aud, role, raw_app_meta_data, encrypted_password) values
  ('e1111111-1111-4111-8111-111111111111', 'dev-rone@users.kinetic.invalid', 'authenticated', 'authenticated',
   '{"kinetic_provider":"dev","kinetic_provider_user_id":"rone"}', ''),
  ('e2222222-2222-4222-8222-222222222222', 'dev-rtwo@users.kinetic.invalid', 'authenticated', 'authenticated',
   '{"kinetic_provider":"dev","kinetic_provider_user_id":"rtwo"}', ''),
  ('e3333333-3333-4333-8333-333333333333', 'dev-rthree@users.kinetic.invalid', 'authenticated', 'authenticated',
   '{"kinetic_provider":"dev","kinetic_provider_user_id":"rthree"}', ''),
  ('e4444444-4444-4444-8444-444444444444', 'dev-rfour@users.kinetic.invalid', 'authenticated', 'authenticated',
   '{"kinetic_provider":"dev","kinetic_provider_user_id":"rfour"}', ''),
  ('e5555555-5555-4555-8555-555555555555', 'dev-rfive@users.kinetic.invalid', 'authenticated', 'authenticated',
   '{"kinetic_provider":"dev","kinetic_provider_user_id":"rfive"}', ''),
  -- Made through public signup: a password and no login-function metadata.
  ('e6666666-6666-4666-8666-666666666666', 'steam-76561198000000099@users.kinetic.invalid', 'authenticated',
   'authenticated', '{"provider":"email"}', '$2a$10$abcdefghijklmnopqrstuuabcdefghijklmnopqrstuvwxyz012'),
  ('e7777777-7777-4777-8777-777777777777', 'dev-rseven@users.kinetic.invalid', 'authenticated', 'authenticated',
   '{"kinetic_provider":"dev","kinetic_provider_user_id":"someone-else"}', '');

select api.srv_ensure_account('e1111111-1111-4111-8111-111111111111', 'dev', 'rone', 'rone');
select api.srv_ensure_account('e2222222-2222-4222-8222-222222222222', 'dev', 'rtwo', 'rtwo');
select api.srv_ensure_account('e3333333-3333-4333-8333-333333333333', 'dev', 'rthree', 'rthree');
select api.srv_ensure_account('e4444444-4444-4444-8444-444444444444', 'dev', 'rfour', 'rfour');

-- Account takeover (critical): only users the login function made for this identity.
select throws_ok($$ select api.srv_ensure_account('e6666666-6666-4666-8666-666666666666', 'steam',
  '76561198000000099', null) $$, 'P0001', 'identity_conflict', 'a self-signed-up user is never adopted');
select throws_ok($$ select api.srv_ensure_account('e7777777-7777-4777-8777-777777777777', 'dev', 'rseven', null) $$,
  'P0001', 'identity_conflict', 'metadata must name this exact identity');

-- Display name collisions retry instead of locking the player out.
select matches(api.srv_ensure_account('e5555555-5555-4555-8555-555555555555', 'dev', 'rfive', 'rone') ->> 'display_name',
  '^rone_[0-9a-f]{6}$', 'a taken name gets a suffix');

-- Join codes rotate when a member is removed.
set local role authenticated;
set local request.jwt.claims = '{"sub":"e1111111-1111-4111-8111-111111111111","role":"authenticated"}';
select api.crew_invite('e2222222-2222-4222-8222-222222222222');
set local request.jwt.claims = '{"sub":"e2222222-2222-4222-8222-222222222222","role":"authenticated"}';
select api.crew_accept((select crew_id from api.my_invites limit 1));
reset role;
create temp table old_code as
  select join_code from game.crews where leader_id = 'e1111111-1111-4111-8111-111111111111';
grant select on old_code to authenticated;
set local role authenticated;
set local request.jwt.claims = '{"sub":"e1111111-1111-4111-8111-111111111111","role":"authenticated"}';
select api.crew_kick('e2222222-2222-4222-8222-222222222222');
reset role;
select isnt((select join_code from game.crews where leader_id = 'e1111111-1111-4111-8111-111111111111'),
  (select join_code from old_code), 'kicking rotates the join code');
set local role authenticated;
set local request.jwt.claims = '{"sub":"e2222222-2222-4222-8222-222222222222","role":"authenticated"}';
select throws_ok($$ select api.crew_join_by_code((select join_code from old_code)) $$, 'P0001', 'crew_not_found',
  'a kicked player cannot rejoin with the old code');

-- A crew whose match is still starting cannot take new members.
set local request.jwt.claims = '{"sub":"e3333333-3333-4333-8333-333333333333","role":"authenticated"}';
select api.crew_create();
reset role;
insert into game.matches (match_id, playlist_id, arena, region, build, fleet)
values ('f0000000-0000-4000-8000-000000000001', 'scrap_3v3_casual', 'YRD', 'local', '0.1.0', 'local');
insert into game.match_players (match_id, slot, team, account_id, harness)
values ('f0000000-0000-4000-8000-000000000001', 1, 1, 'e3333333-3333-4333-8333-333333333333', 'Rigger');
create temp table r3_code as
  select join_code from game.crews where leader_id = 'e3333333-3333-4333-8333-333333333333';
grant select on r3_code to authenticated;
set local role authenticated;
set local request.jwt.claims = '{"sub":"e4444444-4444-4444-8444-444444444444","role":"authenticated"}';
select throws_ok($$ select api.crew_join_by_code((select join_code from r3_code)) $$, 'P0001', 'crew_in_match',
  'no joining a crew whose match is starting');
reset role;
update game.matches set state = 'completed' where match_id = 'f0000000-0000-4000-8000-000000000001';
select is(private.active_match_id('e3333333-3333-4333-8333-333333333333'), null::uuid,
  'a finished match no longer counts as active');

-- Apprentice pool is Scrap and Free Scrap only.
insert into game.playlists (playlist_id, mode, team_size, teams, arenas, regions)
values ('claim_test', 'claim', 3, 2, '{YRD}', '{local}');
set local role authenticated;
set local request.jwt.claims = '{"sub":"e4444444-4444-4444-8444-444444444444","role":"authenticated"}';
select throws_ok($$ select api.queue_join('claim_test', '{"local": 10}', '0.1.0') $$, 'P0001', 'grade_too_low',
  'Grade 1 cannot queue Claim');

-- A banned account loses access at once.
reset role;
insert into game.sanctions (account_id, type, reason) values ('e4444444-4444-4444-8444-444444444444', 'account_ban', 't');
set local role authenticated;
select throws_ok($$ select api.crew_create() $$, 'P0001', 'banned', 'banned accounts cannot call api functions');
reset role;

-- Matches for report tests: rone on team 1, rtwo on team 2, two bots per team.
create function pg_temp.mk(p_id uuid, p_state text) returns void language sql as $$
  insert into game.matches (match_id, playlist_id, arena, region, build, fleet, allocation_id, address, state,
                            started_at)
  values (p_id, 'scrap_3v3_casual', 'YRD', 'local', '0.1.0', 'local', 'a', '127.0.0.1:1', p_state, now());
  insert into game.match_players (match_id, slot, team, account_id, is_bot, harness)
  values (p_id, 1, 1, 'e1111111-1111-4111-8111-111111111111', false, 'Rigger'), (p_id, 2, 1, null, true, 'Rigger'),
         (p_id, 3, 1, null, true, 'Rigger'), (p_id, 4, 2, 'e2222222-2222-4222-8222-222222222222', false, 'Rigger'),
         (p_id, 5, 2, null, true, 'Rigger'), (p_id, 6, 2, null, true, 'Rigger');
$$;
create function pg_temp.p(p_account text, p_team int, p_bot boolean) returns jsonb language sql as $$
  select jsonb_build_object('account_id', p_account, 'team', p_team, 'is_bot', p_bot, 'connected_seconds', 1)
$$;
create function pg_temp.report(p_id uuid, p_players jsonb) returns jsonb language sql as $$
  select api.srv_store_report(p_id, 'a', jsonb_build_object('match_id', p_id, 'players', p_players))
$$;

select pg_temp.mk('f1000000-0000-4000-8000-000000000001', 'in_progress');
select is(pg_temp.report('f1000000-0000-4000-8000-000000000001', jsonb_build_array(
  pg_temp.p('e1111111-1111-4111-8111-111111111111', 1, false), pg_temp.p('e1111111-1111-4111-8111-111111111111', 1, false),
  pg_temp.p('e2222222-2222-4222-8222-222222222222', 2, false), pg_temp.p(null, 1, true), pg_temp.p(null, 2, true),
  pg_temp.p(null, 2, true))) ->> 'reason', 'duplicate_player', 'a duplicated player is quarantined');

select pg_temp.mk('f1000000-0000-4000-8000-000000000002', 'in_progress');
select is(pg_temp.report('f1000000-0000-4000-8000-000000000002', jsonb_build_array(
  pg_temp.p('e1111111-1111-4111-8111-111111111111', 2, false), pg_temp.p('e2222222-2222-4222-8222-222222222222', 1, false),
  pg_temp.p(null, 1, true), pg_temp.p(null, 1, true), pg_temp.p(null, 2, true), pg_temp.p(null, 2, true))) ->> 'reason',
  'roster_mismatch', 'swapped teams are quarantined');

select pg_temp.mk('f1000000-0000-4000-8000-000000000003', 'in_progress');
select is(pg_temp.report('f1000000-0000-4000-8000-000000000003', jsonb_build_array(
  pg_temp.p('e1111111-1111-4111-8111-111111111111', 1, false), pg_temp.p('e2222222-2222-4222-8222-222222222222', 2, false),
  pg_temp.p(null, 1, true), pg_temp.p(null, 1, true), pg_temp.p(null, 1, true), pg_temp.p(null, 2, true))) ->> 'reason',
  'bot_mismatch', 'bots on the wrong team are quarantined');

select pg_temp.mk('f1000000-0000-4000-8000-000000000004', 'in_progress');
select is(pg_temp.report('f1000000-0000-4000-8000-000000000004', jsonb_build_array(
  pg_temp.p('e1111111-1111-4111-8111-111111111111', 1, false),
  pg_temp.p('e2222222-2222-4222-8222-222222222222', 2, false) || '{"abandoned": "maybe"}',
  pg_temp.p(null, 1, true), pg_temp.p(null, 1, true), pg_temp.p(null, 2, true), pg_temp.p(null, 2, true))) ->> 'reason',
  'malformed_player', 'a malformed field quarantines instead of raising');

select pg_temp.mk('f1000000-0000-4000-8000-000000000005', 'in_progress');
select is(pg_temp.report('f1000000-0000-4000-8000-000000000005', jsonb_build_array(
  pg_temp.p('e1111111-1111-4111-8111-111111111111', 1, false), pg_temp.p('------------------------------------', 2, false),
  pg_temp.p(null, 1, true), pg_temp.p(null, 1, true), pg_temp.p(null, 2, true), pg_temp.p(null, 2, true))) ->> 'reason',
  'malformed_player', 'a non-uuid account id quarantines instead of raising');

select pg_temp.mk('f1000000-0000-4000-8000-000000000006', 'aborted');
select is(pg_temp.report('f1000000-0000-4000-8000-000000000006', jsonb_build_array(
  pg_temp.p('e1111111-1111-4111-8111-111111111111', 1, false), pg_temp.p('e2222222-2222-4222-8222-222222222222', 2, false),
  pg_temp.p(null, 1, true), pg_temp.p(null, 1, true), pg_temp.p(null, 2, true), pg_temp.p(null, 2, true))) ->> 'reason',
  'match_aborted', 'a report after an abort is kept, quarantined');
select is((select state from game.matches where match_id = 'f1000000-0000-4000-8000-000000000006'), 'aborted',
  'and the match stays aborted');

-- The first heartbeat does not mark a server ready before the worker records its address.
insert into game.matches (match_id, playlist_id, arena, region, build, fleet, allocation_id)
values ('f2000000-0000-4000-8000-000000000001', 'scrap_3v3_casual', 'YRD', 'local', '0.1.0', 'local', 'early');
select is(api.srv_heartbeat('f2000000-0000-4000-8000-000000000001', 'early', 'waiting_for_players', '{}') ->> 'state',
  'allocating', 'no ready state without an address');

-- Reconnect tickets only inside the reconnect window (60 s casual).
select pg_temp.mk('f3000000-0000-4000-8000-000000000001', 'in_progress');
update game.match_players set left_at = now() - interval '61 seconds'
 where match_id = 'f3000000-0000-4000-8000-000000000001' and slot = 1;
update game.matches set state = 'completed'
 where match_id in ('f1000000-0000-4000-8000-000000000001', 'f1000000-0000-4000-8000-000000000002',
                    'f1000000-0000-4000-8000-000000000003', 'f1000000-0000-4000-8000-000000000004',
                    'f1000000-0000-4000-8000-000000000005');
select is(api.srv_current_match('e1111111-1111-4111-8111-111111111111'), null, 'no ticket after the window');
update game.match_players set left_at = now() - interval '30 seconds'
 where match_id = 'f3000000-0000-4000-8000-000000000001' and slot = 1;
select is(api.srv_current_match('e1111111-1111-4111-8111-111111111111') ->> 'slot', '1', 'a ticket inside the window');

-- Deleting an account works for a crew leader with match history, and anonymizes the rows.
delete from auth.users where id = 'e1111111-1111-4111-8111-111111111111';
select is((select count(*) from game.accounts where account_id = 'e1111111-1111-4111-8111-111111111111'), 0::bigint,
  'the account is gone');
select is((select count(*) from game.match_players where match_id = 'f3000000-0000-4000-8000-000000000001'
           and slot = 1 and account_id is null and not is_bot), 1::bigint, 'its match rows are kept, anonymized');
select is((select count(*) from game.crews where leader_id = 'e1111111-1111-4111-8111-111111111111'), 0::bigint,
  'its crew is removed');

-- Round 2 -------------------------------------------------------------------------------

-- A player deleted mid-match does not break heartbeats, and the report still validates.
insert into auth.users (id, email, aud, role, raw_app_meta_data) values
  ('e8888888-8888-4888-8888-888888888888', 'dev-reight@users.kinetic.invalid', 'authenticated', 'authenticated',
   '{"kinetic_provider":"dev","kinetic_provider_user_id":"reight"}'),
  ('e9999999-9999-4999-8999-999999999999', 'dev-rnine@users.kinetic.invalid', 'authenticated', 'authenticated',
   '{"kinetic_provider":"dev","kinetic_provider_user_id":"rnine"}');
select api.srv_ensure_account('e8888888-8888-4888-8888-888888888888', 'dev', 'reight', 'reight');
select api.srv_ensure_account('e9999999-9999-4999-8999-999999999999', 'dev', 'rnine', 'rnine');
insert into game.matches (match_id, playlist_id, arena, region, build, fleet, allocation_id, address, state, started_at)
values ('f4000000-0000-4000-8000-000000000001', 'scrap_3v3_casual', 'YRD', 'local', '0.1.0', 'local', 'd', '127.0.0.1:1',
        'in_progress', now());
insert into game.match_players (match_id, slot, team, account_id, is_bot, harness)
values ('f4000000-0000-4000-8000-000000000001', 1, 1, 'e8888888-8888-4888-8888-888888888888', false, 'Rigger'),
       ('f4000000-0000-4000-8000-000000000001', 2, 2, 'e9999999-9999-4999-8999-999999999999', false, 'Rigger');
delete from auth.users where id = 'e9999999-9999-4999-8999-999999999999';
select lives_ok($$ select api.srv_heartbeat('f4000000-0000-4000-8000-000000000001', 'd', 'in_progress',
  array['e8888888-8888-4888-8888-888888888888'::uuid]) $$, 'heartbeats survive a deleted player');
select is((select api.srv_store_report('f4000000-0000-4000-8000-000000000001', 'd', jsonb_build_object(
  'match_id', 'f4000000-0000-4000-8000-000000000001',
  'players', jsonb_build_array(
    jsonb_build_object('account_id', 'e8888888-8888-4888-8888-888888888888', 'team', 1, 'is_bot', false, 'connected_seconds', 1),
    jsonb_build_object('account_id', 'e9999999-9999-4999-8999-999999999999', 'team', 2, 'is_bot', false, 'connected_seconds', 1))))
  ->> 'status'), 'stored', 'a report naming a since-deleted player is still stored');

-- Deleting a queued crew's leader hands the crew on and cancels its ticket.
set local role authenticated;
set local request.jwt.claims = '{"sub":"e8888888-8888-4888-8888-888888888888","role":"authenticated"}';
select api.crew_invite('e5555555-5555-4555-8555-555555555555');
set local request.jwt.claims = '{"sub":"e5555555-5555-4555-8555-555555555555","role":"authenticated"}';
select api.crew_accept((select crew_id from api.my_invites limit 1));
select api.crew_set_pick('Rigger', null, true);
reset role;
update game.matches set state = 'completed' where match_id = 'f4000000-0000-4000-8000-000000000001';
set local role authenticated;
set local request.jwt.claims = '{"sub":"e8888888-8888-4888-8888-888888888888","role":"authenticated"}';
select api.queue_join('scrap_3v3_casual', '{"local": 10}', '0.1.0');
reset role;
create temp table chat_before as
  select chat_key from game.crews where leader_id = 'e8888888-8888-4888-8888-888888888888';
delete from auth.users where id = 'e8888888-8888-4888-8888-888888888888';
select is((select leader_id from game.crews c join game.crew_members m using (crew_id)
           where m.account_id = 'e5555555-5555-4555-8555-555555555555'),
  'e5555555-5555-4555-8555-555555555555'::uuid, 'the remaining member leads');
select is((select count(*) from game.mm_tickets t join game.crew_members m using (crew_id)
           where m.account_id = 'e5555555-5555-4555-8555-555555555555'), 0::bigint, 'the ticket is cancelled');
select isnt((select chat_key from game.crews c join game.crew_members m using (crew_id)
             where m.account_id = 'e5555555-5555-4555-8555-555555555555'),
  (select chat_key from chat_before), 'the chat key rotates when a member leaves');

-- A report for a match that never became ready leaves it for the boot timeout.
insert into game.matches (match_id, playlist_id, arena, region, build, fleet, allocation_id)
values ('f5000000-0000-4000-8000-000000000001', 'scrap_3v3_casual', 'YRD', 'local', '0.1.0', 'local', 'n');
select is((select api.srv_store_report('f5000000-0000-4000-8000-000000000001', 'n', jsonb_build_object(
  'match_id', 'f5000000-0000-4000-8000-000000000001', 'players', '[]'::jsonb)) ->> 'reason'),
  'match_not_started', 'an early report is quarantined');
select is((select state from game.matches where match_id = 'f5000000-0000-4000-8000-000000000001'), 'allocating',
  'and the match is not marked completed');

-- Round 3 -------------------------------------------------------------------------------

-- A kicked player cannot rejoin by code, even with the new code; an invite still works.
select ok(not (private.crew_state((select crew_id from game.crew_members
  where account_id = 'e5555555-5555-4555-8555-555555555555')) ? 'join_code'),
  'broadcast crew state carries no join code');
set local role authenticated;
set local request.jwt.claims = '{"sub":"e5555555-5555-4555-8555-555555555555","role":"authenticated"}';
select api.crew_invite('e3333333-3333-4333-8333-333333333333');
set local request.jwt.claims = '{"sub":"e3333333-3333-4333-8333-333333333333","role":"authenticated"}';
select api.crew_accept((select crew_id from api.my_invites limit 1));
set local request.jwt.claims = '{"sub":"e5555555-5555-4555-8555-555555555555","role":"authenticated"}';
select api.crew_kick('e3333333-3333-4333-8333-333333333333');
reset role;
create temp table new_code as
  select join_code from game.crews c join game.crew_members m using (crew_id)
  where m.account_id = 'e5555555-5555-4555-8555-555555555555';
grant select on new_code to authenticated;
set local role authenticated;
set local request.jwt.claims = '{"sub":"e3333333-3333-4333-8333-333333333333","role":"authenticated"}';
select throws_ok($$ select api.crew_join_by_code((select join_code from new_code)) $$, 'P0001', 'kicked',
  'a kicked player cannot rejoin with the new code');
set local request.jwt.claims = '{"sub":"e5555555-5555-4555-8555-555555555555","role":"authenticated"}';
select api.crew_invite('e3333333-3333-4333-8333-333333333333');
set local request.jwt.claims = '{"sub":"e3333333-3333-4333-8333-333333333333","role":"authenticated"}';
select lives_ok($$ select api.crew_accept((select crew_id from api.my_invites limit 1)) $$,
  'an invite from the crew lets them back');
reset role;

select * from finish();
rollback;
