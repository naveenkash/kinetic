-- Steam purchases (13.4) and moderation (17): only the service role moves money or closes
-- reports; players see their own purchases, report players they played with, and change their
-- name through the filter.
begin;
create extension if not exists pgtap with schema extensions;
select plan(24);

insert into auth.users (id, email, aud, role, raw_app_meta_data) values
  ('ac111111-1111-4111-8111-111111111111', 'dev-mo@users.kinetic.invalid', 'authenticated', 'authenticated', '{"kinetic_provider":"dev","kinetic_provider_user_id":"mo"}'),
  ('ac222222-2222-4222-8222-222222222222', 'dev-ned@users.kinetic.invalid', 'authenticated', 'authenticated', '{"kinetic_provider":"dev","kinetic_provider_user_id":"ned"}'),
  ('ac333333-3333-4333-8333-333333333333', 'dev-oz@users.kinetic.invalid', 'authenticated', 'authenticated', '{"kinetic_provider":"dev","kinetic_provider_user_id":"oz"}');
select api.srv_ensure_account('ac111111-1111-4111-8111-111111111111', 'dev', 'mo', 'mo_rigger');
select api.srv_ensure_account('ac222222-2222-4222-8222-222222222222', 'dev', 'ned', 'ned_rigger');
select api.srv_ensure_account('ac333333-3333-4333-8333-333333333333', 'dev', 'oz', 'oz_rigger');
insert into game.skus (sku, kind, steam_item_id, description, prices, credit)
values ('t.credit', 'sponsor_credit', 4242, 'Credit', '{"USD": 499}', 500);
insert into game.purchases (account_id, sku, kind, credit, amount, currency_code) values
  ('ac111111-1111-4111-8111-111111111111', 't.credit', 'sponsor_credit', 500, 499, 'USD'),
  ('ac222222-2222-4222-8222-222222222222', 't.credit', 'sponsor_credit', 500, 499, 'USD');
insert into game.matches (match_id, playlist_id, arena, region, build, fleet, state)
values ('ad000000-0000-4000-8000-000000000001', 'scrap_3v3_casual', 'YRD', 'local', '0.1.0', 'local', 'completed');
insert into game.match_players (match_id, slot, team, account_id, harness) values
  ('ad000000-0000-4000-8000-000000000001', 1, 1, 'ac111111-1111-4111-8111-111111111111', 'Rigger'),
  ('ad000000-0000-4000-8000-000000000001', 2, 2, 'ac222222-2222-4222-8222-222222222222', 'Rigger');

select throws_ok($$ select api.srv_purchase_create('ac222222-2222-4222-8222-222222222222', 't.credit', 'GBP') $$,
  'P0001', 'currency_unsupported', 'a price is needed in the wallet currency');
select throws_ok($$ select api.srv_purchase_create('ac222222-2222-4222-8222-222222222222', 'nope', 'USD') $$,
  'P0001', 'unknown_sku', 'unknown SKUs are refused');
select throws_ok($$ insert into game.skus (sku, kind, steam_item_id, description, prices)
                    values ('t.bad', 'sponsor_credit', 4243, 'Bad', '{}') $$, '23514', null,
  'a credit SKU names its credit');

set local role authenticated;
set local request.jwt.claims = '{"sub":"ac111111-1111-4111-8111-111111111111","role":"authenticated"}';
select throws_ok($$ select api.srv_purchase_finalize(1, 'x') $$, '42501', null, 'players cannot finalize purchases');
select is((select count(*) from api.my_purchases), 1::bigint, 'a player sees their own purchase, not another''s');
select is((select count(*) from api.store where sku = 't.credit'), 1::bigint, 'and the store');

-- Reports.
select is(api.report_player('ad000000-0000-4000-8000-000000000001', 'ac222222-2222-4222-8222-222222222222', 'griefing',
                            'hung me off the rail every round') ->> 'status', 'received', 'a player reports a player');
select is(api.report_player('ad000000-0000-4000-8000-000000000001', 'ac222222-2222-4222-8222-222222222222', 'griefing')
          ->> 'status', 'received', 'reporting twice is accepted but recorded once');
