// The Office Rating (backend-architecture 12.3 step 7; game-modes 6.3 and 8): OpenSkill's
// Weng-Lin Bradley-Terry update for two or more teams, on Rated playlists only. Pure.
//
// Each player is (mu, sigma), starting at (25, 25/3). A team's skill is the sum of its
// players'. The hidden number is never shown; the Ledger shows a band from the conservative
// ordinal mu - 3 sigma (PROPOSED thresholds below; game-modes 6.3 names the six bands).
//
// Match rules (game-modes 8):
// - A Rated forfeit costs Rating as a loss plus 20%: the forfeiting crew's losses are scaled
//   by FORFEIT_LOSS_SCALE.
// - A leaver takes the full loss whatever their crew's result.
// - A no-contest changes nothing.

export const MU0 = 25;
export const SIGMA0 = 25 / 3;
const BETA = SIGMA0 / 2;
const KAPPA = 0.0001;
const FORFEIT_LOSS_SCALE = 1.2;
const CONSERVATIVE_SIGMAS = 3;
export const PROVISIONAL_GAMES = 10;

export interface Rating {
  mu: number;
  sigma: number;
}

export interface RatedPlayer {
  key: string;
  team: number;
  rating: Rating;
  abandoned: boolean;
}

export interface RatedMatch {
  // Team ranks: 1 is first. Equal ranks are a draw.
  rank: Record<number, number>;
  forfeit_team: number | null;
}

interface TeamSum {
  team: number;
  mu: number;
  sigmaSq: number;
}

function teamSums(players: RatedPlayer[]): TeamSum[] {
  const teams = [...new Set(players.map((p) => p.team))].sort((a, b) => a - b);
  return teams.map((team) => {
    const mine = players.filter((p) => p.team === team);
    return {
      team,
      mu: mine.reduce((s, p) => s + p.rating.mu, 0),
      sigmaSq: mine.reduce((s, p) => s + p.rating.sigma ** 2, 0),
    };
  });
}

// Omega (mean shift) and delta (variance shrink) for team `i` against every other team.
function teamUpdate(i: TeamSum, others: TeamSum[], rankOf: (team: number) => number) {
  let omega = 0;
  let delta = 0;
  for (const q of others) {
    const c = Math.sqrt(i.sigmaSq + q.sigmaSq + 2 * BETA ** 2);
    const pIQ = 1 / (1 + Math.exp((q.mu - i.mu) / c));
    const ri = rankOf(i.team);
    const rq = rankOf(q.team);
    const s = ri < rq ? 1 : ri === rq ? 0.5 : 0;
    const gamma = Math.sqrt(i.sigmaSq) / c;
    omega += (i.sigmaSq / c) * (s - pIQ);
    delta += gamma * (i.sigmaSq / c ** 2) * pIQ * (1 - pIQ);
  }
  return { omega, delta };
}

function apply(rating: Rating, team: TeamSum, omega: number, delta: number): Rating {
  const share = rating.sigma ** 2 / team.sigmaSq;
  const sigmaSq = rating.sigma ** 2 * Math.max(1 - share * delta, KAPPA);
  return { mu: rating.mu + share * omega, sigma: Math.sqrt(sigmaSq) };
}

// New ratings, keyed by player. Teams absent from `rank` are treated as last.
export function rate(players: RatedPlayer[], match: RatedMatch): Map<string, Rating> {
  const teams = teamSums(players);
  const last = Math.max(0, ...Object.values(match.rank)) + 1;
  const rankOf = (team: number) => match.rank[team] ?? last;
  const lostTo = (team: number) => (t: number) => (t === team ? last + 1 : rankOf(t));
  const result = new Map<string, Rating>();
  for (const p of players) {
    const team = teams.find((t) => t.team === p.team)!;
    const others = teams.filter((t) => t.team !== p.team);
    // A leaver is rated as if their crew lost, whatever happened after they left.
    const ranks = p.abandoned ? lostTo(p.team) : rankOf;
    const { omega, delta } = teamUpdate(team, others, ranks);
    const scale = match.forfeit_team === p.team && omega < 0 ? FORFEIT_LOSS_SCALE : 1;
    result.set(p.key, apply(p.rating, team, omega * scale, delta));
  }
  return result;
}

export const BANDS = ["Provisional", "Inspected", "Certified", "Rated", "Sanctioned", "Championship"] as const;
export type Band = typeof BANDS[number];
// Ordinal thresholds for Inspected and up (PROPOSED).
const BAND_FLOORS = [-Infinity, 5, 15, 25, 35];

export function ordinal(r: Rating): number {
  return r.mu - CONSERVATIVE_SIGMAS * r.sigma;
}

export function band(r: Rating, games: number): Band {
  if (games < PROVISIONAL_GAMES) return "Provisional";
  const o = ordinal(r);
  const index = BAND_FLOORS.filter((floor) => o >= floor).length;
  return BANDS[index];
}
