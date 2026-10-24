-- Claims, Allegiance and Contracts (backend-architecture sections 7.5 and 14).
begin;
create extension if not exists pgtap with schema extensions;
select plan(29);

insert into auth.users (id, email, aud, role, raw_app_meta_data) values
  ('c1111111-1111-4111-8111-111111111111', 'dev-hal@users.kinetic.invalid', 'authenticated', 'authenticated', '{"kinetic_provider":"dev","kinetic_provider_user_id":"hal"}'),
  ('c2222222-2222-4222-8222-222222222222', 'dev-ivy@users.kinetic.invalid', 'authenticated', 'authenticated', '{"kinetic_provider":"dev","kinetic_provider_user_id":"ivy"}');
select api.srv_ensure_account('c1111111-1111-4111-8111-111111111111', 'dev', 'hal', 'hal');
select api.srv_ensure_account('c2222222-2222-4222-8222-222222222222', 'dev', 'ivy', 'ivy');

-- Other Claims a local database may hold step aside for this test (rolled back at the end).
update game.claims set state = 'archived' where state in ('active', 'scheduled');

set local role authenticated;
set local request.jwt.claims = '{"sub":"c1111111-1111-4111-8111-111111111111","role":"authenticated"}';
select throws_ok($$ select api.declare_allegiance('guild') $$, 'P0001', 'no_active_claim',
  'Allegiance needs an active Claim');

reset role;
insert into game.claims (claim_id, name, arena, state, starts_at, ends_at) values
  ('t-claim', 'Claim T', 'HGT', 'active', now() - interval '8 days', now() + interval '20 days'),
  ('t-next', 'Claim N', 'BLS', 'scheduled', now() + interval '20 days', now() + interval '50 days');
select throws_ok($$ insert into game.claims (claim_id, name, arena, state, starts_at, ends_at)
                    values ('t-two', 'Two', 'YRD', 'active', now() + interval '60 days', now() + interval '61 days') $$,
  '23505', null, 'only one Claim is active at a time');
select throws_ok($$ insert into game.claims (claim_id, name, arena, starts_at, ends_at)
                    values ('t-overlap', 'Overlap', 'YRD', now() + interval '19 days', now() + interval '25 days') $$,
  '23P01', null, 'Claims never overlap');
select throws_ok($$ insert into game.contracts (contract_id, claim_id, faction, week, counter_key, target, title, reward)
                    values ('t-bad', 't-claim', 'guild', 1, 'snaps_survived', 1, 'Bad', '{"items": ["tape.typo"]}') $$,
  'P0001', 'not_a_reward_item', 'a Contract reward names real Contract items only');
select throws_ok($$ insert into game.contracts (contract_id, claim_id, faction, week, counter_key, target, title, reward)
                    values ('t-bad', 't-claim', 'guild', 1, 'snaps_survived', 1, 'Bad', '{"items": "tape.chevron"}') $$,
  'P0001', 'bad_reward', 'and lists them');
insert into game.contracts (contract_id, claim_id, faction, week, counter_key, target, title) values
  ('t-guild-w1', 't-claim', 'guild', 1, 'snaps_survived', 1, 'Week one'),
  ('t-guild-w2', 't-claim', 'guild', 2, 'snaps_survived', 5, 'Week two'),
  ('t-guild-w3', 't-claim', 'guild', 3, 'snaps_survived', 5, 'Week three'),
  ('t-fray-w2', 't-claim', 'fray', 2, 'snaps_survived', 5, 'Fray');

set local role authenticated;
set local request.jwt.claims = '{"sub":"c1111111-1111-4111-8111-111111111111","role":"authenticated"}';
select throws_ok($$ select api.declare_allegiance('office') $$, 'P0001', 'unknown_faction', 'factions are the five');
select is(api.declare_allegiance('guild') ->> 'switched', 'false', 'a first declaration');
select is((select faction from api.my_allegiance), 'guild', 'is recorded');
select throws_ok($$ select faction from game.allegiance_history $$, '42501', null,
  'the declaration history is the worker''s');
select is((select array_agg(contract_id order by contract_id) from api.my_contracts),
  array['t-guild-w1', 't-guild-w2'], 'my faction''s Contracts, up to this week only');
select is((select count(*) from api.claims where claim_id = 't-next'), 0::bigint, 'a scheduled Claim is not shown');

