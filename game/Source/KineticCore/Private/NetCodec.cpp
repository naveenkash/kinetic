#include "Kinetic/NetCodec.h"

#include <algorithm>
#include <cmath>

namespace Kinetic
{
namespace
{
constexpr double MaxQuant = 65534.0;
constexpr int QuatComponentBits = 15;
constexpr double QuatComponentMax = (1 << QuatComponentBits) - 1;
constexpr double InvSqrt2 = 0.70710678118654752;
constexpr int LineIdBits = 12;
constexpr int ForceBits = 8;
constexpr int ModeBits = 3;
constexpr int RestBits = 12;
} // namespace

void FBitWriter::Write(uint64_t Value, int Bits)
{
	for (int I = 0; I < Bits; ++I)
	{
		if (Count % 8 == 0)
		{
			Data.push_back(0);
		}
		if ((Value >> I) & 1u)
		{
			Data.back() = static_cast<uint8_t>(Data.back() | (1u << (Count % 8)));
		}
		++Count;
	}
}

std::optional<uint64_t> FBitReader::Read(int Bits)
{
	if (Cursor + Bits > static_cast<int>(Data.size()) * 8)
	{
		return std::nullopt;
	}
	uint64_t Value = 0;
	for (int I = 0; I < Bits; ++I, ++Cursor)
	{
		if ((Data[static_cast<size_t>(Cursor / 8)] >> (Cursor % 8)) & 1u)
		{
			Value |= uint64_t{1} << I;
		}
	}
	return Value;
}

uint16_t QuantizeUnit(double Value, double Min, double Max)
{
	const double Raw = (Value - Min) / (Max - Min);
	const double T = std::isfinite(Raw) ? std::clamp(Raw, 0.0, 1.0) : 0.5;
	return static_cast<uint16_t>(std::lround(T * MaxQuant));
}

double DequantizeUnit(uint16_t Q, double Min, double Max)
{
	return Min + (Max - Min) * (std::min(static_cast<double>(Q), MaxQuant) / MaxQuant);
}

FQuantVec QuantizePosition(const FVec3& P, const FArenaBounds& B)
{
	return {QuantizeUnit(P.X, B.Min.X, B.Max.X), QuantizeUnit(P.Y, B.Min.Y, B.Max.Y), QuantizeUnit(P.Z, B.Min.Z, B.Max.Z)};
}

FVec3 DequantizePosition(const FQuantVec& Q, const FArenaBounds& B)
{
	return {DequantizeUnit(Q.X, B.Min.X, B.Max.X), DequantizeUnit(Q.Y, B.Min.Y, B.Max.Y),
	        DequantizeUnit(Q.Z, B.Min.Z, B.Max.Z)};
}

FQuantVec QuantizeVelocity(const FVec3& V, double Range)
{
	return {QuantizeUnit(V.X, -Range, Range), QuantizeUnit(V.Y, -Range, Range), QuantizeUnit(V.Z, -Range, Range)};
}

FVec3 DequantizeVelocity(const FQuantVec& Q, double Range)
{
	return {DequantizeUnit(Q.X, -Range, Range), DequantizeUnit(Q.Y, -Range, Range), DequantizeUnit(Q.Z, -Range, Range)};
}

FQuat FQuat::Normalized() const
{
	const double L = std::sqrt(Dot(*this));
	return L > 0.0 ? FQuat{X / L, Y / L, Z / L, W / L} : FQuat{};
}

FQuat AxisAngle(const FVec3& Axis, double Angle)
{
	const FVec3 K = Axis.SafeNormal();
	const double S = std::sin(Angle / 2.0);
	return FQuat{K.X * S, K.Y * S, K.Z * S, std::cos(Angle / 2.0)}.Normalized();
}

uint64_t PackQuat(const FQuat& In)
{
	const FQuat Q = In.Normalized();
	const double C[4] = {Q.X, Q.Y, Q.Z, Q.W};
	int Largest = 0;
	for (int I = 1; I < 4; ++I)
	{
		if (std::abs(C[I]) > std::abs(C[Largest]))
		{
			Largest = I;
		}
	}
	// q and -q are the same rotation: make the dropped component positive.
	const double Sign = C[Largest] < 0.0 ? -1.0 : 1.0;
	uint64_t Bits = static_cast<uint64_t>(Largest);
	int Shift = 2;
	for (int I = 0; I < 4; ++I)
	{
		if (I == Largest)
		{
			continue;
		}
		const double T = std::clamp((C[I] * Sign + InvSqrt2) / (2.0 * InvSqrt2), 0.0, 1.0);
		Bits |= static_cast<uint64_t>(std::lround(T * QuatComponentMax)) << Shift;
		Shift += QuatComponentBits;
	}
	return Bits;
}

FQuat UnpackQuat(uint64_t Bits)
{
	const int Largest = static_cast<int>(Bits & 3u);
	double C[4] = {0, 0, 0, 0};
	double Sum = 0.0;
	int Shift = 2;
	for (int I = 0; I < 4; ++I)
	{
		if (I == Largest)
		{
			continue;
		}
		const double T = static_cast<double>((Bits >> Shift) & static_cast<uint64_t>(QuatComponentMax)) / QuatComponentMax;
		C[I] = T * 2.0 * InvSqrt2 - InvSqrt2;
		Sum += C[I] * C[I];
		Shift += QuatComponentBits;
	}
	C[Largest] = std::sqrt(std::max(0.0, 1.0 - Sum));
	return FQuat{C[0], C[1], C[2], C[3]}.Normalized();
}

std::vector<uint8_t> EncodeBody(const FBodyState& Body, const FArenaBounds& Bounds)
{
	FBitWriter W;
	const FQuantVec P = QuantizePosition(Body.Position, Bounds);
	const FQuantVec V = QuantizeVelocity(Body.LinearVelocity, LinearSpeedRange);
	const FQuantVec A = QuantizeVelocity(Body.AngularVelocity, AngularSpeedRange);
	for (const FQuantVec& Q : {P})
	{
		W.Write(Q.X, 16), W.Write(Q.Y, 16), W.Write(Q.Z, 16);
	}
	W.Write(PackQuat(Body.Rotation), 48);
	for (const FQuantVec& Q : {V, A})
	{
		W.Write(Q.X, 16), W.Write(Q.Y, 16), W.Write(Q.Z, 16);
	}
	W.Write(Body.ServerFrame, 32);
	W.Write(Body.Flags, 8);
	W.Write(0, 24);  // Spare, to the 8-byte frame-and-flags field of section 3
	return W.Bytes();
}

std::optional<FBodyState> DecodeBody(const std::vector<uint8_t>& Bytes, const FArenaBounds& Bounds)
{
	if (Bytes.size() != static_cast<size_t>(BodyPayloadBytes))
	{
		return std::nullopt;
	}
	FBitReader R(Bytes);
	const auto Vec = [&R]() {
		const uint16_t X = static_cast<uint16_t>(*R.Read(16));
		const uint16_t Y = static_cast<uint16_t>(*R.Read(16));
		const uint16_t Z = static_cast<uint16_t>(*R.Read(16));
		return FQuantVec{X, Y, Z};
	};
	FBodyState Out;
	Out.Position = DequantizePosition(Vec(), Bounds);
	Out.Rotation = UnpackQuat(*R.Read(48));
	Out.LinearVelocity = DequantizeVelocity(Vec(), LinearSpeedRange);
	Out.AngularVelocity = DequantizeVelocity(Vec(), AngularSpeedRange);
	Out.ServerFrame = static_cast<uint32_t>(*R.Read(32));
	Out.Flags = static_cast<uint8_t>(*R.Read(8));
	return Out;
}

std::optional<std::vector<uint8_t>> EncodeLine(const FNetLine& Line)
{
	if (Line.LineId > MaxLineId || !std::isfinite(Line.ForceN) || !std::isfinite(Line.RestLengthM))
	{
		return std::nullopt;
	}
	FBitWriter W;
	W.Write(Line.LineId, LineIdBits);
	W.Write(static_cast<uint64_t>(std::lround(std::clamp(Line.ForceN / LineForceRangeN, 0.0, 1.0) * 255.0)), ForceBits);
	W.Write(static_cast<uint64_t>(Line.Mode), ModeBits);
	W.Write(Line.bSlipping, 1), W.Write(Line.bOverspin, 1), W.Write(Line.bSnapping, 1), W.Write(Line.bReeling, 1);
	W.Write(Line.bSeatedInPlate, 1);
	const double Rest = std::clamp(Line.RestLengthM / RestLengthRangeM, 0.0, 1.0) * ((1 << RestBits) - 1);
	W.Write(static_cast<uint64_t>(std::lround(Rest)), RestBits);
	return W.Bytes();
}

std::optional<FNetLine> DecodeLine(const std::vector<uint8_t>& Bytes)
{
	if (Bytes.size() != static_cast<size_t>(LineUpdateBytes))
	{
		return std::nullopt;
	}
	FBitReader R(Bytes);
	FNetLine L;
	L.LineId = static_cast<uint16_t>(*R.Read(LineIdBits));
	L.ForceN = static_cast<double>(*R.Read(ForceBits)) / 255.0 * LineForceRangeN;
	const uint64_t Mode = *R.Read(ModeBits);
	if (Mode > static_cast<uint64_t>(ENetLineMode::Set))
	{
		return std::nullopt;
	}
	L.Mode = static_cast<ENetLineMode>(Mode);
	L.bSlipping = *R.Read(1), L.bOverspin = *R.Read(1), L.bSnapping = *R.Read(1), L.bReeling = *R.Read(1);
	L.bSeatedInPlate = *R.Read(1);
	L.RestLengthM = static_cast<double>(*R.Read(RestBits)) / ((1 << RestBits) - 1) * RestLengthRangeM;
	return L;
}
} // namespace Kinetic
