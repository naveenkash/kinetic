-- Phase 3, export, deletion and retention (backend-architecture section 19).
-- - api.request_export queues a job; the worker writes a JSON archive of the player's data to
--   the private `exports` bucket under their own folder, which only they can read.
-- - api.request_deletion starts a 14-day grace (the account stays usable, so the player can
--   cancel); then the worker deletes the Auth user, identities, display name and personal
--   rows. Match rows are anonymized by the foreign keys (account ids set to null), so other
--   players' history stays whole; purchases are kept, anonymized, for tax records.
-- - The worker's retention step removes replays after 30 days and raw match events after 90,
--   except those attached to an open player report or a quarantined match report, and export
--   archives 7 days after they were written.
-- - Sanctions outlive a deletion: they are kept against a hash of the provider identity and
--   applied again if that identity ever signs up anew, so deleting an account is no way out
--   of a ban.

create table game.account_jobs (
  job_id uuid primary key default gen_random_uuid(),
  account_id uuid references game.accounts (account_id) on delete set null,
  kind text not null check (kind in ('export', 'deletion')),
  status text not null default 'queued'
    check (status in ('queued', 'running', 'done', 'cancelled', 'failed', 'expired')),
  requested_at timestamptz not null default now(),
  run_after timestamptz not null default now(),
  -- A job left running (the worker died) is taken again after a while; failures retry with
  -- backoff, then the job is failed for a person to look at.
  started_at timestamptz,
  attempts integer not null default 0,
  completed_at timestamptz,
  result_path text,
  error text
);
create index account_jobs_due_idx on game.account_jobs (run_after) where status = 'queued';
-- One pending job of each kind per account.
create unique index account_jobs_one_pending on game.account_jobs (account_id, kind) where status in ('queued', 'running');

alter table game.account_jobs enable row level security;
create policy account_jobs_own on game.account_jobs
  for select to authenticated using (account_id = (select auth.uid()));
grant select on game.account_jobs to authenticated;

create view api.my_account_jobs with (security_invoker = true) as
  select j.job_id, j.kind, j.status, j.requested_at, j.run_after, j.completed_at, j.result_path
    from game.account_jobs j;
grant select on api.my_account_jobs to authenticated;

insert into storage.buckets (id, name, public) values ('exports', 'exports', false) on conflict (id) do nothing;

-- A player reads their own export folder; the worker writes it with the service role.
create policy exports_owner_read on storage.objects
  for select to authenticated
  using (bucket_id = 'exports' and (storage.foldername(name))[1] = (select auth.uid())::text);

create function api.request_export()
returns jsonb
language plpgsql security definer set search_path = ''
as $$
declare
  v_uid uuid := private.require_uid();
  v_job uuid;
begin
  perform private.require_rate('export:' || v_uid::text, 1, interval '1 day');
  insert into game.account_jobs (account_id, kind) values (v_uid, 'export')
  on conflict do nothing
  returning job_id into v_job;
  if v_job is null then
    raise exception 'export_pending';
  end if;
  return jsonb_build_object('job_id', v_job, 'status', 'queued');
end
$$;

create function api.request_deletion()
returns jsonb
language plpgsql security definer set search_path = ''
as $$
declare
  v_uid uuid := private.require_uid();
  v_job game.account_jobs;
begin
  perform private.require_rate('deletion:' || v_uid::text, 1, interval '1 day');
  insert into game.account_jobs (account_id, kind, run_after) values (v_uid, 'deletion', now() + interval '14 days')
  on conflict do nothing
  returning * into v_job;
  if v_job.job_id is null then
    select * into v_job from game.account_jobs
     where account_id = v_uid and kind = 'deletion' and status in ('queued', 'running');
  end if;
  return jsonb_build_object('job_id', v_job.job_id, 'deletes_at', v_job.run_after);
end
$$;

create function api.cancel_deletion()
returns jsonb
language plpgsql security definer set search_path = ''
as $$
declare
  v_uid uuid := private.require_uid();
begin
  update game.account_jobs set status = 'cancelled', completed_at = now()
   where account_id = v_uid and kind = 'deletion' and status in ('queued', 'running');
  if not found then
    raise exception 'no_pending_deletion';
  end if;
  return jsonb_build_object('status', 'cancelled');
end
$$;

