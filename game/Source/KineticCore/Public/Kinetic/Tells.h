// Tells (bible/05 section 11, the readability contract; docs/tech/kinematics-and-force-volumes.md
// section 3): every lethal system has an audio tell, a visual tell and a minimum lead. Timeline
// tells run from the server clock on every client with no RPC; event tells (break-free, slip,
// a volatile armed, a rated load's groan) are sent by the server, which holds the consequence
// back until the lead has passed.
#pragma once

#include <optional>
#include <string>
#include <vector>

namespace Kinetic
{
enum class ETellSystem : uint8_t
{
	DogSeated,
	Overspin,
	HangSlip,
	BreakFree,
	RatedLoadMoving,
	HaulerApproach,
	GantryDrop,
	MidMatchEvent,
	ForceVolume,
	RingOut,
	VolatileArmed,
};

const char* ToString(ETellSystem System);

// Section 11's minimum lead, s. A timeline may lead by more, never less.
double MinimumLead(ETellSystem System);

// One timeline tell: a cue (audio and visual, each with its own lead) before a payload at
// TriggerTime, either once or every Period from TriggerTime until Until.
struct FTellEvent
{
	std::string Name;
	ETellSystem System = ETellSystem::MidMatchEvent;
	double TriggerTime = 0.0;  // s, match time
	double Period = 0.0;       // 0: once
	double Until = 1e300;
	double LeadAudio = 0.0;
	double LeadVisual = 0.0;
};

// Every payload time of an event in [From, To), ascending.
std::vector<double> Occurrences(const FTellEvent& Event, double From, double To);

// The Tell.LeadTimes check (3.3): one problem per event whose audio or visual lead undercuts
// its system's minimum, or whose occurrences overlap their own cue (period shorter than lead).
std::vector<std::string> TimelineProblems(const std::vector<FTellEvent>& Timeline);

enum class ECueKind : uint8_t { Audio, Visual };

struct FCue
{
	size_t EventIndex = 0;
	double PayloadTime = 0.0;
	ECueKind Kind = ECueKind::Audio;
};

// The client scheduler (3.2): call Advance with the estimated server time each frame; it fires
// every cue whose time (payload minus lead) has been crossed, once each. A cue whose payload is
// already past (the client joined late, or time jumped) is dropped, a klaxon after the train
// being noise; a zero-lead cue fires as its moment is crossed. Events added later (a one-off
// event replicated at round start, 3.1) join as a late client would: their cues already due
// fire on the next Advance, and nothing else fires again. Time never runs backward here
// (FServerClock guarantees it; earlier times are ignored).
class FTellScheduler
{
public:
	explicit FTellScheduler(std::vector<FTellEvent> InTimeline);

	size_t AddEvent(FTellEvent Event);  // Returns its index for FCue::EventIndex
	std::vector<FCue> Advance(double ServerTime);

private:
	std::vector<FTellEvent> Timeline;
	std::vector<bool> Joined;  // Per event: has it been through an Advance yet
	double Last = -1e300;
	bool bStarted = false;
};

// The server's hold on event-tell consequences (3.2): a rated load's consequence fires no
// sooner than 1.0 s after its groan was sent, a volatile no sooner than 0.3 s after its hiss.
// Each source (one rated load, one cylinder) is gated on its latest cue, by Id; a new cue
// re-arms it. Clear a source once its consequence has fired, and Reset at round reset.
class FTellGate
{
public:
	void Cue(ETellSystem System, int Id, double ServerTime);
	// False until the source was cued and its system's minimum lead has passed.
	bool MayFire(ETellSystem System, int Id, double ServerTime) const;
	std::optional<double> FiresAt(ETellSystem System, int Id) const;
	void Clear(ETellSystem System, int Id);
	void Reset() { Cued.clear(); }

private:
	struct FEntry
	{
		ETellSystem System;
		int Id;
		double CuedAt;
	};
	std::vector<FEntry> Cued;
};

// The mid-match event's payload time (05 9.2): 1:15 remaining in a Scrap round, 4:00 in Claim,
// 2:00 in each Haul half, 3:00 in Free Scrap, on the round clock, which runs from the start of
// the Drop (game-modes 2.2). Its siren leads by 5 s.
enum class EMatchMode : uint8_t { Scrap, Claim, Haul, FreeScrap };
double MidMatchEventTime(EMatchMode Mode, double RoundStart, double RoundSeconds);
double RoundSecondsOf(EMatchMode Mode);  // The round clock (a Haul half), Drop included, s
} // namespace Kinetic
