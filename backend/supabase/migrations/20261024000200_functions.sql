-- KINETIC backend, Phase 1 functions. See docs/tech/backend-architecture.md sections 9, 10, 11, 15.
-- Every client-callable function is security definer with an empty search_path, checks the
-- caller, rate-limits, writes, and broadcasts. Functions named srv_* are for Edge Functions
-- and the worker only (granted to service_role).

-- Helpers --------------------------------------------------------------------------

create function private.hit_rate_limit(p_key text, p_max integer, p_window interval)
returns boolean
language plpgsql security definer set search_path = ''
as $$
declare
  v_hits integer;
begin
  insert into private.rate_limits as r (key, window_start, hits)
  values (p_key, now(), 1)
  on conflict (key) do update
    set hits = case when r.window_start < now() - p_window then 1 else r.hits + 1 end,
        window_start = case when r.window_start < now() - p_window then now() else r.window_start end
  returning hits into v_hits;
  return v_hits <= p_max;
end
$$;

create function private.require_rate(p_key text, p_max integer, p_window interval)
returns void
language plpgsql security definer set search_path = ''
as $$
begin
  if not private.hit_rate_limit(p_key, p_max, p_window) then
    raise exception 'rate_limited' using hint = p_key;
  end if;
end
$$;

create function private.require_uid()
returns uuid
language plpgsql stable security definer set search_path = ''
as $$
declare
  v_uid uuid := auth.uid();
begin
  if v_uid is null then
    raise exception 'not_authenticated';
  end if;
  if not exists (select 1 from game.accounts where account_id = v_uid and status = 'active') then
    raise exception 'account_inactive';
  end if;
  if private.active_sanction(v_uid, array['account_ban']) then
    raise exception 'banned';
  end if;
  return v_uid;
end
$$;

-- Compares dotted build strings such as 0.1.1234. Returns -1, 0 or 1.
create function private.version_cmp(p_a text, p_b text)
returns integer
language plpgsql immutable set search_path = ''
as $$
declare
  v_a integer[];
  v_b integer[];
begin
  if p_a !~ '^\d{1,6}(\.\d{1,6}){0,3}$' or p_b !~ '^\d{1,6}(\.\d{1,6}){0,3}$' then
    raise exception 'bad_build';
  end if;
  v_a := string_to_array(p_a, '.')::integer[];
  v_b := string_to_array(p_b, '.')::integer[];
  return case when v_a < v_b then -1 when v_a > v_b then 1 else 0 end;
end
$$;

-- A player whose client has not sent a presence heartbeat for this long is offline. The
-- worker's cleanup pass uses the same value.
create function private.presence_timeout()
returns interval
language sql immutable set search_path = ''
as $$
  select interval '90 seconds'
$$;

create function private.config_text(p_key text)
returns text
language sql stable security definer set search_path = ''
as $$
  select value #>> '{}' from game.config where key = p_key
$$;

create function private.active_sanction(p_account uuid, p_types text[])
returns boolean
language sql stable security definer set search_path = ''
as $$
  select exists (
    select 1 from game.sanctions
    where account_id = p_account and type = any (p_types)
      and starts_at <= now() and (ends_at is null or ends_at > now())
  )
$$;

create function private.active_match_id(p_account uuid)
returns uuid
language sql stable security definer set search_path = ''
as $$
  select mp.match_id
  from game.match_players mp
  where mp.account_id = p_account and mp.live and not mp.abandoned
  limit 1
$$;

create function private.new_join_code()
returns text
language sql volatile set search_path = ''
as $$
  select upper(substr(replace(gen_random_uuid()::text, '-', ''), 1, 8))
$$;

-- The caller's crew-chat topic, including the rotating chat key (Realtime policies).
create function private.my_crew_chat_topic()
returns text
language sql stable security definer set search_path = ''
as $$
  select 'crew-chat:' || c.crew_id || ':' || c.chat_key
  from game.crews c
  join game.crew_members m using (crew_id)
  where m.account_id = (select auth.uid())
$$;

create function private.broadcast(p_topic text, p_event text, p_payload jsonb)
returns void
language plpgsql security definer set search_path = ''
as $$
begin
  perform realtime.send(p_payload, p_event, p_topic, true);
end
$$;