-- Everything the backend holds about one player, for the export (section 19).
create function private.account_archive(p_account uuid)
returns jsonb
language sql stable security definer set search_path = ''
as $$
  select jsonb_build_object(
    'exported_at', now(),
    'account', (select to_jsonb(a) from game.accounts a where a.account_id = p_account),
    'identities', (select coalesce(jsonb_agg(jsonb_build_object('provider', i.provider, 'provider_user_id',
                                                                i.provider_user_id, 'created_at', i.created_at)), '[]')
                     from game.identities i where i.account_id = p_account),
    'ledger', (select to_jsonb(l) from game.ledgers l where l.account_id = p_account),
    'ledger_entries', (select coalesce(jsonb_agg(to_jsonb(e)), '[]') from game.ledger_entries e where e.account_id = p_account),
    'rope_marks', (select coalesce(jsonb_agg(to_jsonb(r)), '[]') from game.rope_marks r where r.account_id = p_account),
    'career_stats', (select coalesce(jsonb_agg(to_jsonb(c)), '[]') from game.career_stats c where c.account_id = p_account),
    'match_summaries', (select coalesce(jsonb_agg(to_jsonb(s) order by s.created_at), '[]')
                          from game.match_summaries s where s.account_id = p_account),
    'inventory', (select coalesce(jsonb_agg(to_jsonb(v)), '[]') from game.inventory v where v.account_id = p_account),
    'loadouts', (select coalesce(jsonb_agg(to_jsonb(o)), '[]') from game.loadouts o where o.account_id = p_account),
    'wallets', (select coalesce(jsonb_agg(to_jsonb(w)), '[]') from game.wallets w where w.account_id = p_account),
    'wallet_txns', (select coalesce(jsonb_agg(to_jsonb(t) order by t.created_at), '[]')
                      from game.wallet_txns t where t.account_id = p_account),
    'purchases', (select coalesce(jsonb_agg(to_jsonb(p) - 'account_id' order by p.created_at), '[]')
                    from game.purchases p where p.account_id = p_account),
    'entitlements', (select coalesce(jsonb_agg(to_jsonb(e)), '[]') from game.entitlements e where e.account_id = p_account),
    'allegiances', (select coalesce(jsonb_agg(to_jsonb(a)), '[]') from game.allegiances a where a.account_id = p_account),
    'allegiance_history', (select coalesce(jsonb_agg(to_jsonb(h)), '[]')
                             from game.allegiance_history h where h.account_id = p_account),
    'manifest_progress', (select coalesce(jsonb_agg(to_jsonb(m)), '[]')
                            from game.manifest_progress m where m.account_id = p_account),
    'abandons', (select coalesce(jsonb_agg(to_jsonb(b)), '[]') from game.abandons b where b.account_id = p_account),
    'crew', (select coalesce(jsonb_agg(to_jsonb(c) - 'account_id'), '[]') from game.crew_members c where c.account_id = p_account),
    'crew_invites', (select coalesce(jsonb_agg(jsonb_build_object('crew_id', i.crew_id, 'expires_at', i.expires_at)), '[]')
                       from game.crew_invites i where i.to_id = p_account or i.from_id = p_account),
    'crew_kicks', (select coalesce(jsonb_agg(to_jsonb(k)), '[]') from game.crew_kicks k where k.account_id = p_account),
    'presence', (select to_jsonb(p) from game.presence p where p.account_id = p_account),
    'support_flags', (select coalesce(jsonb_agg(jsonb_build_object('reason', f.reason, 'created_at', f.created_at,
                                                                   'resolved_at', f.resolved_at)), '[]')
                        from game.support_flags f where f.account_id = p_account),
    'account_jobs', (select coalesce(jsonb_agg(to_jsonb(j) - 'account_id'), '[]')
                       from game.account_jobs j where j.account_id = p_account),
    'contract_progress', (select coalesce(jsonb_agg(to_jsonb(c)), '[]')
                            from game.contract_progress c where c.account_id = p_account),
    'standing_events', (select coalesce(jsonb_agg(to_jsonb(e) - 'account_id'), '[]')
                          from game.claim_standing_events e where e.account_id = p_account),
    'manifest', (select coalesce(jsonb_agg(to_jsonb(m)), '[]') from game.manifest_paid m where m.account_id = p_account),
    'ratings', (select coalesce(jsonb_agg(jsonb_build_object('mode', r.mode, 'band', r.band, 'games', r.games)), '[]')
                  from game.ratings r where r.account_id = p_account),
    'matches', (select coalesce(jsonb_agg(jsonb_build_object('match_id', mp.match_id, 'team', mp.team,
                                                             'harness', mp.harness, 'joined_at', mp.joined_at)), '[]')
                  from game.match_players mp where mp.account_id = p_account),
    'reports_filed', (select coalesce(jsonb_agg(jsonb_build_object('match_id', r.match_id, 'reason', r.reason,
                                                                   'text', r.text, 'created_at', r.created_at)), '[]')
                        from game.reports r where r.reporter_id = p_account),
    'sanctions', (select coalesce(jsonb_agg(jsonb_build_object('type', s.type, 'starts_at', s.starts_at,
                                                               'ends_at', s.ends_at)), '[]')
                    from game.sanctions s where s.account_id = p_account))
