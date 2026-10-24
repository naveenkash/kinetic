#include "Kinetic/HangSolver.h"

#include <algorithm>
#include <cmath>
#include <limits>

namespace Kinetic
{
namespace
{
constexpr double SlipTolerance = 1e-6;         // m per substep: less is solver residue, not a slip
constexpr double LengthEpsilon = 1e-9;         // m
constexpr double CapTolerance = 1e-9;          // Relative: an impulse this close to its cap is at the cap
constexpr double StretchTolerance = 1e-7;      // m: a position pass that corrects less has converged
constexpr double MaxCorrectionPerStep = 0.05;  // m per line per step: drift is removed gradually, never teleported
constexpr double MaxSweep = 0.05;              // Largest turn per substep, in radians of line
constexpr int MaxSubsteps = 32;
constexpr int TargetRefreshes = 3;             // A line's target depends on its tangential speed
constexpr double LineRegularization = 1e-6;    // Relative, on the diagonal: parallel lines split evenly,
constexpr double FloorRegularization = 1e-10;  // and a floor parallel to a line carries the load first
// A winch slows over the last stretch before a bound, so the load is not jerked into it: at
// 1 m/s² a load up to the Hang load / (g + 1) stops there without slipping (PROPOSED).
constexpr double ReelBrake = 1.0;              // m/s²

struct FLineWork
{
	FVec3 Offset;              // B − A at the start of the substep
	FVec3 Axis;                // Unit, A to B
	double Length = 0.0;
	double SumInvMass = 0.0;
	double Target = 0.0;       // Rest-length rate this substep: the Reel rate, clamped to the bounds
	double RefLength = 0.0;    // The length the rest length is measured against
	double TargetLength = 0.0; // The length the line allows at the end of the substep, m
	double RestBefore = 0.0;
	double Impulse = 0.0;      // N·s this substep, positive = tension
	double Cap = 0.0;          // N·s: the Hang load over the substep
	bool bActive = false;      // A rope that would be taut this substep, or a rod
	bool bImmovable = false;   // Taut, but neither end can move (a kinematic dragging a braced player)
};

// One constraint row: c(v) = Σ Dir · v over at most two bodies. Its impulse λ changes each
// body's velocity by −Dir · λ / m, so a positive λ reduces c.
struct FRow
{
	int Bodies[2] = {-1, -1};
	FVec3 Dirs[2];
	double Low = 0.0;
	double High = 0.0;
	int Line = -1;       // The line it belongs to, or −1 for a floor contact
	double Floor = 0.0;  // Contacts: the floor height
};

double RestOf(const FLineState& S) { return S.L0 + S.SlipAccum + S.ReelAccum; }

double LowBound(const FLineWork& W, const FSpoolGlobals& G) { return std::min(G.DogsMeet, W.RestBefore); }
double HighBound(const FLineWork& W, const FHangLine& Line) { return std::max(Line.Params.LineLength, W.RestBefore); }

FVec3 RelativeVelocity(const std::vector<FHangBody>& Bodies, const FHangLine& Line)
{
	return Bodies[Line.BodyB].Velocity - Bodies[Line.BodyA].Velocity;
}

// The axial speed that brings the line to TargetLength at the end of the substep, given its
// tangential speed: |Offset + v·h| = TargetLength exactly (tangential motion lengthens a line,
// so a swing needs an inward axial speed to stay on its circle).
double DriveSpeed(const FLineWork& W, const FVec3& Relative, double H)
{
	const FVec3 Tangential = Relative - W.Axis * Relative.Dot(W.Axis);
	const double Sweep = Tangential.Length() * H;
	const double Along = std::sqrt(std::max(W.TargetLength * W.TargetLength - Sweep * Sweep, 0.0));
	return (Along - W.Length) / H;
}

// How fast the rest length would have to move for the line to stay just taut this substep.
double LengthRate(const std::vector<FHangBody>& Bodies, const FHangLine& Line, const FLineWork& W, double H)
{
	return ((W.Offset + RelativeVelocity(Bodies, Line) * H).Length() - W.RefLength) / H;
}

FLineWork Prepare(const std::vector<FHangBody>& Bodies, const FHangLine& Line, const FSpoolGlobals& G, double H)
{
	const FLineState& S = *Line.State;
	const FHangBody& A = Bodies[Line.BodyA];
	const FHangBody& B = Bodies[Line.BodyB];
	FLineWork W;
	W.Offset = B.Position - A.Position;
	W.Length = W.Offset.Length();
	W.Axis = W.Offset.SafeNormal();
	W.SumInvMass = A.InvMass + B.InvMass;
	W.RestBefore = RestOf(S);
	W.Cap = Line.HangLoadN * H;
	if (S.Mode != ELineMode::Hang && S.Mode != ELineMode::Set)
	{
		return W;  // Let go earlier in this step
	}
	// A winch cannot gain on ends that cannot move, and slows to a stop at the bounds.
	double Reel = (S.Mode == ELineMode::Hang && W.SumInvMass > 0.0) ? S.ReelInput * G.ReelRate : 0.0;
	const double ToBound = Reel > 0.0 ? HighBound(W, Line) - W.RestBefore : W.RestBefore - LowBound(W, G);
	Reel = std::copysign(std::min(std::abs(Reel), std::sqrt(2.0 * ReelBrake * std::max(ToBound, 0.0))), Reel);
	const double RestNext = std::clamp(W.RestBefore + Reel * H, LowBound(W, G), HighBound(W, Line));
	W.Target = (RestNext - W.RestBefore) / H;
	// Drift from the rest length is left to the position pass, not turned into speed. A slack
	// rope may close its gap this substep before it takes load.
	W.TargetLength = S.bRodMode ? W.Length + W.Target * H : std::max(RestNext, W.Length + W.Target * H);
	W.RefLength = W.TargetLength - W.Target * H;
	const FVec3 Relative = B.Velocity - A.Velocity;
	const bool bTaut = W.Length > LengthEpsilon &&
	                   (S.bRodMode || (W.Offset + Relative * H).Length() >= RestNext - LengthEpsilon);
	W.bActive = bTaut && W.SumInvMass > 0.0;
	W.bImmovable = bTaut && W.SumInvMass <= 0.0;
	return W;
}

double RowSpeed(const std::vector<FHangBody>& Bodies, const FRow& Row)
{
	double Speed = 0.0;
	for (int K = 0; K < 2; ++K)
	{
		if (Row.Bodies[K] >= 0)
		{
			Speed += Row.Dirs[K].Dot(Bodies[Row.Bodies[K]].Velocity);
		}
	}
	return Speed;
}

void ApplyRow(std::vector<FHangBody>& Bodies, const FRow& Row, double Impulse)
{
	for (int K = 0; K < 2; ++K)
	{
		if (Row.Bodies[K] >= 0)
		{
			FHangBody& Body = Bodies[Row.Bodies[K]];
			Body.Velocity = Body.Velocity - Row.Dirs[K] * (Impulse * Body.InvMass);
		}
	}
}

// Solves Matrix · x = Rhs for the rows in Free (Gaussian elimination with partial pivoting).
std::vector<double> SolveFree(const std::vector<std::vector<double>>& Matrix, const std::vector<double>& Rhs,
                              const std::vector<size_t>& Free)
{
	const size_t N = Free.size();
	std::vector<std::vector<double>> M(N, std::vector<double>(N + 1));
	for (size_t I = 0; I < N; ++I)
	{
		for (size_t J = 0; J < N; ++J)
		{
			M[I][J] = Matrix[Free[I]][Free[J]];
		}
		M[I][N] = Rhs[I];
	}
	for (size_t Col = 0; Col < N; ++Col)
	{
		size_t Pivot = Col;
		for (size_t R = Col + 1; R < N; ++R)
		{
			Pivot = std::abs(M[R][Col]) > std::abs(M[Pivot][Col]) ? R : Pivot;
		}
		std::swap(M[Col], M[Pivot]);
		if (std::abs(M[Col][Col]) < std::numeric_limits<double>::min())
		{
			continue;
		}
		for (size_t R = 0; R < N; ++R)
		{
			if (R != Col)
			{
				const double F = M[R][Col] / M[Col][Col];
				for (size_t C = Col; C <= N; ++C)
				{
					M[R][C] -= F * M[Col][C];
				}
			}
		}
	}
	std::vector<double> X(N, 0.0);
	for (size_t I = 0; I < N; ++I)
	{
		X[I] = std::abs(M[I][I]) < std::numeric_limits<double>::min() ? 0.0 : M[I][N] / M[I][I];
	}
	return X;
}

enum class EHeld : uint8_t { Free, AtLow, AtHigh };

// Which rows were held at a bound last time, kept across target refreshes and substeps: a
// solve that starts there usually needs no pivot at all.
struct FHeldGuess
{
	std::vector<EHeld> Lines;     // By line
	std::vector<EHeld> Contacts;  // By body
};

// Box-bounded solve of Matrix · λ = Rhs: each row is free (solved exactly) or held at a
// bound. Each iteration changes only the lowest-numbered row that is wrong: a free row out of
// its bounds is held, a held row that would pull back inside is freed. This least-index rule
// (Murty's) terminates for the regularized J M⁻¹ Jᵀ, which is a P-matrix. Lines at their Hang
// load and slack ropes are the bounds; the rest is solved exactly, so chains of light and heavy
// bodies converge in one step. Held is the starting guess (any guess terminates) and returns
// the final partition. Returns false if it still hit its limit; λ is then finished by
// projected Gauss-Seidel.
bool SolveBounded(const std::vector<std::vector<double>>& Matrix, const std::vector<double>& Rhs,
                  const std::vector<FRow>& Rows, std::vector<double>& Lambda, std::vector<EHeld>& Held)
{
	const size_t N = Rows.size();
	Lambda.assign(N, 0.0);
	double Scale = 1e-300;
	for (const double Value : Rhs)
	{
		Scale = std::max(Scale, std::abs(Value));
	}
	const double Tolerance = 1e-10 * Scale;
	const size_t Limit = 50 * N + 100;
	for (size_t Iteration = 0; Iteration < Limit; ++Iteration)
	{
		std::vector<size_t> Free;
		for (size_t I = 0; I < N; ++I)
		{
			if (Held[I] == EHeld::Free)
			{
				Free.push_back(I);
			}
			else
			{
				Lambda[I] = Held[I] == EHeld::AtLow ? Rows[I].Low : Rows[I].High;
			}
		}
		std::vector<double> Reduced(Free.size());
		for (size_t I = 0; I < Free.size(); ++I)
		{
			Reduced[I] = Rhs[Free[I]];
			for (size_t J = 0; J < N; ++J)
			{
				if (Held[J] != EHeld::Free)
				{
					Reduced[I] -= Matrix[Free[I]][J] * Lambda[J];
				}
			}
		}
		const std::vector<double> Solved = SolveFree(Matrix, Reduced, Free);
		for (size_t I = 0; I < Free.size(); ++I)
		{
			Lambda[Free[I]] = Solved[I];
		}
		size_t Wrong = N;
		EHeld Next = EHeld::Free;
		for (size_t I = 0; I < N && Wrong == N; ++I)
		{
			const double ImpulseTolerance = 1e-12 * (1.0 + std::abs(Lambda[I]));
			if (Held[I] == EHeld::Free)
			{
				if (Lambda[I] < Rows[I].Low - ImpulseTolerance)
				{
					Wrong = I;
					Next = EHeld::AtLow;
				}
				else if (Lambda[I] > Rows[I].High + ImpulseTolerance)
				{
					Wrong = I;
					Next = EHeld::AtHigh;
				}
				continue;
			}
			double Residual = Rhs[I];
			for (size_t J = 0; J < N; ++J)
			{
				Residual -= Matrix[I][J] * Lambda[J];
			}
			if ((Held[I] == EHeld::AtLow && Residual > Tolerance) || (Held[I] == EHeld::AtHigh && Residual < -Tolerance))
			{
				Wrong = I;
			}
		}
		if (Wrong == N)
		{
			return true;
		}
		Held[Wrong] = Next;
	}
	// Not expected: finish from the last iterate by projected Gauss-Seidel.
	for (size_t I = 0; I < N; ++I)
	{
		Lambda[I] = std::clamp(Lambda[I], Rows[I].Low, Rows[I].High);
	}
	for (int Pass = 0; Pass < 2000; ++Pass)
	{
		for (size_t I = 0; I < N; ++I)
		{
			double Residual = Rhs[I];
			for (size_t J = 0; J < N; ++J)
			{
				Residual -= Matrix[I][J] * Lambda[J];
			}
			Lambda[I] = std::clamp(Lambda[I] + Residual / Matrix[I][I], Rows[I].Low, Rows[I].High);
		}
	}
	return false;
}

// Groups of rows that share a movable body: each group is solved on its own, so the cost
// follows the connected groups, not the whole scene. Static ends do not join groups.
std::vector<std::vector<size_t>> GroupRows(const std::vector<FHangBody>& Bodies, const std::vector<FRow>& Rows)
{
	std::vector<int> Parent(Bodies.size());
	for (size_t I = 0; I < Parent.size(); ++I)
	{
		Parent[I] = static_cast<int>(I);
	}
	const auto Find = [&Parent](int X) {
		while (Parent[X] != X)
		{
			X = Parent[X] = Parent[Parent[X]];
		}
		return X;
	};
	const auto Movable = [&Bodies](int Body) { return Body >= 0 && Bodies[Body].InvMass > 0.0; };
	for (const FRow& Row : Rows)
	{
		if (Movable(Row.Bodies[0]) && Movable(Row.Bodies[1]))
		{
			Parent[Find(Row.Bodies[0])] = Find(Row.Bodies[1]);
		}
	}
	std::vector<std::vector<size_t>> Groups;
	std::vector<int> GroupOf(Bodies.size(), -1);
	for (size_t I = 0; I < Rows.size(); ++I)
	{
		const int Body = Movable(Rows[I].Bodies[0]) ? Rows[I].Bodies[0] : Rows[I].Bodies[1];
		const int Root = Find(Body);
		if (GroupOf[Root] < 0)
		{
			GroupOf[Root] = static_cast<int>(Groups.size());
			Groups.emplace_back();
		}
		Groups[GroupOf[Root]].push_back(I);
	}
	return Groups;
}

// One group's rows, solved together: J M⁻¹ Jᵀ, bounded, with the targets refreshed from the
// current estimate (a swing's target depends on its tangential speed).
void SolveGroup(std::vector<FHangBody>& Bodies, std::vector<FHangLine>& Lines, std::vector<FLineWork>& Work,
                const std::vector<FRow>& AllRows, const std::vector<size_t>& Group, double H, FHeldGuess& Guess)
{
	// Lines between the same two bodies are the same constraint: merge them into one row with
	// their summed bounds and share its impulse in proportion to the Hang loads. Solving them
	// as separate rows leaves the split to rounding.
	std::vector<FRow> Rows;
	std::vector<std::vector<size_t>> Members;
	const auto SameBodies = [](const FRow& X, const FRow& Y) {
		return (X.Bodies[0] == Y.Bodies[0] && X.Bodies[1] == Y.Bodies[1]) ||
		       (X.Bodies[0] == Y.Bodies[1] && X.Bodies[1] == Y.Bodies[0]);
	};
	for (const size_t I : Group)
	{
		const FRow& Row = AllRows[I];
		size_t Found = Rows.size();
		for (size_t K = 0; K < Rows.size() && Row.Line >= 0; ++K)
		{
			Found = (Found == Rows.size() && Rows[K].Line >= 0 && SameBodies(Rows[K], Row)) ? K : Found;
		}
		if (Found == Rows.size())
		{
			Rows.push_back(Row);
			Members.push_back({I});
			continue;
		}
		Rows[Found].Low += Row.Low;
		Rows[Found].High += Row.High;
		Members[Found].push_back(I);
		// The tightest line sets the merged row's target.
		if (Work[Row.Line].TargetLength < Work[Rows[Found].Line].TargetLength)
		{
			Rows[Found].Line = Row.Line;
		}
	}
	const size_t N = Rows.size();
	std::vector<std::vector<double>> Matrix(N, std::vector<double>(N, 0.0));
	std::vector<int> Touched;
	for (size_t I = 0; I < N; ++I)
	{
		for (int K = 0; K < 2; ++K)
		{
			if (Rows[I].Bodies[K] >= 0 && Bodies[Rows[I].Bodies[K]].InvMass > 0.0)
			{
				Touched.push_back(Rows[I].Bodies[K]);
			}
		}
		for (size_t J = 0; J < N; ++J)
		{
			for (int K = 0; K < 2; ++K)
			{
				for (int L = 0; L < 2; ++L)
				{
					const int Body = Rows[I].Bodies[K];
					if (Body >= 0 && Body == Rows[J].Bodies[L])
					{
						Matrix[I][J] += Bodies[Body].InvMass * Rows[I].Dirs[K].Dot(Rows[J].Dirs[L]);
					}
				}
			}
		}
	}
	// Ties between parallel rows go to the floor: a line straight above a standing body takes
	// none of the weight the floor carries, and parallel lines split evenly.
	for (size_t I = 0; I < N; ++I)
	{
		Matrix[I][I] *= 1.0 + (Rows[I].Line >= 0 ? LineRegularization : FloorRegularization);
	}
	std::sort(Touched.begin(), Touched.end());
	Touched.erase(std::unique(Touched.begin(), Touched.end()), Touched.end());
	std::vector<FVec3> Start;
	for (const int Body : Touched)
	{
		Start.push_back(Bodies[Body].Velocity);
	}

	std::vector<double> StartSpeed(N);
	for (size_t I = 0; I < N; ++I)
	{
		StartSpeed[I] = RowSpeed(Bodies, Rows[I]);
	}
	std::vector<EHeld> Held(N);
	for (size_t I = 0; I < N; ++I)
	{
		Held[I] = Rows[I].Line >= 0 ? Guess.Lines[Rows[I].Line] : Guess.Contacts[Rows[I].Bodies[0]];
	}
	std::vector<double> Lambda(N, 0.0);
	for (int Refresh = 0; Refresh < TargetRefreshes; ++Refresh)
	{
		std::vector<double> Rhs(N);
		for (size_t I = 0; I < N; ++I)
		{
			const FRow& Row = Rows[I];
			const double Target = Row.Line >= 0
			                          ? DriveSpeed(Work[Row.Line], RelativeVelocity(Bodies, Lines[Row.Line]), H)
			                          : -(Row.Floor - Bodies[Row.Bodies[0]].Position.Z) / H;
			Rhs[I] = StartSpeed[I] - Target;
		}
		if (!SolveBounded(Matrix, Rhs, Rows, Lambda, Held))
		{
			for (size_t I = 0; I < N; ++I)
			{
				for (const size_t Member : Members[I])
				{
					if (AllRows[Member].Line >= 0)
					{
						Lines[AllRows[Member].Line].Events |= LineEvents::SolverLimit;
					}
				}
			}
		}
		for (size_t B = 0; B < Touched.size(); ++B)
		{
			Bodies[Touched[B]].Velocity = Start[B];
		}
		for (size_t I = 0; I < N; ++I)
		{
			ApplyRow(Bodies, Rows[I], Lambda[I]);
		}
	}
	for (size_t I = 0; I < N; ++I)
	{
		if (Rows[I].Line < 0)
		{
			Guess.Contacts[Rows[I].Bodies[0]] = Held[I];
			continue;
		}
		for (const size_t Member : Members[I])
		{
			const FRow& Own = AllRows[Member];
			const double Share = Lambda[I] >= 0.0 ? (Rows[I].High > 0.0 ? Own.High / Rows[I].High : 1.0 / static_cast<double>(Members[I].size()))
			                                      : (Rows[I].Low < 0.0 ? Own.Low / Rows[I].Low : 0.0);
			Work[Own.Line].Impulse = Lambda[I] * Share;
			Guess.Lines[Own.Line] = Held[I];
		}
	}
}

// The velocity solve for one substep: every active line, and the floor under every body that
// could reach it this substep.
void SolveVelocities(std::vector<FHangBody>& Bodies, std::vector<FHangLine>& Lines, std::vector<FLineWork>& Work,
                     double H, FHeldGuess& Guess)
{
	std::vector<FRow> Rows;
	std::vector<double> PullReach(Bodies.size(), 0.0);  // m/s the lines could add this substep
	for (size_t I = 0; I < Lines.size(); ++I)
	{
		const FLineWork& W = Work[I];
		if (!W.bActive)
		{
			continue;
		}
		FRow Row;
		Row.Bodies[0] = Lines[I].BodyA;
		Row.Dirs[0] = W.Axis * -1.0;
		Row.Bodies[1] = Lines[I].BodyB;
		Row.Dirs[1] = W.Axis;
		Row.Low = Lines[I].State->bRodMode ? -W.Cap : 0.0;
		Row.High = W.Cap;
		Row.Line = static_cast<int>(I);
		Rows.push_back(Row);
		PullReach[Lines[I].BodyA] += W.Cap * Bodies[Lines[I].BodyA].InvMass;
		PullReach[Lines[I].BodyB] += W.Cap * Bodies[Lines[I].BodyB].InvMass;
	}
	for (size_t I = 0; I < Bodies.size(); ++I)
	{
		const FHangBody& Body = Bodies[I];
		const double Reach = (Body.Velocity.Length() + PullReach[I]) * H + LengthEpsilon;
		if (Body.FloorZ && Body.InvMass > 0.0 && Body.Position.Z - *Body.FloorZ <= Reach)
		{
			FRow Row;
			Row.Bodies[0] = static_cast<int>(I);
			Row.Dirs[0] = Up * -1.0;  // c = −v_z: a positive impulse lifts
			Row.Low = 0.0;
			Row.High = std::numeric_limits<double>::infinity();
			Row.Floor = *Body.FloorZ;
			Rows.push_back(Row);
		}
	}
	for (const std::vector<size_t>& Group : GroupRows(Bodies, Rows))
	{
		SolveGroup(Bodies, Lines, Work, Rows, Group, H, Guess);
	}
}

struct FRestChange
{
	double Reel = 0.0;  // m
	double Slip = 0.0;  // m
};

// How the rest length moves this substep. Holding: by the Reel. At the Hang load but still
// reeling: by what the winch achieved, never against the input. Slipping (the load still
// separates faster than the line allows): it pays out with the load while holding the Hang
// load, and a reel-in gains nothing.
//
// Neither end movable: nothing gives but the line. A kinematic stretching it meets the full
// Hang load and slips (bible 6.7: a line between a kinematic and anything else moves the
// anything else; the Hang force breaks a brace next step, 5.4, and a placed object, which Hang
// never starts, 6.2, keeps paying the line out until it runs out).
FRestChange RestChange(const std::vector<FHangBody>& Bodies, FHangLine& Line, FLineWork& W, double H)
{
	const double Rate = LengthRate(Bodies, Line, W, H);
	const double Tolerance = SlipTolerance / H;
	if (W.bImmovable)
	{
		const bool bStretched = Rate > Tolerance;
		const bool bCompressed = Line.State->bRodMode && Rate < -Tolerance;
		W.Impulse = bStretched ? W.Cap : bCompressed ? -W.Cap : 0.0;
	}
	else if (!W.bActive)
	{
		return {W.Target * H, 0.0};
	}
	const bool bAtCap = W.Cap > 0.0 ? std::abs(W.Impulse) >= W.Cap * (1.0 - CapTolerance) : true;
	const double OutwardLimit = std::max(W.Target, 0.0);
	const double InwardLimit = std::min(W.Target, 0.0);
	const bool bSlipOut = bAtCap && W.Impulse >= 0.0 && Rate > OutwardLimit + Tolerance;
	const bool bSlipIn = bAtCap && W.Impulse <= 0.0 && Line.State->bRodMode && Rate < InwardLimit - Tolerance;
	if (!bSlipOut && !bSlipIn)
	{
		const double Achieved = bAtCap ? std::clamp(Rate, InwardLimit, OutwardLimit) : W.Target;
		return {Achieved * H, 0.0};
	}
	Line.Events |= LineEvents::Slipping;
	const double Base = bSlipOut ? OutwardLimit : InwardLimit;
	return {Base * H, (Rate - Base) * H};
}

// A line that slips past the end of its filament, or in to the Dogs-meet length, lets go
// (PROPOSED: the bible does not say; holding there would make the line infinitely strong).
void ApplyRestChange(FHangLine& Line, const FLineWork& W, const FRestChange& Change, const FSpoolGlobals& G)
{
	FLineState& S = *Line.State;
	if (S.Mode != ELineMode::Hang && S.Mode != ELineMode::Set)
	{
		return;
	}
	const double Rest = W.RestBefore + Change.Reel + Change.Slip;
	if (Rest > HighBound(W, Line) + LengthEpsilon || Rest < LowBound(W, G) - LengthEpsilon)
	{
		S.Mode = ELineMode::Slack;
		S.ReelInput = 0;
		Line.Events |= LineEvents::RanOut;
		return;
	}
	S.ReelAccum += Change.Reel;
	S.SlipAccum += Change.Slip;
}

// Removes drift from the rest lengths, from each line's correction budget for the step, then
// keeps bodies above their floors. Returns the largest correction.
double PositionPass(std::vector<FHangBody>& Bodies, const std::vector<FHangLine>& Lines, const std::vector<FLineWork>& Work,
                    std::vector<double>& Budget)
{
	double Largest = 0.0;
	for (size_t I = 0; I < Lines.size(); ++I)
	{
		const FHangLine& Line = Lines[I];
		const double SumInvMass = Work[I].SumInvMass;
		const ELineMode Mode = Line.State->Mode;
		if (SumInvMass <= 0.0 || (Mode != ELineMode::Hang && Mode != ELineMode::Set))
		{
			continue;
		}
		FHangBody& A = Bodies[Line.BodyA];
		FHangBody& B = Bodies[Line.BodyB];
		const FVec3 Offset = B.Position - A.Position;
		if (Offset.Length() <= LengthEpsilon)
		{
			continue;
		}
		const double Stretch = Offset.Length() - RestOf(*Line.State);
		if (Stretch <= 0.0 && !Line.State->bRodMode)
		{
			continue;
		}
		const double Correction = std::clamp(Stretch, -Budget[I], Budget[I]);
		Budget[I] -= std::abs(Correction);
		const FVec3 Axis = Offset.SafeNormal();
		A.Position = A.Position + Axis * (A.InvMass * Correction / SumInvMass);
		B.Position = B.Position - Axis * (B.InvMass * Correction / SumInvMass);
		Largest = std::max(Largest, std::abs(Correction));
	}
	for (FHangBody& Body : Bodies)
	{
		if (Body.FloorZ && Body.InvMass > 0.0)
		{
			Body.Position.Z = std::max(Body.Position.Z, *Body.FloorZ);
		}
	}
	return Largest;
}

// Substeps so no line turns more than MaxSweep per substep (a Bola, a short fast swing).
int SubstepsFor(const std::vector<FHangBody>& Bodies, const std::vector<FHangLine>& Lines, double Dt)
{
	double Sweep = 0.0;
	for (const FHangLine& Line : Lines)
	{
		const FVec3 Offset = Bodies[Line.BodyB].Position - Bodies[Line.BodyA].Position;
		const double Length = Offset.Length();
		if (Length <= LengthEpsilon)
		{
			continue;
		}
		const FVec3 Axis = Offset * (1.0 / Length);
		const FVec3 Relative = RelativeVelocity(Bodies, Line);
		Sweep = std::max(Sweep, (Relative - Axis * Relative.Dot(Axis)).Length() * Dt / Length);
	}
	return std::clamp(static_cast<int>(std::ceil(Sweep / MaxSweep)), 1, MaxSubsteps);
}
} // namespace

void SolveHangLines(std::vector<FHangBody>& Bodies, std::vector<FHangLine>& Lines, const FSpoolGlobals& Globals,
                    double Dt, const FHangSolverSettings& Settings)
{
	const int Substeps = SubstepsFor(Bodies, Lines, Dt);
	const double H = Dt / Substeps;
	std::vector<double> Total(Lines.size(), 0.0);
	std::vector<FVec3> LastAxis(Lines.size());
	std::vector<double> Budget(Lines.size(), MaxCorrectionPerStep);
	FHeldGuess Guess{std::vector<EHeld>(Lines.size(), EHeld::Free), std::vector<EHeld>(Bodies.size(), EHeld::Free)};
	for (FHangLine& Line : Lines)
	{
		Line.Events = 0;
	}
	for (int Step = 0; Step < Substeps; ++Step)
	{
		std::vector<FLineWork> Work;
		Work.reserve(Lines.size());
		for (const FHangLine& Line : Lines)
		{
			Work.push_back(Prepare(Bodies, Line, Globals, H));
		}
		SolveVelocities(Bodies, Lines, Work, H, Guess);
		for (size_t I = 0; I < Lines.size(); ++I)
		{
			ApplyRestChange(Lines[I], Work[I], RestChange(Bodies, Lines[I], Work[I], H), Globals);
			Total[I] += Work[I].Impulse;
			LastAxis[I] = Work[I].Axis;
		}
		for (FHangBody& Body : Bodies)
		{
			Body.Position = Body.Position + Body.Velocity * H;
		}
		for (int Pass = 0; Pass < Settings.MaxPositionIterations; ++Pass)
		{
			if (PositionPass(Bodies, Lines, Work, Budget) < StretchTolerance)
			{
				break;
			}
		}
	}
	for (size_t I = 0; I < Lines.size(); ++I)
	{
		FHangLine& Line = Lines[I];
		const double Force = Total[I] / Dt;
		Line.AppliedForce = std::abs(Force);
		Line.State->AppliedForce = Line.AppliedForce;
		Line.ForceOnA = LastAxis[I] * Force;
		Line.ForceOnB = LastAxis[I] * -Force;
	}
}
} // namespace Kinetic
