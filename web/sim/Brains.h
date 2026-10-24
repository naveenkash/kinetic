// Bot controllers for the browser client: what a behavior tree will do in the engine. Each
// bot presses the same buttons a player does (FPlayerInput), decided by KineticCore's Bots.h:
// FBotDefense for lines on its own plates, ChooseRig for what to rig next.
//
// The tree's own choices (PROPOSED for the prototype):
// - A bot walks toward the nearest standing enemy until it is within 70% of its line length,
//   crossing the trench by the nearer bridge, and stops short of any drop.
// - It aims with a lead for the target's velocity and the Dog's drop, plus up to 0.3 m of
//   error, and takes 0.35 s to aim each Dog.
// - A Haul or Jack is held up to 3 s; a Clothesline is Hung and released at once.
// - After a rig it waits 1.0 to 2.0 s before the next.
#pragma once

#include "World.h"

#include "Kinetic/Bots.h"

#include <optional>
#include <vector>

namespace Kinetic::Web
{
class FBotBrain
{
public:
	FBotBrain(int InPlayer, uint64_t MatchSeed, const FKineticTables& Tables, EHarnessClass Class);

	// The buttons to press this tick. Events are the world's since the last tick.
	FPlayerInput Think(const FWorld& World, const std::vector<FWorldEvent>& Events);
	void Reset();  // A new round: plans and rigs dropped

	static constexpr double AimSeconds = 0.35;
	static constexpr double HoldSeconds = 3.0;
	static constexpr double SeatTimeoutSeconds = 1.5;
	static constexpr double AimErrorM = 0.3;
	static constexpr double ApproachShare = 0.7;

private:
	enum class EStage : uint8_t { Idle, AimA, WaitA, AimB, WaitB, Holding, Cooldown };

	FVec3 AimPoint(const FWorld& World, const FVec3& Target, const FVec3& TargetVelocity) const;
	FBotView ViewOf(const FWorld& World) const;
	void FaceToward(const FWorld& World, const FVec3& Point, FPlayerInput& In) const;
	void Walk(const FWorld& World, FPlayerInput& In) const;
	bool OverAnEdge(const FWorld& World, const FVec3& From, const FVec3& To) const;

	int Player;
	const FKineticTables& Tables;
	FBotRandom Random;
	FBotDefense Defense;
	EStage Stage = EStage::Idle;
	double StageUntil = 0.0;
	FBotIntent Intent;
	int LineId = -1;
	std::vector<int> Defending;  // Line ids on its plates the defense is planning for
	std::optional<int> SwingAt;   // A line to face and swing at this tick
};
} // namespace Kinetic::Web