$$;

-- Sanctions kept past deletion, against a keyed hash (HMAC-SHA-256) of each provider identity.
-- SteamIDs are guessable, so a plain hash could be reversed; the key is a server secret made
-- here and never leaves the database.
create table private.secrets (
  name text primary key,
  value text not null
);
alter table private.secrets enable row level security;
revoke all on private.secrets from public, anon, authenticated;
insert into private.secrets (name, value)
values ('identity_hmac_key', encode(extensions.gen_random_bytes(32), 'hex'))
on conflict (name) do nothing;

create table game.retained_sanctions (
  identity_hash text not null,
  type text not null,
  reason text not null,
  ends_at timestamptz,
  retained_at timestamptz not null default now()
);
create index retained_sanctions_hash_idx on game.retained_sanctions (identity_hash);
alter table game.retained_sanctions enable row level security;
revoke all on game.retained_sanctions from public, anon, authenticated;

create function private.identity_hash(p_provider text, p_provider_user_id text)
returns text
language sql stable security definer set search_path = ''
as $$
  select encode(extensions.hmac(p_provider || ':' || p_provider_user_id,
                                (select value from private.secrets where name = 'identity_hmac_key'), 'sha256'), 'hex')
$$;

-- A new identity that was banned before its old account was deleted gets the ban back.
create function private.reapply_retained_sanctions()
returns trigger
language plpgsql security definer set search_path = ''
as $$
begin
  insert into game.sanctions (account_id, type, reason, ends_at, issued_by)
  select new.account_id, r.type, 'retained after deletion: ' || r.reason, r.ends_at, 'system'
    from game.retained_sanctions r
   where r.identity_hash = private.identity_hash(new.provider, new.provider_user_id)
     and (r.ends_at is null or r.ends_at > now());
  return new;
end
$$;
create trigger identities_reapply_sanctions
  after insert on game.identities
  for each row execute function private.reapply_retained_sanctions();

-- Deletes an account in one transaction (section 19): sanctions retained by identity hash,
-- the player's id taken out of stored match report payloads, the Auth audit trail for the user
-- removed, then the Auth user deleted, which cascades to the account and its personal rows and
-- sets the id to null on match rows, purchases, reports and Standing events.
create function private.delete_account(p_account uuid)
returns void
language plpgsql security definer set search_path = ''
as $$
begin
  insert into game.retained_sanctions (identity_hash, type, reason, ends_at)
  select private.identity_hash(i.provider, i.provider_user_id), s.type, s.reason, s.ends_at
    from game.identities i join game.sanctions s on s.account_id = i.account_id
   where i.account_id = p_account and (s.ends_at is null or s.ends_at > now());
  update game.match_reports r
     set payload = jsonb_set(r.payload, '{players}', (
           select coalesce(jsonb_agg(case when e ->> 'account_id' = p_account::text
                                          then e || '{"account_id": null}'::jsonb else e end), '[]')
             from jsonb_array_elements(r.payload -> 'players') e))
   where jsonb_typeof(r.payload -> 'players') = 'array'
     and r.match_id in (select match_id from game.match_players where account_id = p_account);
  -- CHECK on the hosted project: auth.audit_log_entries belongs to the Auth service's role. If
  -- this role may not delete there, the deletion still goes ahead and support is flagged to
  -- remove the entries by hand.
  begin
    delete from auth.audit_log_entries
     where payload ->> 'actor_id' = p_account::text or payload -> 'traits' ->> 'user_id' = p_account::text;
  exception when insufficient_privilege then
    insert into game.support_flags (reason, ref) values ('auth_audit_not_removed', 'user:' || p_account);
  end;
  update game.accounts set status = 'deleted' where account_id = p_account;
  delete from auth.users where id = p_account;
end
$$;

revoke execute on function api.request_export(), api.request_deletion(), api.cancel_deletion(),
  private.account_archive(uuid), private.identity_hash(text, text), private.reapply_retained_sanctions(),
  private.delete_account(uuid) from public, anon, authenticated;
grant execute on function api.request_export(), api.request_deletion(), api.cancel_deletion() to authenticated;