create function private.crew_state(p_crew uuid)
returns jsonb
language sql stable security definer set search_path = ''
as $$
  select jsonb_build_object(
    'crew_id', c.crew_id,
    'leader_id', c.leader_id,
    -- No join_code here: this state is broadcast, and a removed member's cached channel may
    -- still receive it. Members read the code from api.my_crew, checked on every query.
    'chat_topic', 'crew-chat:' || c.crew_id || ':' || c.chat_key,
    'queued', exists (select 1 from game.mm_tickets t where t.crew_id = c.crew_id),
    'members', coalesce((
      select jsonb_agg(jsonb_build_object(
               'account_id', m.account_id, 'display_name', a.display_name,
               'harness', m.harness, 'face', m.face, 'ready', m.ready)
             order by m.joined_at)
      from game.crew_members m
      join game.accounts a on a.account_id = m.account_id
      where m.crew_id = c.crew_id), '[]'::jsonb))
  from game.crews c
  where c.crew_id = p_crew
$$;

create function private.broadcast_crew(p_crew uuid)
returns void
language plpgsql security definer set search_path = ''
as $$
declare
  v_state jsonb := private.crew_state(p_crew);
begin
  if v_state is not null then
    perform private.broadcast('crew:' || p_crew::text, 'crew_state', v_state);
  end if;
end
$$;

-- Removes an account from its crew. Cancels the crew's ticket, promotes a new leader,
-- and deletes the crew when it empties.
create function private.leave_crew(p_account uuid)
returns void
language plpgsql security definer set search_path = ''
as $$
declare
  v_crew uuid;
  v_leader uuid;
begin
  select crew_id into v_crew from game.crew_members where account_id = p_account;
  if v_crew is null then
    return;
  end if;
  select leader_id into v_leader from game.crews where crew_id = v_crew for update;

  delete from game.mm_tickets where crew_id = v_crew;
  delete from game.crew_members where crew_id = v_crew and account_id = p_account;

  if not exists (select 1 from game.crew_members where crew_id = v_crew) then
    delete from game.crews where crew_id = v_crew;
  else
    if v_leader = p_account then
      update game.crews
         set leader_id = (select account_id from game.crew_members
                          where crew_id = v_crew order by joined_at limit 1)
       where crew_id = v_crew;
    end if;
    -- Remaining members are no longer ready, and the old join code stops working.
    update game.crew_members set ready = false where crew_id = v_crew and account_id <> (
      select leader_id from game.crews where crew_id = v_crew);
    update game.crews
       set join_code = private.new_join_code(), chat_key = replace(gen_random_uuid()::text, '-', '')
     where crew_id = v_crew;
    perform private.broadcast_crew(v_crew);
  end if;
  perform private.broadcast('user:' || p_account::text, 'crew_left', jsonb_build_object('crew_id', v_crew));
end
$$;

create function private.ensure_crew(p_account uuid)
returns uuid
language plpgsql security definer set search_path = ''
as $$
declare
  v_crew uuid;
begin
  select crew_id into v_crew from game.crew_members where account_id = p_account;
  if v_crew is not null then
    return v_crew;
  end if;
  insert into game.crews (leader_id) values (p_account) returning crew_id into v_crew;
  insert into game.crew_members (crew_id, account_id, ready) values (v_crew, p_account, true);
  return v_crew;
end
$$;

-- Locks the caller's crew row and returns it. Raises if the caller is not the leader
-- when p_need_leader is true.
create function private.lock_my_crew(p_account uuid, p_need_leader boolean)
returns uuid
language plpgsql security definer set search_path = ''
as $$
declare
  v_crew uuid := private.ensure_crew(p_account);
  v_leader uuid;
begin
  select leader_id into v_leader from game.crews where crew_id = v_crew for update;
  if p_need_leader and v_leader <> p_account then
    raise exception 'not_leader';
  end if;
  return v_crew;
end
$$;

create function private.join_crew(p_account uuid, p_crew uuid)
returns jsonb
language plpgsql security definer set search_path = ''
as $$
declare
  v_current uuid;
begin
  if exists (select 1 from game.crew_members where crew_id = p_crew and account_id = p_account) then
    return private.crew_state(p_crew);
  end if;
  if private.active_match_id(p_account) is not null then
    raise exception 'in_match';
  end if;
  select crew_id into v_current from game.crew_members where account_id = p_account;
  -- Lock the current and target crews in crew_id order, so two players swapping crews
  -- cannot deadlock.
  perform 1 from game.crews where crew_id in (p_crew, v_current) order by crew_id for update;
  if not exists (select 1 from game.crews where crew_id = p_crew) then
    raise exception 'crew_not_found';
  end if;
  if exists (select 1 from game.crew_members m
             where m.crew_id = p_crew and private.active_match_id(m.account_id) is not null) then
    raise exception 'crew_in_match';
  end if;
  if exists (select 1 from game.mm_tickets where crew_id = p_crew) then
    raise exception 'crew_queued';
  end if;
  if (select count(*) from game.crew_members where crew_id = p_crew) >= 3 then
    raise exception 'crew_full';
  end if;

  perform private.leave_crew(p_account);
  insert into game.crew_members (crew_id, account_id) values (p_crew, p_account);
  delete from game.crew_invites where crew_id = p_crew and to_id = p_account;
  perform private.broadcast_crew(p_crew);
  return private.crew_state(p_crew);
