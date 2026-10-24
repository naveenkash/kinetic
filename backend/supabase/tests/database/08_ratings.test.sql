-- The Office Rating stays hidden: players read their band, never the number (game-modes 6.3).
begin;
create extension if not exists pgtap with schema extensions;
select plan(4);

insert into auth.users (id, email, aud, role, raw_app_meta_data) values
  ('a5111111-1111-4111-8111-111111111111', 'dev-jo@users.kinetic.invalid', 'authenticated', 'authenticated', '{"kinetic_provider":"dev","kinetic_provider_user_id":"jo"}'),
  ('a5222222-2222-4222-8222-222222222222', 'dev-kit@users.kinetic.invalid', 'authenticated', 'authenticated', '{"kinetic_provider":"dev","kinetic_provider_user_id":"kit"}');
select api.srv_ensure_account('a5111111-1111-4111-8111-111111111111', 'dev', 'jo', 'jo');
select api.srv_ensure_account('a5222222-2222-4222-8222-222222222222', 'dev', 'kit', 'kit');
insert into game.ratings (account_id, mode, mu, sigma, games, band) values
  ('a5111111-1111-4111-8111-111111111111', 'scrap', 31.2, 2.1, 40, 'Certified'),
  ('a5222222-2222-4222-8222-222222222222', 'scrap', 20.0, 3.0, 12, 'Inspected');

set local role authenticated;
set local request.jwt.claims = '{"sub":"a5111111-1111-4111-8111-111111111111","role":"authenticated"}';
select is((select band from api.my_ratings where mode = 'scrap'), 'Certified', 'a player reads their band');
select is((select count(*) from api.my_ratings), 1::bigint, 'and only their own');
select throws_ok($$ select mu from game.ratings $$, '42501', null, 'the number itself is not readable');
select throws_ok($$ update game.ratings set band = 'Championship' $$, '42501', null, 'nor writable');

select * from finish();
rollback;
