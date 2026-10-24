-- The Ledger tables and views (backend-architecture sections 7.3 and 12.3): RLS on, each
-- player reads only their own rows, and nobody writes them but the worker.
begin;
create extension if not exists pgtap with schema extensions;
select plan(16);

insert into auth.users (id, email, aud, role, raw_app_meta_data) values
  ('d1111111-1111-4111-8111-111111111111', 'dev-dora@users.kinetic.invalid', 'authenticated', 'authenticated', '{"kinetic_provider":"dev","kinetic_provider_user_id":"dora"}'),
  ('d2222222-2222-4222-8222-222222222222', 'dev-eli@users.kinetic.invalid', 'authenticated', 'authenticated', '{"kinetic_provider":"dev","kinetic_provider_user_id":"eli"}');
select api.srv_ensure_account('d1111111-1111-4111-8111-111111111111', 'dev', 'dora', 'dora');
select api.srv_ensure_account('d2222222-2222-4222-8222-222222222222', 'dev', 'eli', 'eli');

insert into game.matches (match_id, playlist_id, arena, region, build, fleet, state)
values ('e0000000-0000-4000-8000-000000000001', 'scrap_3v3_casual', 'YRD', 'local', '0.1.0', 'local', 'completed');
insert into game.rope_marks (account_id, rig, count, tier) values
  ('d1111111-1111-4111-8111-111111111111', 'Gallows', 3, 'tape'),
  ('d2222222-2222-4222-8222-222222222222', 'Kiss', 12, 'band');
insert into game.ledger_entries (account_id, entry_id) values
  ('d1111111-1111-4111-8111-111111111111', 'first-scrap'), ('d2222222-2222-4222-8222-222222222222', 'first-scrap');
insert into game.career_stats (account_id, harness, matches) values
  ('d1111111-1111-4111-8111-111111111111', 'Rigger', 4), ('d2222222-2222-4222-8222-222222222222', 'Breaker', 9);
insert into game.match_summaries (match_id, account_id, ruleset, win, tonnage, breakdown, grade_before, grade_after,
                                  weight_before, weight_after)
select 'e0000000-0000-4000-8000-000000000001', a, 1, true, 100, '[]', 1, 1, 25000, 24900
from unnest(array['d1111111-1111-4111-8111-111111111111'::uuid, 'd2222222-2222-4222-8222-222222222222'::uuid]) a;
update game.ledgers set tonnage_total = 100, weight_balance = 24900
 where account_id = 'd1111111-1111-4111-8111-111111111111';

select is((select count(*) from pg_tables
            where schemaname = 'game' and tablename in ('rope_marks', 'ledger_entries', 'career_stats', 'match_summaries')
              and rowsecurity), 4::bigint, 'RLS is on for every new Ledger table');

set local role anon;
select throws_ok($$ select * from api.my_ledger $$, '42501', null, 'anon cannot read the Ledger');

set local role authenticated;
set local request.jwt.claims = '{"sub":"d1111111-1111-4111-8111-111111111111","role":"authenticated"}';
select is((select tonnage_total from api.my_ledger), 100::bigint, 'a player reads their own Tonnage');
select is((select count(*) from api.my_ledger), 1::bigint, 'and only their own Ledger');
select is((select array_agg(rig) from api.my_rope_marks), array['Gallows'], 'their own Rope Marks');
select is((select count(*) from api.my_ledger_entries), 1::bigint, 'their own entries');
select is((select array_agg(harness) from api.my_career_stats), array['Rigger'], 'their own career');
select is((select count(*) from api.my_match_summaries), 1::bigint, 'their own match summaries');
select throws_ok($$ update game.ledgers set tonnage_total = 999999 $$, '42501', null,
  'players cannot write their Ledger');
select throws_ok($$ insert into game.rope_marks (account_id, rig, count, tier)
                    values ('d1111111-1111-4111-8111-111111111111', 'Kiss', 99, 'mark') $$, '42501', null,
  'players cannot grant themselves Rope Marks');
select throws_ok($$ select nextval('game.cutter_serial_seq') $$, '42501', null, 'players cannot draw Cutter serials');

reset role;
select throws_ok($$ update game.ledgers set weight_balance = -1
                    where account_id = 'd1111111-1111-4111-8111-111111111111' $$, '23514', null,
  'the Weight never goes negative');

-- Clients read the ruleset in force; only the service role requeues reports.
insert into game.rulesets (version, rules) values (1, '{"weight_start_t": 45000, "grade_thresholds_t": [0, 2500]}')
on conflict (version) do nothing;
set local role authenticated;
set local request.jwt.claims = '{"sub":"d1111111-1111-4111-8111-111111111111","role":"authenticated"}';
select is((select version from api.current_ruleset), 1::smallint, 'the ruleset in force is readable');
select ok((select weight_start_t from api.current_ruleset) > 0, 'with the full lease');
select throws_ok($$ select api.srv_requeue_report(gen_random_uuid()) $$, '42501', null,
  'players cannot requeue reports');
reset role;
insert into game.matches (match_id, playlist_id, arena, region, build, fleet, allocation_id, address)
values ('e0000000-0000-4000-8000-000000000002', 'scrap_3v3_casual', 'YRD', 'local', '0.1.0', 'local', 'alloc-e2', 'x');
select is((api.srv_manifest('e0000000-0000-4000-8000-000000000002', 'alloc-e2') ->> 'ruleset')::int,
  (select (value #>> '{}')::int from game.config where key = 'ruleset'), 'the manifest names the ruleset in force');

select * from finish();
rollback;