end
$$;

create function private.presence_state(p_account uuid)
returns text
language sql stable security definer set search_path = ''
as $$
  select case
    when private.active_match_id(p_account) is not null then 'in_match'
    when exists (select 1 from game.mm_tickets t
                 join game.crew_members m using (crew_id)
                 where m.account_id = p_account) then 'queuing'
    when (select count(*) from game.crew_members
          where crew_id = (select crew_id from game.crew_members where account_id = p_account)) > 1
      then 'crew'
    else 'menus'
  end
$$;

-- An account being deleted leaves its crew first: the next member leads, the crew's ticket
-- is cancelled, and the others are told.
create function private.leave_crew_on_delete()
returns trigger
language plpgsql security definer set search_path = ''
as $$
begin
  perform private.leave_crew(old.account_id);
  return old;
end
$$;
create trigger accounts_leave_crew
  before delete on game.accounts
  for each row execute function private.leave_crew_on_delete();

-- Client functions: config and presence -----------------------------------------------

create function api.get_config()
returns jsonb
language sql stable security definer set search_path = ''
as $$
  select jsonb_build_object(
    'min_build', private.config_text('min_build'),
    'latest_build', private.config_text('latest_build'),
    'motd', private.config_text('motd'),
    'regions', coalesce((select jsonb_agg(jsonb_build_object(
        'region_id', region_id, 'beacon_host', beacon_host, 'beacon_port', beacon_port)
        order by region_id)
      from game.regions where enabled), '[]'::jsonb),
    'playlists', coalesce((select jsonb_agg(jsonb_build_object(
        'playlist_id', playlist_id, 'mode', mode, 'team_size', team_size, 'teams', teams,
        'rated', rated, 'regions', regions)
        order by playlist_id)
      from game.playlists where enabled), '[]'::jsonb))
$$;

create function api.presence_heartbeat()
returns jsonb
language plpgsql security definer set search_path = ''
as $$
declare
  v_uid uuid := private.require_uid();
begin
  perform private.require_rate('presence:' || v_uid::text, 6, interval '1 minute');
  insert into game.presence (account_id, online, updated_at) values (v_uid, true, now())
  on conflict (account_id) do update set online = true, updated_at = now();
  return jsonb_build_object('state', private.presence_state(v_uid),
                            'match_id', private.active_match_id(v_uid));
end
$$;

create function api.friends_resolve(p_steam_ids text[])
returns table (steam_id text, account_id uuid, display_name text, state text)
language plpgsql security definer set search_path = ''
as $$
declare
  v_uid uuid := private.require_uid();
begin
  perform private.require_rate('friends:' || v_uid::text, 6, interval '1 minute');
  if coalesce(cardinality(p_steam_ids), 0) > 500 then
    raise exception 'too_many_ids';
  end if;
  return query
    select i.provider_user_id, a.account_id, a.display_name,
           case when p.online and p.updated_at > now() - private.presence_timeout()
                then private.presence_state(a.account_id) else 'offline' end
    from game.identities i
    join game.accounts a on a.account_id = i.account_id and a.status = 'active'
    left join game.presence p on p.account_id = a.account_id
    where i.provider = 'steam' and i.provider_user_id = any (p_steam_ids);
end
$$;

-- Client functions: crews ----------------------------------------------------------------

create function api.crew_create()
returns jsonb
language plpgsql security definer set search_path = ''
as $$
declare
  v_uid uuid := private.require_uid();
  v_crew uuid;
begin
  perform private.require_rate('crew:' || v_uid::text, 30, interval '1 minute');
  v_crew := private.ensure_crew(v_uid);
  perform private.broadcast_crew(v_crew);
  return private.crew_state(v_crew);
end
$$;

create function api.crew_invite(p_to uuid)
returns void
language plpgsql security definer set search_path = ''
as $$
declare
  v_uid uuid := private.require_uid();
  v_crew uuid;
  v_expires timestamptz := now() + interval '120 seconds';