reset role;
insert into game.contract_progress (account_id, contract_id, progress, completed_at) values
  ('c1111111-1111-4111-8111-111111111111', 't-guild-w1', 1, now()),
  ('c1111111-1111-4111-8111-111111111111', 't-guild-w2', 3, null);
set local role authenticated;
set local request.jwt.claims = '{"sub":"c1111111-1111-4111-8111-111111111111","role":"authenticated"}';
select is(api.declare_allegiance('fray') ->> 'switched', 'true', 'switching mid-Claim');
reset role;
select is((select array_agg(faction order by declared_at) from game.allegiance_history
            where account_id = 'c1111111-1111-4111-8111-111111111111'), array['guild', 'fray'],
  'both declarations are kept');
set local role authenticated;
set local request.jwt.claims = '{"sub":"c1111111-1111-4111-8111-111111111111","role":"authenticated"}';
select is((select array_agg(contract_id) from game.contract_progress), array['t-guild-w1'],
  'keeps completed Contracts and resets the rest');
select is((select array_agg(contract_id) from api.my_contracts), array['t-fray-w2'], 'and opens the new faction''s');
select throws_ok($$ insert into game.allegiances (account_id, claim_id, faction)
                    values ('c1111111-1111-4111-8111-111111111111', 't-claim', 'htr') $$, '42501', null,
  'players cannot write Allegiance directly');

-- Standing events are append-only.
reset role;
insert into game.claim_standing_events (claim_id, faction, points, reason, account_id, week)
values ('t-claim', 'fray', 3, 'match', 'c1111111-1111-4111-8111-111111111111', 2);
select throws_ok($$ update game.claim_standing_events set points = 60 where claim_id = 't-claim' $$,
  'P0001', 'standing_events_are_append_only', 'points cannot be changed');
select throws_ok($$ delete from game.claim_standing_events where claim_id = 't-claim' $$,
  'P0001', 'standing_events_are_append_only', 'or deleted');
delete from auth.users where id = 'c1111111-1111-4111-8111-111111111111';
select is((select points from game.claim_standing_events where claim_id = 't-claim'), 3::numeric,
  'a deleted account''s points stay with the faction');

-- The flip: a person reviews, then resolves; the variant goes to config.
set local role authenticated;
set local request.jwt.claims = '{"sub":"c2222222-2222-4222-8222-222222222222","role":"authenticated"}';
select throws_ok($$ select api.srv_claim_resolve('t-claim') $$, '42501', null, 'players cannot resolve Claims');
reset role;
update game.claims set state = 'tallied', outcome = '{"holder": "htr", "variant": "district_removed"}'
 where claim_id = 't-claim';
select throws_ok($$ select api.srv_claim_resolve('t-claim') $$, 'P0001', 'not_reviewed', 'review comes first');
select throws_ok($$ select api.srv_claim_review('t-claim', 'staff-1', 'guild', null) $$, 'P0001', 'bad_override',
  'an override names both the holder and the variant');
select throws_ok($$ select api.srv_claim_review('t-claim', 'staff-1', 'htr', 'district_remove') $$, 'P0001',
  'bad_override', 'and a variant the arena can take');
select throws_ok($$ insert into game.playlists (playlist_id, mode, team_size, teams, rated, arenas, regions)
                    values ('t-rated-haul', 'haul', 3, 2, true, '{YRD}', '{local}') $$, '23514', null,
  'only Scrap and Claim are Rated');
update game.claims set review = '{"anomalies": {"capped_every_week": ["someone"]}}' where claim_id = 't-claim';
select is(api.srv_claim_review('t-claim', 'staff-1') ->> 'state', 'reviewed', 'a person signs off');
select is(api.srv_claim_resolve('t-claim') ->> 'state', 'resolved', 'then resolves it');
select is((select value #>> '{}' from game.config where key = 'variant:HGT'), 'district_removed',
  'which writes the arena variant for next season');
set local role authenticated;
set local request.jwt.claims = '{"sub":"c2222222-2222-4222-8222-222222222222","role":"authenticated"}';
select is((select outcome ->> 'holder' from api.claims where claim_id = 't-claim'), 'htr',
  'players read the resolved outcome');
select throws_ok($$ select review from game.claims $$, '42501', null, 'but never the reviewer''s notes');

select * from finish();
rollback;
