// Unit conversion at the engine boundary. Gameplay data and this library are SI (meters,
// seconds, kilograms, newtons); Unreal lengths are centimeters, so forces in the engine are
// kg·cm/s² (1 N = 100 kg·cm/s²). spool-and-line-implementation.md, header note.
#pragma once

#include "Kinetic/Api.h"

#include "Kinetic/Vec3.h"

namespace Kinetic::Units
{
inline constexpr double CmPerMeter = 100.0;

constexpr double MetersToCm(double Meters) { return Meters * CmPerMeter; }
constexpr double CmToMeters(double Cm) { return Cm / CmPerMeter; }
constexpr FVec3 MetersToCm(const FVec3& Meters) { return Meters * CmPerMeter; }
constexpr FVec3 CmToMeters(const FVec3& Cm) { return Cm / CmPerMeter; }

// Newtons to Unreal force units (kg·cm/s²) and back.
constexpr double NewtonsToEngineForce(double Newtons) { return Newtons * CmPerMeter; }
constexpr double EngineForceToNewtons(double Force) { return Force / CmPerMeter; }
} // namespace Kinetic::Units