begin
  perform private.require_rate('crew:' || v_uid::text, 30, interval '1 minute');
  if p_to is null or p_to = v_uid then
    raise exception 'invalid_target';
  end if;
  if not exists (select 1 from game.accounts where account_id = p_to and status = 'active') then
    raise exception 'unknown_account';
  end if;
  v_crew := private.lock_my_crew(v_uid, true);
  if exists (select 1 from game.mm_tickets where crew_id = v_crew) then
    raise exception 'crew_queued';
  end if;
  if (select count(*) from game.crew_members where crew_id = v_crew) >= 3 then
    raise exception 'crew_full';
  end if;
  if exists (select 1 from game.crew_members where crew_id = v_crew and account_id = p_to) then
    raise exception 'already_member';
  end if;

  insert into game.crew_invites (crew_id, from_id, to_id, expires_at)
  values (v_crew, v_uid, p_to, v_expires)
  on conflict (crew_id, to_id) do update set from_id = excluded.from_id, expires_at = excluded.expires_at;

  perform private.broadcast('user:' || p_to::text, 'crew_invite', jsonb_build_object(
    'crew_id', v_crew,
    'from_id', v_uid,
    'from_name', (select display_name from game.accounts where account_id = v_uid),
    'expires_at', v_expires));
end
$$;

create function api.crew_accept(p_crew uuid)
returns jsonb
language plpgsql security definer set search_path = ''
as $$
declare
  v_uid uuid := private.require_uid();
begin
  perform private.require_rate('crew:' || v_uid::text, 30, interval '1 minute');
  perform 1 from game.crew_invites
   where crew_id = p_crew and to_id = v_uid and expires_at > now();
  if not found then
    raise exception 'no_invite';
  end if;
  -- An invite is the crew taking the player back.
  delete from game.crew_kicks where crew_id = p_crew and account_id = v_uid;
  return private.join_crew(v_uid, p_crew);
end
$$;

create function api.crew_decline(p_crew uuid)
returns void
language plpgsql security definer set search_path = ''
as $$
declare
  v_uid uuid := private.require_uid();
begin
  perform private.require_rate('crew:' || v_uid::text, 30, interval '1 minute');
  delete from game.crew_invites where crew_id = p_crew and to_id = v_uid;
end
$$;

create function api.crew_join_by_code(p_code text)
returns jsonb
language plpgsql security definer set search_path = ''
as $$
declare
  v_uid uuid := private.require_uid();
  v_crew uuid;
begin
  perform private.require_rate('crew:' || v_uid::text, 30, interval '1 minute');
  select crew_id into v_crew from game.crews where join_code = upper(p_code);
  if v_crew is null then
    raise exception 'crew_not_found';
  end if;
  if exists (select 1 from game.crew_kicks
             where crew_id = v_crew and account_id = v_uid and until > now()) then
    raise exception 'kicked';
  end if;
  return private.join_crew(v_uid, v_crew);
end
$$;

create function api.crew_leave()
returns void
language plpgsql security definer set search_path = ''
as $$
declare
  v_uid uuid := private.require_uid();
begin
  perform private.require_rate('crew:' || v_uid::text, 30, interval '1 minute');
  perform private.leave_crew(v_uid);
end
$$;

create function api.crew_kick(p_account uuid)
returns void
language plpgsql security definer set search_path = ''
as $$
declare
  v_uid uuid := private.require_uid();
  v_crew uuid;
begin
  perform private.require_rate('crew:' || v_uid::text, 30, interval '1 minute');
  v_crew := private.lock_my_crew(v_uid, true);
  if p_account = v_uid
     or not exists (select 1 from game.crew_members where crew_id = v_crew and account_id = p_account) then
    raise exception 'invalid_target';
  end if;
  perform private.leave_crew(p_account);
  insert into game.crew_kicks (crew_id, account_id, until)
  values (v_crew, p_account, now() + interval '10 minutes')
  on conflict (crew_id, account_id) do update set until = excluded.until;
  perform private.broadcast('user:' || p_account::text, 'crew_kicked', jsonb_build_object('crew_id', v_crew));
end
$$;

create function api.crew_promote(p_account uuid)
returns void
language plpgsql security definer set search_path = ''
as $$
declare
  v_uid uuid := private.require_uid();
  v_crew uuid;
begin
  perform private.require_rate('crew:' || v_uid::text, 30, interval '1 minute');
  v_crew := private.lock_my_crew(v_uid, true);
  if p_account = v_uid
     or not exists (select 1 from game.crew_members where crew_id = v_crew and account_id = p_account) then
    raise exception 'invalid_target';
  end if;
  update game.crews set leader_id = p_account where crew_id = v_crew;
  perform private.broadcast_crew(v_crew);
end
$$;

