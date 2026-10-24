-- Inventory, loadouts, wallets and the shop (backend-architecture sections 7.4 and 13).
begin;
create extension if not exists pgtap with schema extensions;
select plan(33);

insert into auth.users (id, email, aud, role, raw_app_meta_data) values
  ('f1111111-1111-4111-8111-111111111111', 'dev-fay@users.kinetic.invalid', 'authenticated', 'authenticated', '{"kinetic_provider":"dev","kinetic_provider_user_id":"fay"}'),
  ('f2222222-2222-4222-8222-222222222222', 'dev-gus@users.kinetic.invalid', 'authenticated', 'authenticated', '{"kinetic_provider":"dev","kinetic_provider_user_id":"gus"}');
select api.srv_ensure_account('f1111111-1111-4111-8111-111111111111', 'dev', 'fay', 'fay');
select api.srv_ensure_account('f2222222-2222-4222-8222-222222222222', 'dev', 'gus', 'gus');

insert into game.items (item_id, slot, source_rule, grade_gated) values
  ('line.t_long', 'line', 'grade', 3),
  ('tape.t_shop', 'tape', 'shop', null),
  ('tape.t_shop_two', 'tape', 'shop', null),
  ('livery.t_contract', 'livery', 'contract', null)
on conflict (item_id) do nothing;
insert into game.shop_rotation (item_id, currency, price, starts_at, ends_at) values
  ('tape.t_shop', 'tonnage_credit', 300, now() - interval '1 day', now() + interval '1 day'),
  ('tape.t_shop_two', 'tonnage_credit', 900, now() - interval '1 day', now() + interval '1 day');
insert into game.wallets (account_id, currency, balance) values
  ('f1111111-1111-4111-8111-111111111111', 'tonnage_credit', 500);

-- The catalog rules hold in the database itself.
select throws_ok($$ insert into game.items (item_id, slot, source_rule) values ('line.t_sold', 'line', 'shop') $$,
  '23514', null, 'a sidegrade cannot be a shop item');
select throws_ok($$ insert into game.items (item_id, slot, source_rule) values ('dog.t_shipped', 'dog', 'manifest') $$,
  '23514', null, 'nor ship in the Manifest');
select throws_ok($$ insert into game.shop_rotation (item_id, currency, price, starts_at, ends_at)
                    values ('livery.t_contract', 'tonnage_credit', 1, now(), now() + interval '1 day') $$,
  'P0001', 'not_a_shop_item', 'only shop items are listed');
select throws_ok($$ update game.items set source_rule = 'contract' where item_id = 'tape.t_shop' $$,
  'P0001', 'item_listed_in_shop', 'a listed item cannot leave the shop while listed');
select throws_ok($$ update game.items set retired = true where item_id = 'tape.t_shop' $$,
  'P0001', 'item_listed_in_shop', 'nor be retired while listed');
select throws_ok($$ insert into game.items (item_id, slot, source_rule) values ('cutter.t_admin', 'cutter', 'admin') $$,
  '23514', null, 'a sidegrade cannot be an admin grant');
select throws_ok($$ insert into game.items (item_id, slot, source_rule) values ('cutter.t_ungated', 'cutter', 'contract') $$,
  '23514', null, 'a sidegrade always has a Grade');
select throws_ok($$ insert into game.inventory (account_id, item_id, source)
                    values ('f1111111-1111-4111-8111-111111111111', 'line.t_long', 'shop') $$,
  'P0001', 'sidegrade_not_grantable', 'a sidegrade reaches an inventory only from a Contract');

set local role authenticated;
set local request.jwt.claims = '{"sub":"f1111111-1111-4111-8111-111111111111","role":"authenticated"}';

-- Loadouts: Grade gates and ownership.
select is(api.set_loadout('Rigger', 'line.standard', 'dog.standard', 'cutter.standard') ->> 'line', 'line.standard',
  'the baseline is always allowed');
select throws_ok($$ select api.set_loadout('Rigger', 'line.t_long', 'dog.standard', 'cutter.standard') $$,
  'P0001', 'not_owned', 'a sidegrade above the Grade is refused');
select throws_ok($$ select api.set_loadout('Rigger', 'dog.standard', 'dog.standard', 'cutter.standard') $$,
  'P0001', 'not_owned', 'an item in the wrong slot is refused');
select throws_ok($$ select api.set_loadout('Rigger', 'line.standard', 'dog.standard', 'cutter.standard',
                                           '{"livery": "livery.t_contract"}') $$,
  'P0001', 'not_owned', 'a cosmetic not owned is refused');
select throws_ok($$ select api.set_loadout('Rigger', 'line.standard', 'dog.standard', 'cutter.standard',
                                           '{"line": "line.standard"}') $$,
  'P0001', 'not_owned', 'sidegrades are not cosmetics');
select throws_ok($$ select api.set_loadout('Wizard', 'line.standard', 'dog.standard', 'cutter.standard') $$,
  'P0001', 'unknown_harness', 'unknown harnesses are refused');
select is((select count(*) from api.my_items where item_id = 'line.t_long'), 0::bigint,
  'a locked sidegrade is not among my items');

reset role;
update game.ledgers set grade = 3 where account_id = 'f1111111-1111-4111-8111-111111111111';
insert into game.inventory (account_id, item_id, source) values
  ('f1111111-1111-4111-8111-111111111111', 'livery.t_contract', 'contract');
