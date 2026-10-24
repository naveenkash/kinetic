-- Phase 3, moderation (backend-architecture sections 7.6 and 17): player reports with the
-- match's replay attached, and display-name changes through a name filter. Staff work the
-- report queue from the admin console. Outcomes are not published (menus 13): a reporter sees
-- only that their report was received.

create table game.reports (
  report_id uuid primary key default gen_random_uuid(),
  reporter_id uuid references game.accounts (account_id) on delete set null,
  -- Kept, anonymized, when the reported player's account is deleted: the evidence stays.
  reported_id uuid references game.accounts (account_id) on delete set null,
  match_id uuid not null references game.matches (match_id) on delete cascade,
  reason text not null check (reason in ('cheating', 'griefing', 'abuse', 'name', 'afk', 'other')),
  text text check (length(text) <= 500),
  replay_path text,
  status text not null default 'open' check (status in ('open', 'actioned', 'dismissed')),
  handled_by text,
  created_at timestamptz not null default now(),
  handled_at timestamptz
);
-- One report per reporter, player and match.
create unique index reports_once on game.reports (reporter_id, reported_id, match_id);
create index reports_open_idx on game.reports (created_at) where status = 'open';

-- Words a display name may not contain, compared after folding (PROPOSED starter lists; the
-- full lists are managed from the admin console). Allowed words are removed from a name before
-- the check, so Scunthorpe and Grape pass.
create table game.blocked_words (
  word text primary key check (word ~ '^[a-z]{3,32}$')
);
create table game.allowed_words (
  word text primary key check (word ~ '^[a-z]{3,32}$')
);
insert into game.allowed_words (word) values
  ('scunthorpe'), ('grape'), ('grapes'), ('drape'), ('rapeseed'), ('shitake'), ('shiitake'), ('officer'),
  ('scrape'), ('scraper'), ('scraping'), ('scrapes'), ('therapeutic'), ('nazir'), ('slutsky');

-- Names are unique whatever their case, so Ned_Rigger cannot pass for ned_rigger. Names that
-- already differ only in case must be resolved first (srv_reset_display_name), or this fails.
do $$
begin
  if exists (select lower(display_name) from game.accounts group by 1 having count(*) > 1) then
    raise exception 'display names differing only in case exist; reset them before this migration';
  end if;
end
$$;
create unique index accounts_display_name_lower on game.accounts (lower(display_name));
insert into game.blocked_words (word) values
  ('admin'), ('moderator'), ('office'), ('marshal'), ('official'), ('staff'),
  ('fuck'), ('fck'), ('shit'), ('cunt'), ('kunt'), ('nigger'), ('nigga'), ('faggot'), ('retard'), ('whore'),
  ('slut'), ('nazi'), ('hitler'), ('rape');

alter table game.reports enable row level security;
alter table game.blocked_words enable row level security;
alter table game.allowed_words enable row level security;
revoke all on game.reports, game.blocked_words, game.allowed_words from public, anon, authenticated;

create trigger reports_audit after update on game.reports
  for each row execute function private.audit_change('staff', 'report_id');
create trigger blocked_words_audit after insert or delete on game.blocked_words
  for each row execute function private.audit_change('staff', 'word');
create trigger allowed_words_audit after insert or delete on game.allowed_words
  for each row execute function private.audit_change('staff', 'word');
-- Name resets and other staff edits to accounts.
create trigger accounts_audit after update on game.accounts
  for each row execute function private.audit_change('staff', 'account_id');

-- Folds a name, or a listed word, for the filter: lower case; ph to f; digit and symbol
-- stand-ins to letters (0 o, 1 | l i, 3 e, 4 @ a, 5 $ s, 6 9 g, 7 t); v to u; separators
-- dropped.
create function private.fold_base(p_name text)
returns text
language sql immutable
set search_path = ''
as $$
  select regexp_replace(translate(replace(lower(p_name), 'ph', 'f'), '01l|3456@$79v', 'oiiieasgastgu'), '[^a-z]', '', 'g')
$$;

-- A listed word as a pattern over folded names: each run of k letters must appear at least k
-- times (nigger is n+i+g{2,}e+r+), so repeating any letter does not hide the word, and Niger
-- (one g) is not nigger.
create function private.word_pattern(p_word text)
returns text
language sql immutable
set search_path = ''
as $$
  select string_agg(case when length(m[1]) = 1 then substr(m[1], 1, 1) || '+'
                         else substr(m[1], 1, 1) || '{' || length(m[1]) || ',}' end, '' order by n)
    from regexp_matches(private.fold_base(p_word), '(([a-z])\2*)', 'g') with ordinality as r(m, n)
$$;

-- True if p_pattern matches p_text anywhere not wholly inside one of the allowed spans.
create function private.has_uncovered(p_text text, p_pattern text, p_allowed int4range[])
returns boolean
language plpgsql immutable
set search_path = ''
as $$
declare
  v_from integer := 1;
  v_at integer;
  v_end integer;
begin
  if p_pattern is null or length(p_pattern) = 0 then
    return false;
  end if;
  loop
    v_at := regexp_instr(p_text, p_pattern, v_from);
    exit when v_at = 0;
    v_end := v_at + length(regexp_substr(p_text, p_pattern, v_at));
    if not exists (select 1 from unnest(coalesce(p_allowed, '{}')) r where r @> int4range(v_at, v_end)) then
      return true;
    end if;
    v_from := v_at + 1;
  end loop;
  return false;
end
$$;

