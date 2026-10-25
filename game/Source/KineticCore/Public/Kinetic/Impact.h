// Impact classification on a player: stagger, flip, Stall. bible/05 sections 6.4, 7.1 to
// 7.3; player-physics.md section 4 (the Impact Monitor). Thresholds are inclusive and are
// multiplied by the other surface's modifier.
#pragma once

#include "Kinetic/Api.h"

#include "Kinetic/Tables.h"
#include "Kinetic/Vec3.h"

#include <cstdint>
#include <map>
#include <vector>

namespace Kinetic
{
enum class EOutcome : uint8_t { None, Stagger, Flip, Stall };

KINETICCORE_API EOutcome MoreSevere(EOutcome A, EOutcome B);

// Value is a relative speed (m/s) or an impulse (N·s), whichever the table measures.
KINETICCORE_API EOutcome Classify(EImpactTable Table, double Value, ESurface Surface, const FKineticTables& Tables);

// Player against player (7.2): each party is judged on the other's mass times the relative
// speed against the object table, with the symmetric 14 m/s floor; the more severe wins.
KINETICCORE_API EOutcome ClassifyPlayerVsPlayer(double OtherMassKg, double RelativeSpeed, const FKineticTables& Tables);

inline double ImpactImpulse(double MassKg, double Speed) { return MassKg * Speed; }

// Blasts (6.4). Propane's Stall and flip radii are authored results that override the falloff.
// Under more than 1 m of water the impulse halves and is judged on the falloff alone (PROPOSED:
// the bible halves the impulse but does not say whether the authored radii still apply).
struct FBlastResult
{
	EOutcome Outcome = EOutcome::None;
	double ImpulseNs = 0.0;
};

KINETICCORE_API FBlastResult ClassifyBlast(EVolatile Kind, double DistanceM, bool bDeepUnderwater, const FKineticTables& Tables);

// One step's contacts on a player, debounced and filtered (player-physics 4.1 and 4.2).
enum class EContactKind : uint8_t { Static, Kinematic, Object, Player };

struct FContact
{
	int OtherId = 0;
	EContactKind Kind = EContactKind::Static;
	double RelativeSpeed = 0.0;   // m/s along the contact normal
	double ImpulseNs = 0.0;       // Solver impulse this contact point
	double OtherMassKg = 0.0;     // Player and object contacts
	ESurface Surface = ESurface::Hard;
	FVec3 Normal = {};            // The direction the contact pushes the player; required for Object
	                              // contacts (asserted; without asserts a missing one counts as bearing down)
};

// How each contact is judged (player-physics 4.1):
// - Static and kinematic: relative speed only. The threshold is speed against the world,
//   whatever the player's mass (7.2: a Breaker hitting a wall at 12 m/s Stalls like anyone).
// - Object: the solver impulse summed over the body's contact points, less the share of its
//   weight the player carries, m_other·g·dt·max(0, −n·up) per step (PROPOSED: without it a
//   2,000 kg container sitting still on a player reads 327 N·s every 60 Hz step and staggers
//   them four times a second; a sustained heavy load is the crush test's job, 7.1). An object
//   arriving sideways bears its weight on the floor, so nothing is subtracted.
// - Player: 7.2, the other's mass times the relative speed, with the 14 m/s floor.
// Every threshold times every surface modifier is above player-physics 4.2's resting limits
// (1 m/s, 50 N·s), so a resting contact never classifies; ImpactTest checks the tables.

// The share of an object's weight a contact pushing the player along Normal carries: 1 straight
// down onto them, 0 sideways or from below. A missing normal counts as 1, so a resting weight
// never reads as a hit when a caller forgets it (EndStep asserts in builds with asserts).
KINETICCORE_API double SupportShare(const FVec3& Normal);

class KINETICCORE_API FImpactMonitor
{
public:
	explicit FImpactMonitor(const FKineticTables& InTables) : Tables(InTables) {}

	// Call after a body handoff (grounded to flight body): the settle is not a hit.
	void NotifyHandoff() { SettleStepsLeft = HandoffSettleSteps; }

	// Classifies one physics step of Dt seconds. Impulses are summed per other body within the
	// step, never across bodies; the most severe outcome across bodies is returned.
	// Debounce (4.2): a body that produced a Stagger or Flip is ignored for 0.25 s unless the
	// measure that judged it (speed, impulse or m_other·v) has at least doubled. A Stall is
	// never debounced (PROPOSED: 4.2 only exempts a doubled hit, but a Stall is the end of the
	// player's run and must never be swallowed).
	EOutcome EndStep(const std::vector<FContact>& Contacts, double Now, double Dt);

	static constexpr double RestingSpeed = 1.0;     // m/s, player-physics 4.2
	static constexpr double RestingImpulse = 50.0;  // N·s, player-physics 4.2
	static constexpr double DebounceSeconds = 0.25;
	static constexpr double DebounceEscalation = 2.0;
	static constexpr int HandoffSettleSteps = 2;

private:
	struct FLastHit
	{
		double Time = -1e9;
		double Measure = 0.0;
	};

	double Measure(const FContact& Summed, double Dt) const;
	EOutcome ClassifyBody(const FContact& Summed, double MeasureValue) const;

	const FKineticTables& Tables;
	std::map<int, FLastHit> LastHits;
	int SettleStepsLeft = 0;
};

// The crush test (7.1, player-physics 4.3): two bodies pressing from opposing sides, each at or
// over the crush force (inclusive, 6.2), at least one a kinematic or a body of Heavy mass, held
// for the crush time. Contact points are summed per body first, as Chaos reports several
// manifold points per body pair. The timer decays at twice the rate when the condition lapses.
// Line-only contacts never count.
struct FCrushContact
{
	int OtherId = 0;
	FVec3 Normal;
	double ForceN = 0.0;
	bool bKinematicOrHeavy = false;
};

class KINETICCORE_API FCrushDetector
{
public:
	explicit FCrushDetector(const FKineticTables& InTables) : Tables(InTables) {}
	bool Update(const std::vector<FCrushContact>& Contacts, double Dt);
	double Seconds() const { return Timer; }

	static constexpr double OpposingDot = -0.7;
	static constexpr double DecayRate = 2.0;

private:
	bool IsPinned(const std::vector<FCrushContact>& Contacts) const;

	const FKineticTables& Tables;
	double Timer = 0.0;
};
} // namespace Kinetic
