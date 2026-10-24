-- The Manifest (backend-architecture 7.4; bible/04; menus 9): crates hold cosmetics and Ledger
-- entries only, open in order, and players see their own progress.
begin;
create extension if not exists pgtap with schema extensions;
select plan(11);

update game.claims set state = 'archived' where state in ('active', 'scheduled');
insert into game.claims (claim_id, name, arena, state, starts_at, ends_at)
values ('m-claim', 'Claim M', 'YRD', 'active', now() - interval '100 days', now() - interval '99 days');
insert into game.items (item_id, slot, source_rule, grade_gated) values
  ('tape.m_crate', 'tape', 'manifest', null), ('tape.m_shop', 'tape', 'shop', null), ('line.m_side', 'line', 'grade', 3)
on conflict (item_id) do nothing;

select lives_ok($$ insert into game.manifest_crates (claim_id, track, tier, tonnage_t, reward)
                   values ('m-claim', 'free', 1, 500, '{"items": ["tape.m_crate"], "entries": ["entry-38"]}') $$,
  'a crate of a Manifest cosmetic and an entry');
select throws_ok($$ insert into game.manifest_crates (claim_id, track, tier, tonnage_t, reward)
                    values ('m-claim', 'free', 2, 900, '{"items": ["line.m_side"]}') $$,
  'P0001', 'not_a_reward_item', 'no sidegrade ever ships in the Manifest');
select throws_ok($$ insert into game.manifest_crates (claim_id, track, tier, tonnage_t, reward)
                    values ('m-claim', 'free', 2, 900, '{"items": ["tape.m_shop"]}') $$,
  'P0001', 'not_a_reward_item', 'nor a shop item');
select throws_ok($$ insert into game.manifest_crates (claim_id, track, tier, tonnage_t, reward)
                    values ('m-claim', 'free', 2, 400, '{}') $$,
  'P0001', 'crates_out_of_order', 'a later crate needs more Tonnage');
select throws_ok($$ insert into game.entitlements (account_id, entitlement)
                    values (gen_random_uuid(), 'Sponsorship Kessl') $$,
  '23514', null, 'entitlements are named kind:ref');

insert into auth.users (id, email, aud, role, raw_app_meta_data) values
  ('ab111111-1111-4111-8111-111111111111', 'dev-lu@users.kinetic.invalid', 'authenticated', 'authenticated', '{"kinetic_provider":"dev","kinetic_provider_user_id":"lu"}');
select api.srv_ensure_account('ab111111-1111-4111-8111-111111111111', 'dev', 'lu', 'lu');
insert into game.manifest_progress (account_id, claim_id, tonnage_t)
values ('ab111111-1111-4111-8111-111111111111', 'm-claim', 600);
insert into game.manifest_crates (claim_id, track, tier, tonnage_t, reward) values
  ('m-claim', 'free', 3, 550, '{"entries": ["entry-40"]}'),
  ('m-claim', 'sponsored', 1, 100, '{"entries": ["entry-s1"]}');
select is(private.pay_manifest('ab111111-1111-4111-8111-111111111111', 'm-claim') -> 'free', '[1, 3]'::jsonb,
  'every Free crate reached is paid');
insert into game.manifest_crates (claim_id, track, tier, tonnage_t, reward)
values ('m-claim', 'free', 2, 520, '{"entries": ["entry-39"]}');
select is(private.pay_manifest('ab111111-1111-4111-8111-111111111111', 'm-claim') -> 'free', '[2]'::jsonb,
  'a crate added below the total later is paid too');
insert into game.entitlements (account_id, entitlement)
values ('ab111111-1111-4111-8111-111111111111', 'sponsorship:m-claim');
select is((select count(*) from game.ledger_entries
            where account_id = 'ab111111-1111-4111-8111-111111111111' and entry_id = 'entry-s1'), 1::bigint,
  'sponsoring pays the Sponsored crates already passed, at once');
select is((select count(*) from game.manifest_paid where account_id = 'ab111111-1111-4111-8111-111111111111'),
  4::bigint, 'and each crate is paid once');

set local role authenticated;
set local request.jwt.claims = '{"sub":"ab111111-1111-4111-8111-111111111111","role":"authenticated"}';
select is((select earned from api.my_manifest where claim_id = 'm-claim' and track = 'free' and tier = 2), true,
  'a player sees their crate ticked');
select throws_ok($$ insert into game.entitlements (account_id, entitlement)
                    values ('ab111111-1111-4111-8111-111111111111', 'sponsorship:m-claim') $$,
  '42501', null, 'players cannot sponsor themselves');

select * from finish();
rollback;