create function api.crew_set_pick(p_harness text, p_face text default null, p_ready boolean default false)
returns jsonb
language plpgsql security definer set search_path = ''
as $$
declare
  v_uid uuid := private.require_uid();
  v_crew uuid;
begin
  perform private.require_rate('crew:' || v_uid::text, 30, interval '1 minute');
  if p_harness not in ('Climber', 'Scrapper', 'Rigger', 'Doubler', 'Breaker') then
    raise exception 'unknown_harness';
  end if;
  if p_face is not null and p_face not in (
      'baskoro-hanif', 'dela-santos', 'jesus-vilca', 'mei-lin-haldane', 'nnamdi-ferreira',
      'odile-lacroix', 'paloma-reyes-okafor', 'rook-adeyemi', 'tendai-mutasa', 'teodora-quispe') then
    raise exception 'unknown_face';
  end if;
  v_crew := private.lock_my_crew(v_uid, false);
  if exists (select 1 from game.mm_tickets where crew_id = v_crew) then
    raise exception 'crew_queued';
  end if;
  update game.crew_members
     set harness = p_harness, face = p_face, ready = coalesce(p_ready, false)
   where crew_id = v_crew and account_id = v_uid;
  perform private.broadcast_crew(v_crew);
  return private.crew_state(v_crew);
end
$$;

-- Client functions: queue ----------------------------------------------------------------

create function api.queue_join(p_playlist text, p_pings jsonb, p_build text)
returns jsonb
language plpgsql security definer set search_path = ''
as $$
declare
  v_uid uuid := private.require_uid();
  v_crew uuid;
  v_playlist game.playlists;
  v_count integer;
  v_max_grade integer;
  v_min_grade integer;
  v_pool text;
  v_pings jsonb;
  v_members jsonb;
  v_min_build text := private.config_text('min_build');
begin
  perform private.require_rate('queue:' || v_uid::text, 20, interval '1 minute');

  if v_min_build is not null and private.version_cmp(p_build, v_min_build) < 0 then
    raise exception 'build_too_old';
  end if;

  select * into v_playlist from game.playlists where playlist_id = p_playlist and enabled;
  if not found then
    raise exception 'unknown_playlist';
  end if;

  v_crew := private.lock_my_crew(v_uid, true);
  if exists (select 1 from game.mm_tickets where crew_id = v_crew) then
    raise exception 'already_queued';
  end if;

  select count(*), max(l.grade), min(l.grade)
    into v_count, v_max_grade, v_min_grade
    from game.crew_members m join game.ledgers l using (account_id)
   where m.crew_id = v_crew;

  if v_count > v_playlist.team_size and v_playlist.team_size > 1 then
    raise exception 'crew_too_large';
  end if;
  if exists (select 1 from game.crew_members
             where crew_id = v_crew and account_id <> v_uid and not ready) then
    raise exception 'crew_not_ready';
  end if;
  if exists (select 1 from game.crew_members m
             join game.accounts a on a.account_id = m.account_id
             where m.crew_id = v_crew and a.status <> 'active') then
    raise exception 'member_inactive';
  end if;
  if exists (select 1 from game.crew_members m
             where m.crew_id = v_crew
               and private.active_sanction(m.account_id,
                     array['matchmaking_ban', 'queue_lock', 'account_ban'])) then
    raise exception 'queue_locked';
  end if;
  if v_playlist.rated and exists (select 1 from game.crew_members m
             where m.crew_id = v_crew and private.active_sanction(m.account_id, array['rated_lock'])) then
    raise exception 'rated_locked';
  end if;
  if exists (select 1 from game.crew_members m
             where m.crew_id = v_crew and private.active_match_id(m.account_id) is not null) then
    raise exception 'in_match';
  end if;

  -- Keep only numeric pings for enabled regions in this playlist (networking section 9).
  select coalesce(jsonb_object_agg(r.region_id, (p_pings ->> r.region_id)::numeric), '{}'::jsonb)
    into v_pings
    from game.regions r
   where r.enabled
     and r.region_id = any (v_playlist.regions)
     and jsonb_typeof(p_pings -> r.region_id) = 'number'
     and (p_pings ->> r.region_id)::numeric between 0 and 1000;
  if not exists (select 1 from jsonb_each_text(v_pings) e where e.value::numeric <= 150) then
    raise exception 'no_region';
  end if;

  -- Grade pools, game-modes section 6.1: the crew queues in its highest member's pool.
  if v_playlist.rated then
    if v_min_grade < 2 then
      raise exception 'grade_too_low';
    end if;
    v_pool := 'rated';
  elsif v_max_grade <= 1 then
    if v_playlist.mode not in ('scrap', 'free_scrap') then
      raise exception 'grade_too_low';
    end if;
    v_pool := 'apprentice';
  elsif v_max_grade <= 3 then
    v_pool := 'hand';
  else
    v_pool := 'journeyman';
  end if;

  select jsonb_agg(jsonb_build_object(
           'account_id', m.account_id, 'harness', m.harness, 'face', m.face, 'grade', l.grade)
         order by m.joined_at)
    into v_members
    from game.crew_members m join game.ledgers l using (account_id)
   where m.crew_id = v_crew;

  insert into game.mm_tickets (crew_id, playlist_id, grade_pool, build, members, pings_ms)
  values (v_crew, p_playlist, v_pool, p_build, v_members, v_pings);

  perform private.broadcast('crew:' || v_crew::text, 'queue_state', jsonb_build_object(
    'queued', true, 'playlist_id', p_playlist, 'grade_pool', v_pool, 'since', now()));
  return jsonb_build_object('queued', true, 'grade_pool', v_pool, 'regions', v_pings);
