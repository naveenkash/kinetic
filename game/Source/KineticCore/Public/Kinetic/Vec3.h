// A small double-precision vector. Unreal's FVector is also double precision (large world
// coordinates), so when this library moves into the KineticCore module this type maps onto
// FVector one for one.
#pragma once

#include <cmath>

namespace Kinetic
{
struct FVec3
{
	double X = 0.0;
	double Y = 0.0;
	double Z = 0.0;

	constexpr FVec3() = default;
	constexpr FVec3(double InX, double InY, double InZ) : X(InX), Y(InY), Z(InZ) {}

	constexpr FVec3 operator+(const FVec3& O) const { return {X + O.X, Y + O.Y, Z + O.Z}; }
	constexpr FVec3 operator-(const FVec3& O) const { return {X - O.X, Y - O.Y, Z - O.Z}; }
	constexpr FVec3 operator-() const { return {-X, -Y, -Z}; }
	constexpr FVec3 operator*(double S) const { return {X * S, Y * S, Z * S}; }
	constexpr FVec3 operator/(double S) const { return {X / S, Y / S, Z / S}; }

	constexpr double Dot(const FVec3& O) const { return X * O.X + Y * O.Y + Z * O.Z; }
	double Length() const { return std::sqrt(Dot(*this)); }

	// Unit vector, or zero when the length is below Epsilon (coincident endpoints).
	FVec3 SafeNormal(double Epsilon = 1e-9) const
	{
		const double L = Length();
		return L > Epsilon ? *this / L : FVec3{};
	}
};

constexpr FVec3 operator*(double S, const FVec3& V) { return V * S; }

inline const FVec3 Up{0.0, 0.0, 1.0};
} // namespace Kinetic
