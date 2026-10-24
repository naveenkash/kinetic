#include "World.h"

namespace Kinetic::Web
{
namespace
{
FSolid Box(const FVec3& Min, const FVec3& Max, std::string Tag, ESurface Surface = ESurface::Hard)
{
	return {Min, Max, Surface, std::move(Tag)};
}

// A shipping container standing on the floor, its long side along x or y.
FSolid Container(double X, double Y, bool bAlongX, double Floor = 0.0)
{
	const double HalfLong = 3.0, HalfShort = 1.25, Height = 2.6;
	const double HX = bAlongX ? HalfLong : HalfShort;
	const double HY = bAlongX ? HalfShort : HalfLong;
	return Box({X - HX, Y - HY, Floor}, {X + HX, Y + HY, Floor + Height}, "container", ESurface::Scrap);
}
} // namespace

// The Yard in grey-box (PROPOSED layout for the prototype, after arenas/the-yard): the Floor
// is two yards 30 m by 40 m split by a 4 m trench with two bridges. Three containers and a
// stack on each side, a doorway of two containers, pylons at the trench edges and pylons on
// footings past the east and west edges. A Haul toward a pylon across the trench or past an
// edge is the arena's ring-out.
FArena MakeYard()
{
	FArena A;
	A.Name = "The Yard (grey-box)";
	// The Floor.
	A.Solids.push_back(Box({-32, -20, -1}, {-2, 20, 0}, "floor"));
	A.Solids.push_back(Box({2, -20, -1}, {32, 20, 0}, "floor"));
	A.Solids.push_back(Box({-2, -15, -1}, {2, -11, 0}, "bridge"));
	A.Solids.push_back(Box({-2, 11, -1}, {2, 15, 0}, "bridge"));
	// The trench bed, out of reach: falling in is a ring-out.
	A.Solids.push_back(Box({-2, -20, -9}, {2, 20, -8}, "trench"));
	// Low walls along the long sides, with gaps that open on the drop.
	for (const double Side : {-1.0, 1.0})
	{
		A.Solids.push_back(Box({-32, Side * 20 - 0.3, 0}, {-14, Side * 20 + 0.3, 1.2}, "wall"));
		A.Solids.push_back(Box({14, Side * 20 - 0.3, 0}, {32, Side * 20 + 0.3, 1.2}, "wall"));
	}
	// Containers: mirrored west and east.
	for (const double S : {-1.0, 1.0})
	{
		A.Solids.push_back(Container(S * 18, -6, true));
		A.Solids.push_back(Container(S * 24, 9, false));
		A.Solids.push_back(Container(S * 10, 4, true));
		A.Solids.push_back(Container(S * 10, 4, true, 2.6));  // A stack, two high
		// A doorway: two containers end to end with a 2 m gap.
		A.Solids.push_back(Container(S * 12, -17, false));
		A.Solids.push_back(Container(S * 12, -9, false));
		A.Doorways.push_back({{S * 12, -14, 1.2}, {S * 12, -12, 1.2}});
		// Trench pylons, 0.6 m square, on the near bank.
		for (const double Y : {-5.0, 5.0})
		{
			const double X = S * 2.6;
			A.Solids.push_back(Box({X - 0.3, Y - 0.3, 0}, {X + 0.3, Y + 0.3, 4}, "post", ESurface::Hard));
			A.Anchors.push_back({{X - S * 0.3, Y, 1.5}});  // The face toward the trench
		}
		// Edge pylons on footings 2 m past the end of the Floor.
		for (const double Y : {-8.0, 8.0})
		{
			const double X = S * 34.3;
			A.Solids.push_back(Box({X - 0.3, Y - 0.3, -10}, {X + 0.3, Y + 0.3, 3}, "post", ESurface::Hard));
			A.Anchors.push_back({{X - S * 0.3, Y, 1.5}});
		}
		// Anchors on container faces for Hauls into the steel.
		A.Anchors.push_back({{S * 18, -6 + S * 1.25, 1.5}});
		A.Anchors.push_back({{S * 24 - S * 1.25, 9, 1.5}});
		A.Anchors.push_back({{S * 10, 4 - 1.25, 1.5}});
		// Loose crates and a stacked pair.
		A.Crates.push_back({{S * 15, 12, 0.5}, {0.5, 0.5, 0.5}, 60.0, EPlacement::Loose});
		A.Crates.push_back({{S * 6, -4, 0.5}, {0.5, 0.5, 0.5}, 60.0, EPlacement::Loose});
		A.Crates.push_back({{S * 20, 2, 0.6}, {0.6, 0.6, 0.6}, 180.0, EPlacement::Loose});
		A.Crates.push_back({{S * 27, -12, 0.5}, {0.5, 0.5, 0.5}, 60.0, EPlacement::Stack});
		A.Crates.push_back({{S * 27, -12, 1.5}, {0.5, 0.5, 0.5}, 60.0, EPlacement::Stack});
	}
	// Spawns: team sides at the far ends.
	for (const double Y : {-6.0, 0.0, 6.0})
	{
		A.Spawns[0].push_back({-28, Y, 0});
		A.Spawns[1].push_back({28, Y, 0});
	}
	A.KillZ = -6.0;
	return A;
}
} // namespace Kinetic::Web
