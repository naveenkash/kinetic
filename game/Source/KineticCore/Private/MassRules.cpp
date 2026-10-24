#include "Kinetic/MassRules.h"

#include <algorithm>

namespace Kinetic
{
namespace
{
// Floating-point slack for hold timers that should land exactly on a step boundary.
constexpr double TimeEpsilon = 1e-9;
} // namespace

FPlayerBody PlayerBodyFor(const FHarnessTuning& Harness, bool bBraced, bool bGrounded)
{
	return {Harness.MassKg, Harness.FootHold, Harness.BracedFootHold, bBraced && Harness.bCanBrace, bGrounded};
}

FPlayerLineResponse ResolvePlayerUnderLine(const FPlayerBody& Player, const FVec3& LineForce, bool bKickActive,
                                           double Gravity)
{
	const double FreeInvMass = 1.0 / Player.MassKg;
	// Foot-hold is judged on the pull along the ground, and holds like friction: a line pulling
	// up takes weight off the feet and lowers foot-hold in proportion, to nothing once it lifts
	// the whole weight (the hoist below); pulling down does not raise it (PROPOSED: 5.3's
	// examples are all along the ground). A flat pull is judged exactly as 5.3 says.
	const double Upward = LineForce.Dot(Up);
	const double ForceN = (LineForce - Up * Upward).Length();
	const double Unloaded = std::clamp(Upward / (Player.MassKg * Gravity), 0.0, 1.0);
	const double Acceleration = ForceN / Player.MassKg;
	FPlayerLineResponse Out;

	if (!Player.bGrounded)
	{
		Out.Footing = EFooting::Airborne;
		Out.InvMass = FreeInvMass;
		return Out;
	}
	if (Player.bBraced)
	{
		if (Acceleration <= Player.BracedFootHold * (1.0 - Unloaded))
		{
			Out.Footing = EFooting::Braced;  // Static to this line, the Kick included
			return Out;
		}
		Out.bBraceBroken = true;  // Over the brace foot-hold: the body is free
	}
	const bool bFeetUnloaded = Upward >= Player.MassKg * Gravity;
	if (!bKickActive && bFeetUnloaded && Upward > HoistUpwardShare * LineForce.Length())
	{
		Out.Footing = EFooting::Hoisted;
		Out.InvMass = FreeInvMass;
		return Out;
	}
	if (bKickActive || Out.bBraceBroken || Acceleration > Player.FootHold * (1.0 - Unloaded))
	{
		Out.Footing = EFooting::Airborne;
		Out.InvMass = FreeInvMass;
		return Out;
	}
	Out.Footing = EFooting::Skid;
	Out.InvMass = FreeInvMass;
	Out.DragAcceleration = (LineForce - Up * Upward) / Player.MassKg;  // Along the ground
	return Out;
}

EMassTier TierOf(double MassKg, bool bStructural, const FMassTierBounds& Bounds)
{
	if (bStructural || MassKg > Bounds.HeavyUpperKg)
	{
		return EMassTier::Rated;
	}
	if (MassKg < Bounds.LightUpperKg)
	{
		return EMassTier::Light;
	}
	return MassKg < Bounds.MediumUpperKg ? EMassTier::Medium : EMassTier::Heavy;
}

bool IsSmallPropBonusTarget(double MassKg, const FKineticTables& Tables)
{
	const EMassTier Tier = TierOf(MassKg, false, Tables.MassTiers());
	return Tier == EMassTier::Light ||
	       (Tier == EMassTier::Medium && MassKg < Tables.Globals().SmallPropBonusMassCap);
}

FObjectLineResponse ResolveObjectUnderLines(const FObjectBody& Object, const FVec3& ActivePull, bool bOnlyHangLines,
                                            const FKineticTables& Tables)
{
	const double FreeInvMass = 1.0 / Object.MassKg;
	const double PullN = ActivePull.Length();
	FObjectLineResponse Out;

	if (!CanEverMoveUnderALine(Object))
	{
		return Out;  // Fixed, or rated: moves only through its commit timeline
	}
	if (Object.bBrokenFree)
	{
		Out.InvMass = FreeInvMass;
		return Out;
	}
	if (Object.bKinematicAttachment)
	{
		if (MeetsBreakFree(PullN, Tables.BreakFreeN(EPlacement::Rated)))
		{
			Out.bReleasedFromPath = true;
			Out.InvMass = FreeInvMass;
		}
		return Out;
	}
	if (Object.Placement == EPlacement::Loose)
	{
		Out.InvMass = FreeInvMass;
		return Out;
	}
	if (bOnlyHangLines)
	{
		return Out;  // The placement carries a Hang line's load (6.2)
	}
	if (MeetsBreakFree(PullN, Tables.BreakFreeN(Object.Placement)))
	{
		Out.bBreaksFreeNow = true;
		Out.InvMass = FreeInvMass;
	}
	return Out;
}

bool CanEverMoveUnderALine(const FObjectBody& Object)
{
	return !Object.bFixedScenery && (!Object.bRated || Object.bKinematicAttachment);
}

bool FRatedCommit::Update(double PullAlongAxisN, double Dt, const FKineticTables& Tables)
{
	if (bCommitted)
	{
		return true;
	}
	Held = MeetsBreakFree(PullAlongAxisN, Tables.BreakFreeN(EPlacement::Rated)) ? Held + Dt : 0.0;
	bCommitted = Held + TimeEpsilon >= Tables.Globals().RatedCommitHold;
	return bCommitted;
}

double ForceVolumeAcceleration(double ARef, double MassKg, const FSpoolGlobals& Globals)
{
	return std::min(ARef * Globals.ForceVolumeReferenceMassKg / MassKg, Globals.ForceVolumeCapMul * ARef);
}
} // namespace Kinetic
