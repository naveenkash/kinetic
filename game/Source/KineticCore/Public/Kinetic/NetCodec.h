// Replication encodings (docs/tech/networking.md section 3): quantized vectors at 1 cm in 16
// bits an axis within arena bounds, a smallest-three quaternion in 6 bytes, the 32-byte body
// payload (position, rotation, linear and angular velocity, server frame and flags) behind the
// engine's 16-byte actor header, and the 5-byte line update. Engine-free: the Unreal side
// writes these bytes through its own archive.
#pragma once

#include "Kinetic/Api.h"

#include "Kinetic/Vec3.h"

#include <cstdint>
#include <optional>
#include <vector>

namespace Kinetic
{
// Appends and reads bit fields, least significant bit first.
class KINETICCORE_API FBitWriter
{
public:
	void Write(uint64_t Value, int Bits);
	const std::vector<uint8_t>& Bytes() const { return Data; }
	int BitCount() const { return Count; }

private:
	std::vector<uint8_t> Data;
	int Count = 0;
};

class KINETICCORE_API FBitReader
{
public:
	explicit FBitReader(const std::vector<uint8_t>& InData) : Data(InData) {}
	explicit FBitReader(std::vector<uint8_t>&&) = delete;  // It reads in place: no temporaries
	// Nothing once the data runs out.
	std::optional<uint64_t> Read(int Bits);

private:
	const std::vector<uint8_t>& Data;
	int Cursor = 0;
};

// The arena's box: positions inside it quantize to 1 cm; outside, they clamp to its faces.
struct FArenaBounds
{
	FVec3 Min{-327.0, -327.0, -100.0};
	FVec3 Max{327.0, 327.0, 300.0};
};

inline constexpr double PositionStep = 0.01;   // m
inline constexpr double LinearSpeedRange = 50.0;  // m/s, the 6.3 hard cap
inline constexpr double AngularSpeedRange = 50.0;  // rad/s (PROPOSED)

// Quantizes each axis to 16 bits: a range of R over 65,534 steps, so the middle of a symmetric
// range (a body at rest) is exactly code 32,767. Anything not finite sends the middle.
KINETICCORE_API uint16_t QuantizeUnit(double Value, double Min, double Max);
KINETICCORE_API double DequantizeUnit(uint16_t Q, double Min, double Max);

struct FQuantVec { uint16_t X = 0, Y = 0, Z = 0; };
KINETICCORE_API FQuantVec QuantizePosition(const FVec3& P, const FArenaBounds& Bounds);
KINETICCORE_API FVec3 DequantizePosition(const FQuantVec& Q, const FArenaBounds& Bounds);
KINETICCORE_API FQuantVec QuantizeVelocity(const FVec3& V, double Range);
KINETICCORE_API FVec3 DequantizeVelocity(const FQuantVec& Q, double Range);

// A unit quaternion. Smallest-three: the largest component is dropped (its index in 2 bits,
// its sign folded in by negating the whole quaternion) and the other three, each within
// ±1/√2, are sent in 15 bits each: 47 bits in 6 bytes.
struct KINETICCORE_API FQuat
{
	double X = 0.0, Y = 0.0, Z = 0.0, W = 1.0;
	FQuat Normalized() const;
	double Dot(const FQuat& O) const { return X * O.X + Y * O.Y + Z * O.Z + W * O.W; }
};
KINETICCORE_API FQuat AxisAngle(const FVec3& Axis, double Angle);
KINETICCORE_API uint64_t PackQuat(const FQuat& Q);  // 47 bits
KINETICCORE_API FQuat UnpackQuat(uint64_t Bits);

// A replicated physics body: 32 bytes of payload.
struct FBodyState
{
	FVec3 Position;
	FQuat Rotation;
	FVec3 LinearVelocity;
	FVec3 AngularVelocity;
	uint32_t ServerFrame = 0;
	uint8_t Flags = 0;  // Awake, flight body, attached to a line, and so on
};
inline constexpr int BodyPayloadBytes = 32;
inline constexpr int ActorHeaderBytes = 16;  // The engine's, counted in the budget only
KINETICCORE_API std::vector<uint8_t> EncodeBody(const FBodyState& Body, const FArenaBounds& Bounds);
KINETICCORE_API std::optional<FBodyState> DecodeBody(const std::vector<uint8_t>& Bytes, const FArenaBounds& Bounds);

// A line update: 5 bytes. Force as one byte over 0 to 6 kN (the singing pitch and emissive
// need no finer), the mode, five state bits, and the Reel rest length at 1/80 m to 51.2 m.
// Line ids are 12 bits: the server recycles them, and an id past 4,095 does not encode.
enum class ENetLineMode : uint8_t { Slack, Haul, Jack, Hang, Set };
struct FNetLine
{
	uint16_t LineId = 0;        // 12 bits: up to 4,096 lines a match
	double ForceN = 0.0;
	ENetLineMode Mode = ENetLineMode::Slack;
	bool bSlipping = false, bOverspin = false, bSnapping = false, bReeling = false, bSeatedInPlate = false;
	double RestLengthM = 0.0;
};
inline constexpr int LineUpdateBytes = 5;
inline constexpr double LineForceRangeN = 6000.0;
inline constexpr double RestLengthRangeM = 51.2;
inline constexpr uint16_t MaxLineId = 4095;
KINETICCORE_API std::optional<std::vector<uint8_t>> EncodeLine(const FNetLine& Line);
KINETICCORE_API std::optional<FNetLine> DecodeLine(const std::vector<uint8_t>& Bytes);
} // namespace Kinetic
