-- Local and dev seed. Production reference data is managed from the admin console.

insert into game.config (key, value) values
  ('min_build', '"0.1.0"'),
  ('latest_build', '"0.1.0"'),
  ('motd', '"Office notice: local build. Data is not kept."')
on conflict (key) do update set value = excluded.value;

insert into game.regions (region_id, beacon_host, beacon_port, enabled) values
  ('local', '127.0.0.1', 7000, true),
  ('na-east', 'beacon-na-east.invalid', 7000, true),
  ('eu-west', 'beacon-eu-west.invalid', 7000, true)
on conflict (region_id) do nothing;

insert into game.playlists
  (playlist_id, mode, team_size, teams, rated, arenas, regions, bot_fill_pools, bot_fill_after_s)
values
  ('scrap_3v3_casual', 'scrap', 3, 2, false, '{YRD}', '{local,na-east,eu-west}', '{apprentice}', 60),
  ('free_scrap_8', 'free_scrap', 1, 8, false, '{YRD}', '{local,na-east,eu-west}', '{}', 60)
on conflict (playlist_id) do nothing;
