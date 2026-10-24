#include "Kinetic/Tells.h"

#include <algorithm>
#include <cmath>

namespace Kinetic
{
namespace
{
constexpr double TimeEpsilon = 1e-9;
constexpr double ScrapRoundSeconds = 150.0;  // 2:30 (game-modes 1)
constexpr double ClaimRoundSeconds = 480.0;  // 8:00
constexpr double HaulHalfSeconds = 240.0;    // 4:00 per half
constexpr double FreeScrapSeconds = 360.0;   // 6:00
} // namespace

const char* ToString(ETellSystem System)
{
	switch (System)
	{
	case ETellSystem::DogSeated: return "dog_seated";
	case ETellSystem::Overspin: return "overspin";
	case ETellSystem::HangSlip: return "hang_slip";
	case ETellSystem::BreakFree: return "break_free";
	case ETellSystem::RatedLoadMoving: return "rated_load_moving";
	case ETellSystem::HaulerApproach: return "hauler_approach";
	case ETellSystem::GantryDrop: return "gantry_drop";
	case ETellSystem::MidMatchEvent: return "mid_match_event";
	case ETellSystem::ForceVolume: return "force_volume";
	case ETellSystem::RingOut: return "ring_out";
	case ETellSystem::VolatileArmed: return "volatile_armed";
	}
	return "unknown";
}

double MinimumLead(ETellSystem System)
{
	switch (System)
	{
	case ETellSystem::DogSeated: return 0.0;  // The window starts here
	case ETellSystem::Overspin: return 0.8;
	case ETellSystem::HangSlip: return 0.3;  // Per 0.5 m of slip
	case ETellSystem::BreakFree: return 0.2;
	case ETellSystem::RatedLoadMoving: return 1.0;
	case ETellSystem::HaulerApproach: return 3.0;
	case ETellSystem::GantryDrop: return 2.0;
	case ETellSystem::MidMatchEvent: return 5.0;
	case ETellSystem::ForceVolume: return 1.0;
	case ETellSystem::RingOut: return 0.5;
	case ETellSystem::VolatileArmed: return 0.3;
	}
	return 0.0;
}

std::vector<double> Occurrences(const FTellEvent& Event, double From, double To)
{
	std::vector<double> Out;
	if (Event.Period <= 0.0)
	{
		if (Event.TriggerTime >= From && Event.TriggerTime < To && Event.TriggerTime < Event.Until)
		{
			Out.push_back(Event.TriggerTime);
		}
		return Out;
	}
	const double FirstIndex = std::max(0.0, std::ceil((From - Event.TriggerTime) / Event.Period - TimeEpsilon));
	for (double T = Event.TriggerTime + FirstIndex * Event.Period; T < To && T < Event.Until; T += Event.Period)
	{
		Out.push_back(T);
	}
	return Out;
}

std::vector<std::string> TimelineProblems(const std::vector<FTellEvent>& Timeline)
{
	std::vector<std::string> Problems;
	for (const FTellEvent& E : Timeline)
	{
		const double Minimum = MinimumLead(E.System);
		if (E.LeadAudio + TimeEpsilon < Minimum || E.LeadVisual + TimeEpsilon < Minimum)
		{
			Problems.push_back(E.Name + ": lead under the " + ToString(E.System) + " minimum");
		}
		if (E.Period > 0.0 && E.Period < std::max(E.LeadAudio, E.LeadVisual))
		{
			Problems.push_back(E.Name + ": repeats before its own cue");
		}
	}
	return Problems;
}

FTellScheduler::FTellScheduler(std::vector<FTellEvent> InTimeline)
    : Timeline(std::move(InTimeline)), Joined(Timeline.size(), false)
{
}

size_t FTellScheduler::AddEvent(FTellEvent Event)
{
	Timeline.push_back(std::move(Event));
	Joined.push_back(false);
	return Timeline.size() - 1;
}

std::vector<FCue> FTellScheduler::Advance(double ServerTime)
{
	std::vector<FCue> Fired;
	if (bStarted && ServerTime <= Last)
	{
		return Fired;
	}
	for (size_t I = 0; I < Timeline.size(); ++I)
	{
		const FTellEvent& E = Timeline[I];
		const bool bFirst = !Joined[I];
		// An event added mid-match still sees zero-lead moments since the last frame.
		const double From = bStarted ? Last : ServerTime;
		const double MaxLead = std::max(E.LeadAudio, E.LeadVisual);
		for (const double Payload : Occurrences(E, From - TimeEpsilon, ServerTime + MaxLead + TimeEpsilon))
		{
			for (const auto& [Kind, Lead] : {std::pair{ECueKind::Audio, E.LeadAudio}, std::pair{ECueKind::Visual, E.LeadVisual}})
			{
				const double CueTime = Payload - Lead;
				// A zero-lead cue is its moment: fire it as it is crossed. A leading cue whose
				// payload has passed is late and dropped.
				const bool bLate = Lead > 0.0 && Payload <= ServerTime;
				const bool bDue = CueTime <= ServerTime + TimeEpsilon;
				// A late join fires leading cues already due; a zero-lead cue fires once crossed since
				// the last frame (or, for a scheduler's first frame, at that frame).
				const double Since = bStarted ? Last : ServerTime - TimeEpsilon;
				const bool bNew = bFirst && Lead > 0.0 ? true : CueTime > Since + (bStarted ? TimeEpsilon : 0.0);
				if (!bLate && bDue && bNew)
				{
					Fired.push_back({I, Payload, Kind});
				}
			}
		}
		Joined[I] = true;
	}
	Last = ServerTime;
	bStarted = true;
	return Fired;
}

void FTellGate::Cue(ETellSystem System, int Id, double ServerTime)
{
	Clear(System, Id);  // The latest cue is the one that counts
	Cued.push_back({System, Id, ServerTime});
}

void FTellGate::Clear(ETellSystem System, int Id)
{
	std::erase_if(Cued, [&](const FEntry& E) { return E.System == System && E.Id == Id; });
}

std::optional<double> FTellGate::FiresAt(ETellSystem System, int Id) const
{
	for (const FEntry& E : Cued)
	{
		if (E.System == System && E.Id == Id)
		{
			return E.CuedAt + MinimumLead(System);
		}
	}
	return std::nullopt;
}

bool FTellGate::MayFire(ETellSystem System, int Id, double ServerTime) const
{
	const std::optional<double> At = FiresAt(System, Id);
	return At && ServerTime + TimeEpsilon >= *At;
}

double RoundSecondsOf(EMatchMode Mode)
{
	switch (Mode)
	{
	case EMatchMode::Scrap: return ScrapRoundSeconds;
	case EMatchMode::Claim: return ClaimRoundSeconds;
	case EMatchMode::Haul: return HaulHalfSeconds;
	case EMatchMode::FreeScrap: return FreeScrapSeconds;
	}
	return 0.0;
}

double MidMatchEventTime(EMatchMode Mode, double RoundStart, double RoundSeconds)
{
	double Remaining = 0.0;
	switch (Mode)
	{
	case EMatchMode::Scrap: Remaining = 75.0; break;
	case EMatchMode::Claim: Remaining = 240.0; break;
	case EMatchMode::Haul: Remaining = 120.0; break;
	case EMatchMode::FreeScrap: Remaining = 180.0; break;
	}
	return RoundStart + RoundSeconds - Remaining;
}
} // namespace Kinetic