end
$$;

create function api.queue_cancel()
returns void
language plpgsql security definer set search_path = ''
as $$
declare
  v_uid uuid := private.require_uid();
  v_crew uuid;
begin
  perform private.require_rate('queue:' || v_uid::text, 20, interval '1 minute');
  select crew_id into v_crew from game.crew_members where account_id = v_uid;
  if v_crew is null then
    return;
  end if;
  delete from game.mm_tickets where crew_id = v_crew;
  if found then
    perform private.broadcast('crew:' || v_crew::text, 'queue_state', jsonb_build_object('queued', false));
  end if;
end
$$;

-- Service functions (Edge Functions and worker) -------------------------------------------

create function api.srv_rate_limit(p_key text, p_max integer, p_window_s integer)
returns boolean
language sql security definer set search_path = ''
as $$
  select private.hit_rate_limit(p_key, p_max, make_interval(secs => p_window_s))
$$;

create function api.srv_build_ok(p_build text)
returns boolean
language plpgsql stable security definer set search_path = ''
as $$
declare
  v_min text := private.config_text('min_build');
begin
  if p_build is null or p_build !~ '^\d{1,6}(\.\d{1,6}){0,3}$' then
    return false;
  end if;
  return v_min is null or private.version_cmp(p_build, v_min) >= 0;
end
$$;

create function api.srv_find_account(p_provider text, p_provider_user_id text)
returns uuid
language sql stable security definer set search_path = ''
as $$
  select account_id from game.identities
  where provider = p_provider and provider_user_id = p_provider_user_id
$$;

-- Looks up an Auth user by address without creating one (generateLink would create it).
create function api.srv_find_auth_user(p_email text)
returns uuid
language sql stable security definer set search_path = ''
as $$
  -- Matches the partial unique index on email, so this is one index probe.
  select id from auth.users where email = lower(p_email) and is_sso_user = false
$$;

create function api.srv_ensure_account(p_user uuid, p_provider text, p_provider_user_id text,
                                       p_display_name text default null)
returns jsonb
language plpgsql security definer set search_path = ''
as $$
declare
  v_owner uuid;
  v_name text := p_display_name;
  v_meta jsonb;
  v_try integer := 0;
begin
  select account_id into v_owner from game.identities
   where provider = p_provider and provider_user_id = p_provider_user_id;
  if v_owner is not null and v_owner <> p_user then
    raise exception 'identity_conflict';
  end if;

  select raw_app_meta_data into v_meta from auth.users where id = p_user;
  if not found then
    raise exception 'unknown_user';
  end if;
  -- app_metadata is writable only through the admin API, so a user made by public signup
  -- (or for another identity) never carries these values. GoTrue gives admin-created users
  -- a random password hash, so the password column proves nothing either way.
  if v_meta ->> 'kinetic_provider' is distinct from p_provider
     or v_meta ->> 'kinetic_provider_user_id' is distinct from p_provider_user_id then
    raise exception 'identity_conflict';
  end if;

  if not exists (select 1 from game.accounts where account_id = p_user) then
    if v_name is null or v_name !~ '^[A-Za-z0-9_-]{3,24}$' then
      v_name := 'rigger_' || substr(md5(p_provider || ':' || p_provider_user_id), 1, 10);
    end if;
    loop
      begin
        -- A concurrent first login may insert the same account; that is not a name clash.
        insert into game.accounts (account_id, display_name) values (p_user, v_name)
        on conflict (account_id) do nothing;
        exit;
      exception when unique_violation then
        v_try := v_try + 1;
        if v_try > 5 then
          raise exception 'display_name_unavailable';
        end if;
        v_name := left(v_name, 17) || '_' || substr(replace(gen_random_uuid()::text, '-', ''), 1, 6);
      end;
    end loop;
  end if;
  insert into game.ledgers (account_id) values (p_user) on conflict do nothing;
  insert into game.identities (provider, provider_user_id, account_id)
  values (p_provider, p_provider_user_id, p_user) on conflict do nothing;

  return (
    select jsonb_build_object(
      'account_id', a.account_id, 'display_name', a.display_name, 'status', a.status,
      'grade', l.grade,
      'banned', private.active_sanction(a.account_id, array['account_ban']))
    from game.accounts a join game.ledgers l using (account_id)
    where a.account_id = p_user);