select throws_ok($$ select api.report_player('ad000000-0000-4000-8000-000000000001',
                                              'ac333333-3333-4333-8333-333333333333', 'abuse') $$,
  'P0001', 'not_in_match', 'only someone from the same match');
select throws_ok($$ select api.report_player('ad000000-0000-4000-8000-000000000001',
                                              'ac222222-2222-4222-8222-222222222222', 'lag') $$,
  'P0001', 'bad_reason', 'with a listed reason');
select throws_ok($$ select * from game.reports $$, '42501', null, 'reports are not readable by players');

-- Names.
select throws_ok($$ select api.change_display_name('Sh1t_Hauler') $$, 'P0001', 'name_refused',
  'the filter sees through stand-ins');
select throws_ok($$ select api.change_display_name('the.office') $$, 'P0001', 'name_refused',
  'and through the character rules');
select throws_ok($$ select api.change_display_name('ned_rigger') $$, 'P0001', 'display_name_unavailable',
  'a taken name is refused');
reset role;
select is((select array_agg(n) from unnest(array['Hit1er', 'Hltler', 'Nlgger', 'N199er', 'Fvck', 'Fuuuck', 'Kunt',
                                                  'Phaggot', 'F_ck', 'Fck_you', 'Fuuck', 'Shiit', 'Cuunt', 'Kuunt',
                                                  'Sluut', 'Fcck', 'Niggger', 'Nigggger', 'Fagggot', 'Faggggot',
                                                  'ScrapeRape', 'Niigger', 'Niggeer', 'Niiggaa', 'Faaggot', 'Faggoot',
                                                  'Fagggoot', 'Staaff', 'Offiicial', 'Offiice', 'NaziReich', 'Nazi_Rule',
                                                  'NaziRiot', 'SlutSky', 'Slut_Sky', 'RapeSeed', 'ShitAke', 'OfficeRep',
                                                  'Office_Rat', 'ScraperApe']) n
            where private.name_allowed(n)), null,
  'stand-ins, look-alikes and repeats are folded');
select is((select array_agg(n order by n) from unnest(array['Scunthorpe', 'Grape_Hauler', 'Shitake', 'Officer_K',
                                                             'Rapeseed', 'Niger', 'Nigeria', 'Fukuda', 'Scrape_King',
                                                             'Therapeutic', 'Nazir', 'Slutsky', 'Nazir_Khan']) n
            where private.name_allowed(n)),
  array['Fukuda', 'Grape_Hauler', 'Nazir', 'Nazir_Khan', 'Niger', 'Nigeria', 'Officer_K', 'Rapeseed', 'Scrape_King',
        'Scunthorpe', 'Shitake', 'Slutsky', 'Therapeutic'], 'real names and allowed words pass');
select is(private.name_allowed('Grape_Rape'), false, 'an allowed word does not hide a blocked one');
set local role authenticated;
set local request.jwt.claims = '{"sub":"ac111111-1111-4111-8111-111111111111","role":"authenticated"}';
select throws_ok($$ select api.change_display_name('NED_RIGGER') $$, 'P0001', 'display_name_unavailable',
  'names are unique whatever their case');
select is(api.change_display_name('Spine_Walker') ->> 'display_name', 'Spine_Walker', 'a clean name is taken');
select throws_ok($$ select api.change_display_name('Spine_Runner') $$, 'P0001', 'name_change_too_soon',
  'once every 30 days');

reset role;
select is((select count(*) from game.reports), 1::bigint, 'one report on the queue');
select api.srv_reset_display_name('ac222222-2222-4222-8222-222222222222');
select ok((select name_changed_at > now() - interval '1 minute' from game.accounts
            where account_id = 'ac222222-2222-4222-8222-222222222222'), 'a reset starts the 30 days again');
delete from auth.users where id = 'ac222222-2222-4222-8222-222222222222';
select is((select count(*) from game.reports where reported_id is null), 1::bigint,
  'a report outlives the reported player''s deletion');
insert into game.sanctions (account_id, type, reason) values ('ac333333-3333-4333-8333-333333333333', 'account_ban', 't');
select throws_ok($$ select api.srv_purchase_create('ac333333-3333-4333-8333-333333333333', 't.credit', 'USD') $$,
  'P0001', 'account_inactive', 'a banned account cannot buy');

select * from finish();
rollback;
