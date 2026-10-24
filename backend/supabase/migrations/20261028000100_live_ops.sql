-- Phase 4 (backend-architecture 26): live-ops and console identities.
--
-- - api.get_config also returns the feature flags, the arena variant flags and the current
--   Claim (section 18 lists them; Phase 1 shipped without).
-- - Playlist, region and identity changes made by staff are audited, as config already is.
--   Identity changes, and a player's own link, are audited by platform and account only: the
--   audit is append-only and outlives a deleted account, so it never holds a platform id.
-- - A console identity (PSN, Xbox, Nintendo) can be linked to an existing account, so a
--   player keeps one Ledger across platforms; a linked identity signs in to that account.

create or replace function api.get_config()
returns jsonb
language sql stable security definer set search_path = ''
as $$
  select jsonb_build_object(
    'min_build', private.config_text('min_build'),
    'latest_build', private.config_text('latest_build'),
    'motd', private.config_text('motd'),
    'flags', coalesce((select value from game.config where key = 'flags' and jsonb_typeof(value) = 'object'),
                      '{}'::jsonb),
    'variants', coalesce((select jsonb_object_agg(substr(key, 9), value) from game.config where key like 'variant:%'),
                         '{}'::jsonb),
    'claim', (select jsonb_build_object('claim_id', c.claim_id, 'name', c.name, 'arena', c.arena, 'state', c.state,
                                        'starts_at', c.starts_at, 'ends_at', c.ends_at)
                from game.claims c where c.state = 'active' limit 1),
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

create trigger playlists_audit after insert or update or delete on game.playlists
  for each row execute function private.audit_change('staff', 'playlist_id');
create trigger regions_audit after insert or update or delete on game.regions
  for each row execute function private.audit_change('staff', 'region_id');
-- Staff changes to identities are audited by platform and account: the audit is append-only
-- and outlives a deleted account, so it never holds a platform id.
create function private.audit_identity()
returns trigger
language plpgsql security definer set search_path = ''
as $$
declare
  v_staff text := private.current_staff();
  v_row game.identities := coalesce(new, old);
begin
  if v_staff is null then
    return v_row;
  end if;
  if not exists (select 1 from admin.staff where staff_id = v_staff) then
    raise exception 'unknown_staff' using hint = v_staff;
  end if;
  insert into admin.audit (staff_id, action, target, before, after)
  values (v_staff, lower(tg_op), 'identities:' || v_row.provider || ':' || v_row.account_id,
          case when tg_op <> 'INSERT' then jsonb_build_object('account_id', old.account_id) end,
          case when tg_op <> 'DELETE' then jsonb_build_object('account_id', new.account_id) end);
  return v_row;
end
$$;
revoke execute on function private.audit_identity() from public, anon, authenticated;
create trigger identities_audit after insert or update or delete on game.identities
  for each row execute function private.audit_identity();

-- One identity per platform per account.
create unique index identities_account_provider on game.identities (account_id, provider);

-- Sign-in for an identity. Unchanged from Phase 1, except that an identity already linked to
-- this account signs in to it (a console identity linked to a Steam account has no Auth user of
-- its own, so the creating-function check on app_metadata does not apply to it).
create or replace function api.srv_ensure_account(p_user uuid, p_provider text, p_provider_user_id text,
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
  -- a random password hash, so the password column proves nothing either way. A linked
  -- identity was proved when it was linked.
  if v_owner is null
     and (v_meta ->> 'kinetic_provider' is distinct from p_provider
          or v_meta ->> 'kinetic_provider_user_id' is distinct from p_provider_user_id) then
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

-- Links a proved console identity to the signed-in account (auth-console, link mode, which
-- also re-proves an identity the account already has). The account must be active and not
-- banned, the identity free, and the account without one on that platform already.
create function api.srv_link_identity(p_account uuid, p_provider text, p_provider_user_id text)
returns jsonb
language plpgsql security definer set search_path = ''
as $$
declare
  v_owner uuid;
begin
  if p_provider not in ('psn', 'xbox', 'nintendo') then
    raise exception 'bad_provider';
  end if;
  if not exists (select 1 from game.accounts where account_id = p_account and status = 'active') then
    raise exception 'unknown_account';
  end if;
  if private.active_sanction(p_account, array['account_ban']) then
    raise exception 'banned';
  end if;
  select account_id into v_owner from game.identities
   where provider = p_provider and provider_user_id = p_provider_user_id;
  if v_owner = p_account then
    return jsonb_build_object('linked', true, 'provider', p_provider, 'already', true);
  end if;
  if v_owner is not null then
    raise exception 'identity_conflict';
  end if;
  if exists (select 1 from game.identities where account_id = p_account and provider = p_provider) then
    raise exception 'provider_already_linked';
  end if;
  insert into game.identities (provider, provider_user_id, account_id)
  values (p_provider, p_provider_user_id, p_account);
  insert into admin.audit (staff_id, action, target, after)
  -- 'player:' cannot be a staff name (admin.staff allows no colon).
  values ('player:', 'link', 'identities:' || p_provider || ':' || p_account, jsonb_build_object('account_id', p_account));
  return jsonb_build_object('linked', true, 'provider', p_provider, 'already', false);
end
$$;

-- Staff only (the console): removes a linked console identity, never an account's last one.
-- Only console platforms unlink: the identity an account was made for is proved by its Auth
-- user's app_metadata, so its next sign-in would simply attach it again.
create function api.srv_unlink_identity(p_account uuid, p_provider text)
returns jsonb
language plpgsql security definer set search_path = ''
as $$
begin
  if p_provider not in ('psn', 'xbox', 'nintendo') then
    raise exception 'bad_provider';
  end if;
  if (select count(*) from game.identities where account_id = p_account) <= 1 then
    raise exception 'last_identity';
  end if;
  delete from game.identities where account_id = p_account and provider = p_provider;
  if not found then
    raise exception 'not_linked';
  end if;
  return jsonb_build_object('unlinked', true, 'provider', p_provider);
end
$$;

revoke execute on function api.srv_link_identity(uuid, text, text), api.srv_unlink_identity(uuid, text)
  from public, anon, authenticated;
grant execute on function api.srv_link_identity(uuid, text, text), api.srv_unlink_identity(uuid, text)
  to service_role;