end
$$;

create function api.srv_current_match(p_account uuid)
returns jsonb
language sql stable security definer set search_path = ''
as $$
  select jsonb_build_object('match_id', m.match_id, 'address', m.address,
                            'team', mp.team, 'slot', mp.slot, 'state', m.state)
  from game.match_players mp
  join game.matches m using (match_id)
  join game.playlists p on p.playlist_id = m.playlist_id
  where mp.account_id = p_account
    and mp.live
    and not mp.abandoned
    and m.state in ('waiting_for_players', 'in_progress')
    and m.address is not null
    and (mp.left_at is null
         or mp.left_at > now() - make_interval(secs => case when p.rated then 90 else 60 end))
  order by m.created_at desc
  limit 1
$$;

create function api.srv_manifest(p_match uuid, p_allocation text)
returns jsonb
language plpgsql stable security definer set search_path = ''
as $$
declare
  v_match game.matches;
  v_playlist game.playlists;
begin
  select * into v_match from game.matches where match_id = p_match;
  if not found or v_match.allocation_id is distinct from p_allocation then
    raise exception 'unknown_match';
  end if;
  if v_match.state in ('completed', 'aborted') then
    raise exception 'match_closed';
  end if;
  select * into v_playlist from game.playlists where playlist_id = v_match.playlist_id;
  return jsonb_build_object(
    'match_id', v_match.match_id,
    'playlist_id', v_playlist.playlist_id,
    'mode', v_playlist.mode,
    'rated', v_playlist.rated,
    'arena', v_match.arena,
    'variant', v_match.variant,
    'region', v_match.region,
    'build', v_match.build,
    'team_size', v_playlist.team_size,
    'teams', v_playlist.teams,
    'reconnect_window_s', case when v_playlist.rated then 90 else 60 end,
    'event_schema', 1,
    'ruleset', 1,
    'players', (
      select jsonb_agg(jsonb_build_object(
               'slot', mp.slot, 'team', mp.team, 'account_id', mp.account_id,
               'is_bot', mp.is_bot,
               'display_name', coalesce(a.display_name, 'Office crew ' || mp.slot),
               'harness', mp.harness, 'face', mp.face, 'loadout', mp.loadout)
             order by mp.slot)
      from game.match_players mp
      left join game.accounts a on a.account_id = mp.account_id
      where mp.match_id = p_match));
end
$$;

create function api.srv_heartbeat(p_match uuid, p_allocation text, p_state text, p_connected uuid[])
returns jsonb
language plpgsql security definer set search_path = ''
as $$
declare
  v_match game.matches;
begin
  -- FOR NO KEY UPDATE does not block the key-share lock that inserting an abandon takes,
  -- so a heartbeat and a player event at the same moment cannot deadlock.
  select * into v_match from game.matches where match_id = p_match for no key update;
  if not found or v_match.allocation_id is distinct from p_allocation then
    raise exception 'unknown_match';
  end if;
  if v_match.state in ('completed', 'aborted', 'reporting') then
    return jsonb_build_object('state', v_match.state);
  end if;
  if p_state not in ('waiting_for_players', 'in_progress') then
    raise exception 'bad_state';
  end if;

  if v_match.state = 'allocating' then
    if v_match.address is null then
      update game.matches set last_heartbeat_at = now() where match_id = p_match;
      return jsonb_build_object('state', 'allocating');
    end if;
    update game.matches set state = 'waiting_for_players', ready_at = now() where match_id = p_match;
  end if;
  if p_state = 'in_progress' then
    update game.matches
       set state = 'in_progress', started_at = coalesce(started_at, now())
     where match_id = p_match;
  end if;
  update game.matches set last_heartbeat_at = now() where match_id = p_match;
  update game.match_players
     set connected = (account_id = any (coalesce(p_connected, '{}')))
   where match_id = p_match and not is_bot and account_id is not null;

  return jsonb_build_object('state', (select state from game.matches where match_id = p_match));
