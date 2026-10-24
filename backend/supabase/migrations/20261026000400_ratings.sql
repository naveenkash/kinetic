-- Phase 2, the Office Rating (backend-architecture 12.3 step 7; game-modes 6.3): an OpenSkill
-- rating per account and mode, updated by the rewards pass for Rated playlists. The number is
-- hidden; players read only their band.

create table game.ratings (
  account_id uuid not null references game.accounts (account_id) on delete cascade,
  mode text not null check (mode in ('scrap', 'claim')),
  mu double precision not null,
  sigma double precision not null check (sigma > 0),
  games integer not null default 0 check (games >= 0),
  band text not null check (band in ('Provisional', 'Inspected', 'Certified', 'Rated', 'Sanctioned', 'Championship')),
  updated_at timestamptz not null default now(),
  primary key (account_id, mode)
);

-- Only Scrap and Claim are Rated (game-modes 6.3): a rated playlist in another mode is refused.
alter table game.playlists add constraint playlists_rated_modes check (not rated or mode in ('scrap', 'claim'));

alter table game.ratings enable row level security;
create policy ratings_own on game.ratings for select to authenticated using (account_id = (select auth.uid()));
-- Column grants keep mu and sigma out of reach even of the owner.
grant select (account_id, mode, games, band, updated_at) on game.ratings to authenticated;

create view api.my_ratings with (security_invoker = true) as
  select r.mode, r.band, r.games, r.updated_at from game.ratings r;
grant select on api.my_ratings to authenticated;

-- The match summary records the band before and after, never the number.
alter table game.match_summaries add column rating jsonb;
create or replace view api.my_match_summaries with (security_invoker = true) as
  select s.match_id, s.ruleset, s.win, s.tonnage, s.credit, s.breakdown, s.grade_before, s.grade_after,
         s.weight_before, s.weight_after, s.rope_marks, s.unlocks, s.contracts, s.created_at, s.standing, s.rating
  from game.match_summaries s;
