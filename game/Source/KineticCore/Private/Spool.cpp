#include "Kinetic/Spool.h"

#include <algorithm>

namespace Kinetic
{
namespace
{
constexpr double NewtonsPerKilonewton = 1000.0;

bool CanOverspin(const FHarnessTuning& Harness) { return Harness.OverspinTensionN > Harness.TensionN; }

// 0 to 1 over the ramp: tension escalates from T toward the overspin tension over 1.0 s.
double OverspinProgress(const FHarnessTuning& Harness, const FSpoolGlobals& Globals, double Held)
{
	if (!CanOverspin(Harness) || Held <= 0.0)
	{
		return 0.0;
	}
	return std::clamp(Held / Globals.OverspinRamp, 0.0, 1.0);
}

// 1, or the Doubler split while two lines are live, restoring linearly after one is released.
double SplitFactor(const FHarnessTuning& Harness, const FSpoolGlobals& Globals, const FTensionInputs& Inputs)
{
	if (Harness.ActiveLines < 2)
	{
		return 1.0;
	}
	if (Inputs.bSecondLineLive)
	{
		return Globals.DoublerSplit;
	}
	const double Restored = std::clamp(Inputs.SecondsSinceSecondLineReleased / Globals.DoublerRestore, 0.0, 1.0);
	return Globals.DoublerSplit + (1.0 - Globals.DoublerSplit) * Restored;
}
} // namespace

double LineTension(const FHarnessTuning& Harness, const FSpoolGlobals& Globals, const FTensionInputs& Inputs)
{
	const double Progress = OverspinProgress(Harness, Globals, Inputs.OverspinHeldSeconds);
	const double Base = Harness.TensionN + (Harness.OverspinTensionN - Harness.TensionN) * Progress;
	return Base * SplitFactor(Harness, Globals, Inputs);
}

double HangLoadWithOverspin(const FHarnessTuning& Harness, const FSpoolGlobals& Globals, double OverspinHeldSeconds)
{
	const double Progress = OverspinProgress(Harness, Globals, OverspinHeldSeconds);
	const double Gain = CanOverspin(Harness) ? Harness.OverspinTensionN / Harness.TensionN - 1.0 : 0.0;
	return Harness.HangLoadN * (1.0 + Gain * Progress);
}

EOverspinStage OverspinStage(const FHarnessTuning& Harness, const FSpoolGlobals& Globals, double OverspinHeldSeconds)
{
	if (!CanOverspin(Harness) || OverspinHeldSeconds <= 0.0)
	{
		return EOverspinStage::None;
	}
	if (OverspinHeldSeconds >= Globals.OverspinSnap)
	{
		return EOverspinStage::Snapped;
	}
	return OverspinHeldSeconds >= Globals.OverspinOctave ? EOverspinStage::Octave : EOverspinStage::Escalating;
}

double SnapImpulseNs(double AppliedForceN, const FSpoolGlobals& Globals)
{
	return Globals.SnapImpulseBase * AppliedForceN / Globals.SnapReferenceForce;
}

double CutRecoilNs(double AppliedForceN, const FSpoolGlobals& Globals)
{
	return Globals.CutRecoilPer1000N * AppliedForceN / NewtonsPerKilonewton;
}
} // namespace Kinetic
