// Who gets which update, how often (docs/tech/networking.md sections 2, 3 and 5): the priority
// of each actor for one viewer, the tier and update rate of each body, the per-client budget,
// and the per-connection scheduler that fits a tick's updates into the budget, shedding tier-3
// and then tier-2 bodies first and never lines or players.
#pragma once

#include "Kinetic/Api.h"

#include "Kinetic/Vec3.h"

#include <cstdint>
#include <unordered_map>
#include <vector>

namespace Kinetic
{
enum class ENetActorKind : uint8_t { Line, OwnBody, OtherPlayer, Body, GeometryCluster };

// What a connection's viewer knows about one actor this tick.
struct FNetActor
{
	int Id = 0;
	ENetActorKind Kind = ENetActorKind::Body;
	bool bAwake = true;
	bool bDogInViewerPlate = false;  // Lines: a Dog seated in this viewer's Catch Plate
	bool bOwnedByViewer = false;     // Lines
	bool bOnLiveLine = false;        // Bodies: attached to any live line
	double DistanceToViewer = 1e9;   // m
	double DistanceToAnyPlayer = 1e9;
	bool bInViewerFrustum = false;
	bool bNearestClusters = false;   // GC level-1 clusters: among the nearest 36
	bool bAirborne = false;          // Other players: on their physics flight body
};

// Section 5's table; 0 means dormant (not sent).
KINETICCORE_API int NetPriority(const FNetActor& Actor);

// Section 2's rates and section 3's tiers.
enum class EBodyTier : uint8_t { Tier1, Tier2, Tier3, Dormant };
inline constexpr double Tier1RadiusM = 15.0;
inline constexpr double Tier2RadiusM = 40.0;  // PROPOSED: section 3 names the tier, not its edge
KINETICCORE_API EBodyTier TierOf(const FNetActor& Body);
// Other players at 30 Hz (section 2), or 60 Hz while airborne: section 3's worst case counts
// their flight bodies at the owner's rate. Lines at 30 Hz, except the viewer's own lines and
// any line with a Dog in the viewer's plate, at 60 Hz (section 2's "lines on their body").
KINETICCORE_API double UpdateHz(const FNetActor& Actor);  // 0 for dormant
KINETICCORE_API int UpdateBytes(const FNetActor& Actor);  // Payload plus the actor header
KINETICCORE_API bool NeverShed(const FNetActor& Actor);   // Lines and players
// The order in which the rest are shed, last shed first: tier-1 bodies (0), tier-2 bodies and
// the nearest clusters (1), tier-3 bodies and the other clusters (2).
KINETICCORE_API int ShedRank(const FNetActor& Actor);

// Section 3's worst case, from its own counts, for checking the arithmetic.
struct FBandwidthCase
{
	int OtherPlayers = 5;
	int Lines = 16;
	int Tier1 = 24, Tier2 = 40, Tier3 = 32;
	int ClustersNear = 36, ClustersFar = 36;
	double EventsBytesPerSecond = 1500.0;
	double PacketsPerSecond = 60.0;
};
KINETICCORE_API double DownstreamBytesPerSecond(const FBandwidthCase& Case);

inline constexpr double MaxClientRateBytes = 100000.0;  // MaxClientRate
inline constexpr double UdpIpHeaderBytes = 28.0;

// One connection's sender: each tick it is offered every actor and picks what to send. An
// actor is due when its update interval has elapsed. Due actors that must never be shed go
// first, whatever the budget. Then tier-1 bodies. Then anything else starved (a second or more
// overdue, PROPOSED), the most overdue first, so nothing awake goes unsent for long while the
// budget covers tier 1. Then the rest by shed rank, then priority, then the most overdue, while
// the tick's budget (MaxClientRate over the tick rate, less headers) lasts. Starved actors go
// first within tier 1 too. Anything not sent stays due and is offered again next tick.
class KINETICCORE_API FNetScheduler
{
public:
	explicit FNetScheduler(double InTickHz = 60.0, double InBudgetBytesPerSecond = MaxClientRateBytes)
	    : TickHz(InTickHz), Budget(InBudgetBytesPerSecond)
	{
	}

	// The ids sent this tick, in send order.
	std::vector<int> Tick(const std::vector<FNetActor>& Actors, double Now);
	double BytesSent() const { return Sent; }
	void Forget(int Id) { Last.erase(Id); }  // The actor was destroyed
	static constexpr double StarvedSeconds = 1.0;

private:
	double TickHz;
	double Budget;
	std::unordered_map<int, double> Last;  // Id to when it was last sent
	double Sent = 0.0;
};
} // namespace Kinetic
