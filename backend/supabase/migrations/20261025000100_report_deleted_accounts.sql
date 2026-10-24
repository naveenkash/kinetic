-- Match reports may carry humans whose account was deleted before the server fetched its
-- manifest: the manifest gives their account_id as null, so the server can only report null.
-- srv_store_report quarantined every such report as malformed_player. A null account_id on a
-- human is now accepted and matched, per team, against the roster's null-account human slots
-- (the existing deleted-account rule); duplicates are counted over non-null ids only. Grants
-- are unchanged (CREATE OR REPLACE keeps them).

-- Stores a match report of facts (section 12). Phase 1 stores without rewards. A report
-- that does not match the roster exactly is quarantined, never rewarded.
create or replace function api.srv_store_report(p_match uuid, p_allocation text, p_payload jsonb)
returns jsonb
language plpgsql security definer set search_path = ''
as $$
declare
  v_match game.matches;
  v_teams integer;
  v_reason text;
  v_max_seconds numeric;
  v_status text;
  v_players jsonb := case when jsonb_typeof(p_payload -> 'players') = 'array'
                          then p_payload -> 'players' else '[]'::jsonb end;
begin
  select * into v_match from game.matches where match_id = p_match for no key update;
  if not found or v_match.allocation_id is distinct from p_allocation then
    raise exception 'unknown_match';
  end if;
  if exists (select 1 from game.match_reports where match_id = p_match) then
    return jsonb_build_object('status', 'duplicate');
  end if;
  if v_match.state = 'completed' then
    raise exception 'match_closed';
  end if;

  select teams into v_teams from game.playlists where playlist_id = v_match.playlist_id;
  v_max_seconds := extract(epoch from now() - coalesce(v_match.started_at, v_match.ready_at, v_match.created_at)) + 30;

  if jsonb_typeof(p_payload -> 'players') is distinct from 'array' then
    v_reason := 'players_missing';
  elsif (p_payload ->> 'match_id') is distinct from p_match::text then
    v_reason := 'match_id_mismatch';
  elsif exists (
      select 1 from jsonb_array_elements(v_players) e
      where jsonb_typeof(e) <> 'object'
         or private.json_bool(e -> 'is_bot', false) is null
         or private.json_bool(e -> 'abandoned', false) is null
         or private.json_num(e -> 'connected_seconds') is null
         or private.json_num(e -> 'team') is null
         -- A human's account_id is a UUID, or null for an account deleted before the manifest
         -- was fetched (the manifest gives null); null humans are matched per team below.
         or (private.json_bool(e -> 'is_bot', false) = false and private.json_uuid(e -> 'account_id') is null
             and coalesce(jsonb_typeof(e -> 'account_id'), 'null') <> 'null')) then
    v_reason := 'malformed_player';
  elsif exists (
      select 1 from jsonb_array_elements(v_players) e
      where private.json_num(e -> 'connected_seconds') not between 0 and v_max_seconds) then
    v_reason := 'connected_seconds';
  elsif (select count(private.json_uuid(e -> 'account_id')) from jsonb_array_elements(v_players) e
         where not private.json_bool(e -> 'is_bot', false))
        <> (select count(distinct private.json_uuid(e -> 'account_id')) from jsonb_array_elements(v_players) e
            where not private.json_bool(e -> 'is_bot', false)) then
    v_reason := 'duplicate_player';
  elsif exists (
      -- Every human on the roster is reported, on their team.
      select account_id, team::numeric from game.match_players
       where match_id = p_match and not is_bot and account_id is not null
      except
      select private.json_uuid(e -> 'account_id'), private.json_num(e -> 'team')
        from jsonb_array_elements(v_players) e where not private.json_bool(e -> 'is_bot', false))
    or exists (
      -- Reported humans not on the roster can only be slots whose account was deleted since:
      -- no such account may exist, and there must be the same number per team.
      select 1
        from jsonb_array_elements(v_players) e
        join game.accounts a on a.account_id = private.json_uuid(e -> 'account_id')
       where not private.json_bool(e -> 'is_bot', false)
         and not exists (select 1 from game.match_players mp
                         where mp.match_id = p_match and mp.account_id = a.account_id))
    or exists (
      with extra as (
        select private.json_num(e -> 'team') as team
          from jsonb_array_elements(v_players) e
         where not private.json_bool(e -> 'is_bot', false)
           and not exists (select 1 from game.match_players mp
                           where mp.match_id = p_match and not mp.is_bot
                             and mp.account_id = private.json_uuid(e -> 'account_id')))
      (select team, count(*) from extra group by team
       except
       select team::numeric, count(*) from game.match_players
        where match_id = p_match and not is_bot and account_id is null group by team)
      union all
      (select team::numeric, count(*) from game.match_players
        where match_id = p_match and not is_bot and account_id is null group by team
       except
       select team, count(*) from extra group by team)) then
    v_reason := 'roster_mismatch';
  elsif exists (
      (select private.json_num(e -> 'team'), count(*)
         from jsonb_array_elements(v_players) e where private.json_bool(e -> 'is_bot', false) group by 1
       except
       select team::numeric, count(*) from game.match_players where match_id = p_match and is_bot group by 1)
      union all
      (select team::numeric, count(*) from game.match_players where match_id = p_match and is_bot group by 1
       except
       select private.json_num(e -> 'team'), count(*)
         from jsonb_array_elements(v_players) e where private.json_bool(e -> 'is_bot', false) group by 1)) then
    v_reason := 'bot_mismatch';
  elsif (p_payload -> 'winner_team') is not null
        and jsonb_typeof(p_payload -> 'winner_team') <> 'null'
        and (private.json_num(p_payload -> 'winner_team') is null
             or private.json_num(p_payload -> 'winner_team') not between 1 and v_teams) then
    v_reason := 'winner_team';
  elsif v_match.state = 'aborted' then
    -- Kept for audit (for example a heartbeat_lost abort over a network blip), never rewarded.
    v_reason := 'match_aborted';
  elsif v_match.state = 'allocating' then
    v_reason := 'match_not_started';
  end if;

  v_status := case when v_reason is null then 'stored' else 'quarantined' end;
  insert into game.match_reports (match_id, payload, status, quarantine_reason, raw_events_path, replay_path)
  values (p_match, p_payload, v_status, v_reason,
          'match-events/' || p_match::text || '.jsonl.gz',
          'replays/' || p_match::text || '.replay');

  -- Abandons the server reports for players on this roster are recorded either way.
  insert into game.abandons (account_id, match_id)
  select mp.account_id, p_match
    from jsonb_array_elements(v_players) e
    join game.match_players mp
      on mp.match_id = p_match and mp.account_id = private.json_uuid(e -> 'account_id')
   where private.json_bool(e -> 'abandoned', false)
  on conflict do nothing;

  -- An aborted match stays aborted, and a match that never became ready is left for the
  -- worker's boot timeout, which requeues or tells its crews.
  if v_match.state not in ('aborted', 'allocating') then
    update game.matches set state = 'completed', ended_at = now() where match_id = p_match;
  end if;
  return jsonb_build_object('status', v_status, 'reason', v_reason);
end
$$;