-- A name passes unless a blocked word's pattern matches its folded text somewhere not wholly
-- inside an allowed token. Tokens are the name's parts split at _ and - and at a lower-to-upper
-- case change; an allowed word counts only as a whole token, so Nazir_Khan, Scrape_King and
-- Scunthorpe pass but NaziReich, Office_Rat and ScraperApe do not.
create function private.name_allowed(p_name text)
returns boolean
language plpgsql stable security definer set search_path = ''
as $$
declare
  v_folded text := '';
  v_spans int4range[] := '{}';
  v_token text;
  v_part text;
begin
  if p_name !~ '^[A-Za-z0-9_-]{3,24}$' then
    return false;
  end if;
  for v_token in
    select t from regexp_split_to_table(regexp_replace(p_name, '([a-z])([A-Z])', '\1_\2', 'g'), '[_-]+')
                  with ordinality as x(t, n) order by n
  loop
    v_part := private.fold_base(v_token);
    continue when v_part = '';
    if exists (select 1 from game.allowed_words w where v_part ~ ('^' || private.word_pattern(w.word) || '$')) then
      v_spans := v_spans || int4range(length(v_folded) + 1, length(v_folded) + 1 + length(v_part));
    end if;
    v_folded := v_folded || v_part;
  end loop;
  return not exists (select 1 from game.blocked_words w
                      where private.has_uncovered(v_folded, private.word_pattern(w.word), v_spans));
end
$$;

-- Client functions --------------------------------------------------------------------

-- Reports a player from a match both took part in. The match's replay is attached.
create function api.report_player(p_match uuid, p_reported uuid, p_reason text, p_text text default null)
returns jsonb
language plpgsql security definer set search_path = ''
as $$
declare
  v_uid uuid := private.require_uid();
begin
  perform private.require_rate('report:' || v_uid::text, 5, interval '1 hour');
  if p_reported is null or p_reported = v_uid then
    raise exception 'bad_target';
  end if;
  if p_reason is null or p_reason not in ('cheating', 'griefing', 'abuse', 'name', 'afk', 'other') then
    raise exception 'bad_reason';
  end if;
  if p_text is not null and length(p_text) > 500 then
    raise exception 'text_too_long';
  end if;
  if not exists (select 1 from game.match_players where match_id = p_match and account_id = v_uid)
     or not exists (select 1 from game.match_players where match_id = p_match and account_id = p_reported) then
    raise exception 'not_in_match';
  end if;
  insert into game.reports (reporter_id, reported_id, match_id, reason, text, replay_path)
  values (v_uid, p_reported, p_match, p_reason, nullif(btrim(p_text), ''),
          (select replay_path from game.match_reports where match_id = p_match))
  on conflict (reporter_id, reported_id, match_id) do nothing;
  return jsonb_build_object('status', 'received');
end
$$;

-- A new display name, once every 30 days (PROPOSED), through the name filter.
create function api.change_display_name(p_name text)
returns jsonb
language plpgsql security definer set search_path = ''
as $$
declare
  v_uid uuid := private.require_uid();
  v_last timestamptz;
begin
  perform private.require_rate('name:' || v_uid::text, 5, interval '1 hour');
  if p_name is null or not private.name_allowed(p_name) then
    raise exception 'name_refused';
  end if;
  select name_changed_at into v_last from game.accounts where account_id = v_uid for update;
  if v_last > now() - interval '30 days' then
    raise exception 'name_change_too_soon';
  end if;
  begin
    update game.accounts set display_name = p_name, name_changed_at = now() where account_id = v_uid;
  exception when unique_violation then
    raise exception 'display_name_unavailable';
  end;
  return jsonb_build_object('display_name', p_name);
end
$$;

-- Service functions (admin console) -------------------------------------------------------

-- Closes a report. Staff action on the player (a sanction) is a separate console step.
create function api.srv_report_handle(p_report uuid, p_status text, p_staff text)
returns jsonb
language plpgsql security definer set search_path = ''
as $$
begin
  if p_status not in ('actioned', 'dismissed') then
    raise exception 'bad_status';
  end if;
  update game.reports set status = p_status, handled_by = p_staff, handled_at = now()
   where report_id = p_report and status = 'open';
  if not found then
    raise exception 'not_open';
  end if;
  return jsonb_build_object('report_id', p_report, 'status', p_status);
end
$$;

-- Resets a display name that slipped past the filter (a word added to the list since) to a
-- neutral one; the 30 days before the next change start again.
create function api.srv_reset_display_name(p_account uuid)
returns jsonb
language plpgsql security definer set search_path = ''
as $$
declare
  v_name text := 'rigger_' || substr(replace(gen_random_uuid()::text, '-', ''), 1, 10);
begin
  update game.accounts set display_name = v_name, name_changed_at = now() where account_id = p_account;
  if not found then
    raise exception 'unknown_account';
  end if;
  return jsonb_build_object('display_name', v_name);
end
$$;

revoke execute on function
  private.fold_base(text), private.word_pattern(text), private.has_uncovered(text, text, int4range[]),
  private.name_allowed(text), api.report_player(uuid, uuid, text, text),
  api.change_display_name(text), api.srv_report_handle(uuid, text, text), api.srv_reset_display_name(uuid)
  from public, anon, authenticated;
grant execute on function api.report_player(uuid, uuid, text, text), api.change_display_name(text) to authenticated;
grant execute on function api.srv_report_handle(uuid, text, text), api.srv_reset_display_name(uuid) to service_role;
