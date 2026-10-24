-- Server endpoints and report validation (backend-architecture sections 11 and 12).
begin;
create extension if not exists pgtap with schema extensions;
select plan(13);

insert into auth.users (id, email, aud, role, raw_app_meta_data) values
  ('b1111111-1111-4111-8111-111111111111', 'dev-one@users.kinetic.invalid', 'authenticated', 'authenticated', '{"kinetic_provider":"dev","kinetic_provider_user_id":"one"}'),
  ('b2222222-2222-4222-8222-222222222222', 'dev-two@users.kinetic.invalid', 'authenticated', 'authenticated', '{"kinetic_provider":"dev","kinetic_provider_user_id":"two"}');
select api.srv_ensure_account('b1111111-1111-4111-8111-111111111111', 'dev', 'one', 'one');
select api.srv_ensure_account('b2222222-2222-4222-8222-222222222222', 'dev', 'two', 'two');

insert into game.matches (match_id, playlist_id, arena, region, build, fleet, allocation_id, address)
values ('c0000000-0000-4000-8000-000000000001', 'scrap_3v3_casual', 'YRD', 'local', '0.1.0', 'local', 'alloc-1', '127.0.0.1:1'),
       ('c0000000-0000-4000-8000-000000000002', 'scrap_3v3_casual', 'YRD', 'local', '0.1.0', 'local', 'alloc-2', '127.0.0.1:2');
insert into game.match_players (match_id, slot, team, account_id, is_bot, harness)
select m, s.slot, s.team, s.account_id, s.account_id is null, 'Rigger'
from unnest(array['c0000000-0000-4000-8000-000000000001'::uuid, 'c0000000-0000-4000-8000-000000000002'::uuid]) m,
     (values (1, 1, 'b1111111-1111-4111-8111-111111111111'::uuid), (2, 1, null), (3, 1, null),
             (4, 2, 'b2222222-2222-4222-8222-222222222222'::uuid), (5, 2, null), (6, 2, null)) s(slot, team, account_id);

select throws_ok($$ select api.srv_manifest('c0000000-0000-4000-8000-000000000001', 'wrong') $$, 'P0001', 'unknown_match',
  'the allocation id must match');
select is(jsonb_array_length(api.srv_manifest('c0000000-0000-4000-8000-000000000001', 'alloc-1') -> 'players'), 6,
  'the manifest lists every slot');

select is(api.srv_heartbeat('c0000000-0000-4000-8000-000000000001', 'alloc-1', 'waiting_for_players', '{}') ->> 'state',
  'waiting_for_players', 'the first heartbeat marks the server ready');
select is((select ready_at is not null from game.matches where match_id = 'c0000000-0000-4000-8000-000000000001'), true,
  'ready_at is set');
select api.srv_player_event('c0000000-0000-4000-8000-000000000001', 'alloc-1', 'b1111111-1111-4111-8111-111111111111', 'joined');
select is((select api.srv_current_match('b1111111-1111-4111-8111-111111111111') ->> 'slot'), '1',
  'a joined player can fetch a reconnect ticket for their slot');
select is(api.srv_heartbeat('c0000000-0000-4000-8000-000000000001', 'alloc-1', 'in_progress',
  array['b1111111-1111-4111-8111-111111111111'::uuid]) ->> 'state', 'in_progress', 'the server goes live');
select throws_ok($$ select api.srv_player_event('c0000000-0000-4000-8000-000000000001', 'alloc-1',
  'b1111111-1111-4111-8111-111111111111', 'teleported') $$, 'P0001', 'bad_event', 'unknown events are refused');

select is(api.srv_store_report('c0000000-0000-4000-8000-000000000001', 'alloc-1', jsonb_build_object(
  'match_id', 'c0000000-0000-4000-8000-000000000001', 'winner_team', 1,
  'players', jsonb_build_array(
    jsonb_build_object('account_id', 'b1111111-1111-4111-8111-111111111111', 'team', 1, 'is_bot', false, 'connected_seconds', 1),
    jsonb_build_object('account_id', 'b2222222-2222-4222-8222-222222222222', 'team', 2, 'is_bot', false, 'connected_seconds', 0,
                       'abandoned', true),
    jsonb_build_object('account_id', null, 'team', 1, 'is_bot', true, 'connected_seconds', 1),
    jsonb_build_object('account_id', null, 'team', 1, 'is_bot', true, 'connected_seconds', 1),
    jsonb_build_object('account_id', null, 'team', 2, 'is_bot', true, 'connected_seconds', 1),
    jsonb_build_object('account_id', null, 'team', 2, 'is_bot', true, 'connected_seconds', 1)))) ->> 'status',
  'stored', 'a valid report is stored');
select is((select state from game.matches where match_id = 'c0000000-0000-4000-8000-000000000001'), 'completed',
  'the match completes');
select is((select count(*) from game.abandons where match_id = 'c0000000-0000-4000-8000-000000000001'), 1::bigint,
  'reported abandons are recorded');
select is(api.srv_store_report('c0000000-0000-4000-8000-000000000001', 'alloc-1', '{}') ->> 'status', 'duplicate',
  'a second report for the same match is ignored');

-- A report that names the wrong players is quarantined, not stored.
select api.srv_heartbeat('c0000000-0000-4000-8000-000000000002', 'alloc-2', 'in_progress', '{}');
select is(api.srv_store_report('c0000000-0000-4000-8000-000000000002', 'alloc-2', jsonb_build_object(
  'match_id', 'c0000000-0000-4000-8000-000000000002',
  'players', jsonb_build_array(
    jsonb_build_object('account_id', 'b1111111-1111-4111-8111-111111111111', 'team', 1, 'is_bot', false, 'connected_seconds', 1),
    jsonb_build_object('account_id', 'd9999999-9999-4999-8999-999999999999', 'team', 2, 'is_bot', false, 'connected_seconds', 1),
    jsonb_build_object('team', 1, 'is_bot', true, 'connected_seconds', 1), jsonb_build_object('team', 1, 'is_bot', true, 'connected_seconds', 1),
    jsonb_build_object('team', 2, 'is_bot', true, 'connected_seconds', 1), jsonb_build_object('team', 2, 'is_bot', true, 'connected_seconds', 1))))
  ->> 'reason', 'roster_mismatch', 'a roster mismatch is quarantined');
select is((select status from game.match_reports where match_id = 'c0000000-0000-4000-8000-000000000002'), 'quarantined',
  'and stored as quarantined');

select * from finish();
rollback;