set local role authenticated;
set local request.jwt.claims = '{"sub":"f1111111-1111-4111-8111-111111111111","role":"authenticated"}';
select is(api.set_loadout('Rigger', 'line.t_long', 'dog.standard', 'cutter.standard',
                          '{"livery": "livery.t_contract"}') -> 'cosmetics' ->> 'livery', 'livery.t_contract',
  'Grade 3 unlocks the sidegrade and the owned cosmetic equips');
select is((select count(*) from api.my_items where item_id in ('line.t_long', 'livery.t_contract')), 2::bigint,
  'both are among my items');

-- The match manifest re-validates: a retired or lost item falls back to the baseline.
reset role;
update game.items set retired = true where item_id = 'line.t_long';
delete from game.inventory where item_id = 'livery.t_contract';
insert into game.matches (match_id, playlist_id, arena, region, build, fleet)
values ('a0000000-0000-4000-8000-0000000000f1', 'scrap_3v3_casual', 'YRD', 'local', '0.1.0', 'local');
insert into game.match_players (match_id, slot, team, account_id, harness) values
  ('a0000000-0000-4000-8000-0000000000f1', 1, 1, 'f1111111-1111-4111-8111-111111111111', 'Rigger');
select is((select loadout from game.match_players where match_id = 'a0000000-0000-4000-8000-0000000000f1'),
  '{"line": "line.standard", "dog": "dog.standard", "cutter": "cutter.standard", "cosmetics": {}}'::jsonb,
  'the match carries only what the player owns now');

-- The shop.
set local role authenticated;
set local request.jwt.claims = '{"sub":"f1111111-1111-4111-8111-111111111111","role":"authenticated"}';
select throws_ok($$ select api.shop_purchase('tape.t_shop', 250, gen_random_uuid()) $$,
  'P0001', 'price_changed', 'the price must be the one shown');
select throws_ok($$ select api.shop_purchase('tape.t_shop_two', 900, gen_random_uuid()) $$,
  'P0001', 'insufficient_funds', 'the balance must cover it');
select throws_ok($$ select api.shop_purchase('line.standard', 1, gen_random_uuid()) $$,
  'P0001', 'not_listed', 'unlisted items cannot be bought');
select throws_ok($$ select api.shop_purchase('tape.t_shop', null, gen_random_uuid()) $$,
  'P0001', 'price_changed', 'a missing price is not a price');
select is(api.shop_purchase('tape.t_shop', 300, 'aaaaaaaa-0000-4000-8000-000000000001') ->> 'balance', '200',
  'a purchase debits the wallet');
select is(api.shop_purchase('tape.t_shop', 300, 'aaaaaaaa-0000-4000-8000-000000000001') ->> 'status', 'duplicate',
  'a retried purchase is not charged twice');
select throws_ok($$ select api.shop_purchase('tape.t_shop', 300, gen_random_uuid()) $$,
  'P0001', 'already_owned', 'an owned item is not sold again');
select is((select balance from api.my_wallets where currency = 'tonnage_credit'), 200::bigint,
  'the wallet shows the balance');
select is((select count(*) from api.my_wallet_txns), 1::bigint, 'one transaction was written');

set local request.jwt.claims = '{"sub":"f2222222-2222-4222-8222-222222222222","role":"authenticated"}';
select is((select count(*) from api.my_wallets), 0::bigint, 'another player sees no wallet of mine');
select throws_ok($$ update game.wallets set balance = 999999 $$, '42501', null, 'players cannot write wallets');

-- A retired item is not sold, even while a rotation still names it.
reset role;
alter table game.items disable trigger items_listed;
update game.items set retired = true where item_id = 'tape.t_shop_two';
alter table game.items enable trigger items_listed;
update game.wallets set balance = 5000 where account_id = 'f1111111-1111-4111-8111-111111111111';
set local role authenticated;
set local request.jwt.claims = '{"sub":"f1111111-1111-4111-8111-111111111111","role":"authenticated"}';
select throws_ok($$ select api.shop_purchase('tape.t_shop_two', 900, gen_random_uuid()) $$,
  'P0001', 'not_listed', 'a retired item cannot be bought');

-- The server's loadout at a reset reads what the player owns now.
select throws_ok($$ select api.srv_player_loadout('a0000000-0000-4000-8000-0000000000f1', 'x',
                                                  'f1111111-1111-4111-8111-111111111111') $$,
  '42501', null, 'players cannot call the server loadout');
reset role;
update game.matches set allocation_id = 'alloc-f1', state = 'in_progress'
 where match_id = 'a0000000-0000-4000-8000-0000000000f1';
update game.items set retired = false where item_id = 'line.t_long';
insert into game.inventory (account_id, item_id, source) values
  ('f1111111-1111-4111-8111-111111111111', 'livery.t_contract', 'contract');
select is(api.srv_player_loadout('a0000000-0000-4000-8000-0000000000f1', 'alloc-f1',
                                 'f1111111-1111-4111-8111-111111111111') ->> 'line', 'line.t_long',
  'a change made in the lobby reaches the server at the reset');
select is((select loadout -> 'cosmetics' ->> 'livery' from game.match_players
            where match_id = 'a0000000-0000-4000-8000-0000000000f1'), 'livery.t_contract',
  'and is recorded on the roster');

select * from finish();
rollback;
