#include "Kinetic/NetPriority.h"

#include "Kinetic/NetCodec.h"

#include <algorithm>

namespace Kinetic
{
namespace
{
constexpr double OwnHz = 60.0;
constexpr double PlayerHz = 30.0;  // Section 2: other players and tier-1 bodies at 30 Hz
constexpr double AirbornePlayerHz = 60.0;
constexpr double LineHz = 30.0;
constexpr double ViewerLineHz = 60.0;
constexpr double Tier1Hz = 30.0;
constexpr double Tier2Hz = 10.0;
constexpr double Tier3Hz = 3.0;
constexpr double ClusterNearHz = 10.0;
constexpr double ClusterFarHz = 2.0;
constexpr double DueSlack = 1e-9;
} // namespace

int NetPriority(const FNetActor& A)
{
	switch (A.Kind)
	{
	case ENetActorKind::Line:
		if (A.bDogInViewerPlate)
		{
			return 10;  // The panic window depends on it
		}
		return A.bOwnedByViewer ? 8 : 5;
	case ENetActorKind::OwnBody:
		return 10;
	case ENetActorKind::OtherPlayer:
		return 6;
	case ENetActorKind::Body:
		if (!A.bAwake)
		{
			return 0;
		}
		if (A.bOnLiveLine)
		{
			return 5;
		}
		if (A.DistanceToViewer <= Tier1RadiusM)
		{
			return 4;
		}
		return A.bInViewerFrustum ? 3 : 1;
	case ENetActorKind::GeometryCluster:
		return A.bAwake ? 2 : 0;
	}
	return 0;
}

EBodyTier TierOf(const FNetActor& B)
{
	if (!B.bAwake)
	{
		return EBodyTier::Dormant;
	}
	if (B.bOnLiveLine || B.DistanceToAnyPlayer <= Tier1RadiusM)
	{
		return EBodyTier::Tier1;
	}
	return B.DistanceToAnyPlayer <= Tier2RadiusM ? EBodyTier::Tier2 : EBodyTier::Tier3;
}

double UpdateHz(const FNetActor& A)
{
	switch (A.Kind)
	{
	case ENetActorKind::Line: return A.bDogInViewerPlate || A.bOwnedByViewer ? ViewerLineHz : LineHz;
	case ENetActorKind::OwnBody: return OwnHz;
	case ENetActorKind::OtherPlayer: return A.bAirborne ? AirbornePlayerHz : PlayerHz;
	case ENetActorKind::GeometryCluster:
		return !A.bAwake ? 0.0 : (A.bNearestClusters ? ClusterNearHz : ClusterFarHz);
	case ENetActorKind::Body:
		switch (TierOf(A))
		{
		case EBodyTier::Tier1: return Tier1Hz;
		case EBodyTier::Tier2: return Tier2Hz;
		case EBodyTier::Tier3: return Tier3Hz;
		case EBodyTier::Dormant: return 0.0;
		}
	}
	return 0.0;
}

int UpdateBytes(const FNetActor& A)
{
	return A.Kind == ENetActorKind::Line ? LineUpdateBytes : BodyPayloadBytes + ActorHeaderBytes;
}

bool NeverShed(const FNetActor& A)
{
	return A.Kind == ENetActorKind::Line || A.Kind == ENetActorKind::OwnBody || A.Kind == ENetActorKind::OtherPlayer;
}

int ShedRank(const FNetActor& A)
{
	if (A.Kind == ENetActorKind::GeometryCluster)
	{
		return A.bNearestClusters ? 1 : 2;
	}
	switch (TierOf(A))
	{
	case EBodyTier::Tier1: return 0;
	case EBodyTier::Tier2: return 1;
	default: return 2;
	}
}

double DownstreamBytesPerSecond(const FBandwidthCase& C)
{
	const double Body = BodyPayloadBytes + ActorHeaderBytes;
	return C.OtherPlayers * Body * AirbornePlayerHz + Body * OwnHz + C.Lines * LineUpdateBytes * LineHz +
	       C.Tier1 * Body * Tier1Hz + C.Tier2 * Body * Tier2Hz + C.Tier3 * Body * Tier3Hz +
	       C.ClustersNear * Body * ClusterNearHz + C.ClustersFar * Body * ClusterFarHz + C.EventsBytesPerSecond +
	       C.PacketsPerSecond * UdpIpHeaderBytes;
}

std::vector<int> FNetScheduler::Tick(const std::vector<FNetActor>& Actors, double Now)
{
	struct FDue
	{
		const FNetActor* Actor;
		bool bNeverShed;
		bool bStarved;
		int Rank;
		int Priority;
		double Overdue;
	};
	std::vector<FDue> Due;
	for (const FNetActor& A : Actors)
	{
		const double Hz = UpdateHz(A);
		const int Priority = NetPriority(A);
		if (Hz <= 0.0 || Priority <= 0)
		{
			continue;
		}
		const auto It = Last.find(A.Id);
		const double Overdue = It == Last.end() ? StarvedSeconds : Now - It->second - 1.0 / Hz;
		if (Overdue >= -DueSlack)
		{
			const bool bNever = NeverShed(A);
			Due.push_back({&A, bNever, !bNever && Overdue >= StarvedSeconds - DueSlack, bNever ? 0 : ShedRank(A), Priority,
			               Overdue});
		}
	}
	std::stable_sort(Due.begin(), Due.end(), [](const FDue& L, const FDue& R) {
		if (L.bNeverShed != R.bNeverShed)
		{
			return L.bNeverShed;
		}
		// Groups: tier 1, then anything else starved, then the rest by shed rank.
		const auto Group = [](const FDue& D) { return D.Rank == 0 ? 0 : D.bStarved ? 1 : 2; };
		if (Group(L) != Group(R))
		{
			return Group(L) < Group(R);
		}
		if (L.bStarved != R.bStarved)
		{
			return L.bStarved;  // Within tier 1, too
		}
		if (L.bStarved)
		{
			return L.Overdue > R.Overdue;
		}
		if (L.Rank != R.Rank)
		{
			return L.Rank < R.Rank;
		}
		return L.Priority != R.Priority ? L.Priority > R.Priority : L.Overdue > R.Overdue;
	});
	double Left = Budget / TickHz - UdpIpHeaderBytes;
	std::vector<int> SentIds;
	for (const FDue& D : Due)
	{
		const int Bytes = UpdateBytes(*D.Actor);
		if (!D.bNeverShed && Bytes > Left)
		{
			continue;  // Stays due; a smaller update further down may still fit
		}
		Left -= Bytes;
		Sent += Bytes;
		SentIds.push_back(D.Actor->Id);
		Last[D.Actor->Id] = Now;
	}
	return SentIds;
}
} // namespace Kinetic