end
$$;

create function api.srv_player_event(p_match uuid, p_allocation text, p_account uuid, p_event text)
returns void
language plpgsql security definer set search_path = ''
as $$
begin
  perform 1 from game.matches
   where match_id = p_match and allocation_id = p_allocation
     and state in ('waiting_for_players', 'in_progress');
  if not found then
    raise exception 'unknown_match';
  end if;
  if not exists (select 1 from game.match_players where match_id = p_match and account_id = p_account) then
    raise exception 'unknown_player';
  end if;

  if p_event = 'joined' then
    update game.match_players set connected = true, joined_at = coalesce(joined_at, now()), left_at = null
     where match_id = p_match and account_id = p_account and not abandoned;
  elsif p_event = 'left' then
    update game.match_players set connected = false, left_at = now()
     where match_id = p_match and account_id = p_account;
  elsif p_event = 'abandoned' then
    update game.match_players set connected = false, abandoned = true, left_at = coalesce(left_at, now())
     where match_id = p_match and account_id = p_account;
    insert into game.abandons (account_id, match_id) values (p_account, p_match) on conflict do nothing;
  else
    raise exception 'bad_event';
  end if;
end
$$;

-- Readers for untrusted report JSON. A malformed field returns null, so the report is
-- quarantined instead of raising (a raise would make the server retry forever).
create function private.json_uuid(p_value jsonb)
returns uuid
language plpgsql immutable set search_path = ''
as $$
begin
  if jsonb_typeof(p_value) = 'string'
     and (p_value #>> '{}') ~* '^[0-9a-f]{8}-[0-9a-f]{4}-[0-9a-f]{4}-[0-9a-f]{4}-[0-9a-f]{12}$' then
    return (p_value #>> '{}')::uuid;
  end if;
  return null;
end
$$;

create function private.json_bool(p_value jsonb, p_default boolean)
returns boolean
language sql immutable set search_path = ''
as $$
  select case
    when p_value is null or jsonb_typeof(p_value) = 'null' then p_default
    when jsonb_typeof(p_value) = 'boolean' then (p_value #>> '{}')::boolean
    else null
  end
$$;

create function private.json_num(p_value jsonb)
returns numeric
language sql immutable set search_path = ''
as $$
  select case when jsonb_typeof(p_value) = 'number' then (p_value #>> '{}')::numeric end
$$;

-- Stores a match report of facts (section 12). Phase 1 stores without rewards. A report
-- that does not match the roster exactly is quarantined, never rewarded.
create function api.srv_store_report(p_match uuid, p_allocation text, p_payload jsonb)
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
         or (private.json_bool(e -> 'is_bot', false) = false and private.json_uuid(e -> 'account_id') is null)) then
    v_reason := 'malformed_player';
  elsif exists (
      select 1 from jsonb_array_elements(v_players) e
      where private.json_num(e -> 'connected_seconds') not between 0 and v_max_seconds) then
    v_reason := 'connected_seconds';
  elsif (select count(*) from jsonb_array_elements(v_players) e where not private.json_bool(e -> 'is_bot', false))
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

-- Grants ----------------------------------------------------------------------------------
-- Nothing is callable until granted below.
revoke execute on all functions in schema api, private from public, anon, authenticated;
grant execute on function private.my_crew_id(), private.my_crew_chat_topic() to authenticated;

grant execute on function api.get_config() to anon, authenticated;
grant execute on function
  api.presence_heartbeat(),
  api.friends_resolve(text[]),
  api.crew_create(),
  api.crew_invite(uuid),
  api.crew_accept(uuid),
  api.crew_decline(uuid),
  api.crew_join_by_code(text),
  api.crew_leave(),
  api.crew_kick(uuid),
  api.crew_promote(uuid),
  api.crew_set_pick(text, text, boolean),
  api.queue_join(text, jsonb, text),
  api.queue_cancel()
  to authenticated;
grant execute on function
  api.srv_rate_limit(text, integer, integer),
  api.srv_build_ok(text),
  api.srv_find_account(text, text),
  api.srv_find_auth_user(text),
  api.srv_ensure_account(uuid, text, text, text),
  api.srv_current_match(uuid),
  api.srv_manifest(uuid, text),
  api.srv_heartbeat(uuid, text, text, uuid[]),
  api.srv_player_event(uuid, text, uuid, text),
  api.srv_store_report(uuid, text, jsonb)
  to service_role;
