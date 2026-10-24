// Spool-level rules that set a line's tension and its release impulses: overspin, the
// Doubler's two-line split, the overspin snap and the cut recoil. bible/05 sections 3.7,
// 3.8 and 4.2.
#pragma once

#include "Kinetic/Tables.h"

#include <cstdint>
#include <limits>

namespace Kinetic
{
struct FTensionInputs
{
	double OverspinHeldSeconds = 0.0;  // Mode input held past engagement (Doubler only)
	bool bSecondLineLive = false;      // Doubler with two live lines
	double SecondsSinceSecondLineReleased = std::numeric_limits<double>::infinity();
};

// The owner's current pull tension for one line, in newtons.
double LineTension(const FHarnessTuning& Harness, const FSpoolGlobals& Globals, const FTensionInputs& Inputs);

// On Hang, overspin raises the Hang load instead of the pull, in proportion to the tension
// gain (PROPOSED: the bible states the direction, not the curve).
double HangLoadWithOverspin(const FHarnessTuning& Harness, const FSpoolGlobals& Globals, double OverspinHeldSeconds);

enum class EOverspinStage : uint8_t { None, Escalating, Octave, Snapped };

EOverspinStage OverspinStage(const FHarnessTuning& Harness, const FSpoolGlobals& Globals, double OverspinHeldSeconds);

// The single impulse the far Dog's body receives when an overspun line snaps (3.7).
double SnapImpulseNs(double AppliedForceN, const FSpoolGlobals& Globals);

// The recoil both ends receive when a line is cut (4.2).
double CutRecoilNs(double AppliedForceN, const FSpoolGlobals& Globals);
} // namespace Kinetic
