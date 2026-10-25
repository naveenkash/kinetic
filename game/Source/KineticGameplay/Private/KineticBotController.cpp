#include "KineticBotController.h"

#include "Camera/CameraComponent.h"
#include "Components/BoxComponent.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kinetic/Tables.h"
#include "KineticCharacter.h"
#include "KineticHarness.h"
#include "KineticLadder.h"
#include "KineticRulesSubsystem.h"
#include "NavigationPath.h"
#include "NavigationSystem.h"

using Kinetic::FVec3;

namespace
{
FVec3 ToCore(const FVector& Cm) { return {Cm.X / 100.0, Cm.Y / 100.0, Cm.Z / 100.0}; }
FVector FromCore(const FVec3& M) { return FVector(M.X * 100.0, M.Y * 100.0, M.Z * 100.0); }

constexpr double HalfHeightCm = 95.0;
constexpr double ReachedCm = 60.0;          // A path point is reached this close (flat)
constexpr double StuckSeconds = 2.5;        // No 30 cm of progress in this long: jump and replan
constexpr double ReplanSeconds = 2.0;
constexpr double LadderGiveUpSeconds = 15.0;
constexpr double RetryRigSeconds = 0.3;     // Nothing to rig: look again after this
constexpr int32 AnchorActors = 16;          // The nearest tagged pieces of scenery a bot considers
const FName AnchorTag(TEXT("Kinetic.Anchor"));

FVector VelocityOf(const AKineticCharacter& C) { return C.GetCharacterMovement()->Velocity; }
}  // namespace

AKineticBotController::AKineticBotController()
{
	bSetControlRotationFromPawnOrientation = false;
	bWantsPlayerState = false;
}

void AKineticBotController::UpdateControlRotation(float DeltaTime, bool bUpdatePawn)
{
	APawn* const MyPawn = GetPawn();
	if (MyPawn && bUpdatePawn)
	{
		MyPawn->FaceRotation(GetControlRotation(), DeltaTime);
	}
}

UKineticSpoolWorld* AKineticBotController::Spool() const
{
	return GetWorld() ? GetWorld()->GetSubsystem<UKineticSpoolWorld>() : nullptr;
}

void AKineticBotController::Init(int32 InSlot, uint64 MatchSeed, Kinetic::EHarnessClass InClass)
{
	const UKineticRulesSubsystem* Rules = GEngine ? GEngine->GetEngineSubsystem<UKineticRulesSubsystem>() : nullptr;
	Tables = Rules ? Rules->Tables() : nullptr;
	Slot = InSlot;
	Class = InClass;
	Random.Emplace(MatchSeed, InSlot);
	if (Tables)
	{
		Defense.Emplace(Kinetic::EBotProfile::Backfill, InClass, Kinetic::ECutterVariant::Standard, *Tables,
		                Kinetic::FBotRandom(MatchSeed ^ 0x5DEECE66Dull, InSlot));
	}
	Anchors.Reset();
	for (TActorIterator<AActor> It(GetWorld()); It; ++It)
	{
		if (It->ActorHasTag(AnchorTag))
		{
			Anchors.Add(*It);
		}
	}
	NewRound();
}

void AKineticBotController::NewRound()
{
	if (Defense)
	{
		for (const int32 Id : Defending)
		{
			Defense->Ended(Id);
		}
	}
	Defending.Reset();
	Stage = EStage::Idle;
	StageUntil = 0.0;
	LineId = -1;
	bDrawn = false;
	Route = FRoute();
}

void AKineticBotController::CoolDown(double Now)
{
	Stage = EStage::Cooldown;
	StageUntil = Now + Random->Uniform(1.0, 2.0);
}

bool AKineticBotController::HasFloorAt(const FVector& Point, double FeetZ) const
{
	// Scenery to stand on under the point, within a step of the feet.
	FHitResult Hit;
	const FVector From(Point.X, Point.Y, FeetZ + 50.0);
	const FVector To(Point.X, Point.Y, FeetZ - 150.0);
	return GetWorld()->LineTraceSingleByObjectType(Hit, From, To, FCollisionObjectQueryParams(ECC_WorldStatic));
}

bool AKineticBotController::OverAnEdge(const FVector& From, const FVector& To, double FeetZ) const
{
	for (int32 I = 1; I <= 12; ++I)
	{
		if (!HasFloorAt(FMath::Lerp(From, To, static_cast<double>(I) / 13.0), FeetZ))
		{
			return true;
		}
	}
	return false;
}

Kinetic::FBotView AKineticBotController::ViewOf(const AKineticCharacter& Me, const TArray<FKineticRiggerView>& Riggers) const
{
	const UKineticSpoolWorld* World = Spool();
	const FVector At = Me.GetActorLocation();
	Kinetic::FBotView V;
	V.Slot = Slot;
	V.Class = Class;
	V.Position = ToCore(At);
	V.bLineLive = World->ActiveLine(&Me) != nullptr;
	V.bLineOnSelf = World->LinesOnPlayer(&Me).Num() > 0;
	for (const FKineticRiggerView& R : Riggers)
	{
		if (R.Character.Get() == &Me)
		{
			V.Team = R.Team;
		}
	}
	const AKineticCharacter* Nearest = nullptr;
	for (const FKineticRiggerView& R : Riggers)
	{
		const AKineticCharacter* O = R.Character.Get();
		if (!O || O == &Me)
		{
			continue;
		}
		V.Others.push_back({R.Slot, R.Team, ToCore(O->GetActorLocation()), ToCore(VelocityOf(*O)), R.bStanding});
		if (R.Team != V.Team && R.bStanding &&
		    (!Nearest || FVector::Dist(O->GetActorLocation(), At) < FVector::Dist(Nearest->GetActorLocation(), At)))
		{
			Nearest = O;
		}
	}
	// Anchors: of the nearest tagged scenery, the point nearest the enemy (a Haul drags them
	// into it) and the point nearest the bot (a wall at its back, for a Slingshot).
	const double Reach = Tables->Harness(Class).LineLength * 100.0;
	TArray<FBox> Boxes;
	for (const TWeakObjectPtr<AActor>& A : Anchors)
	{
		if (const AActor* Actor = A.Get())
		{
			const FBox Box = Actor->GetComponentsBoundingBox();
			if (Box.ComputeSquaredDistanceToPoint(At) <= Reach * Reach)
			{
				Boxes.Add(Box);
			}
		}
	}
	Boxes.Sort([&At](const FBox& L, const FBox& R) { return L.ComputeSquaredDistanceToPoint(At) < R.ComputeSquaredDistanceToPoint(At); });
	Boxes.SetNum(FMath::Min(Boxes.Num(), AnchorActors));
	const auto PointOn = [](const FBox& Box, const FVector& Near) {
		FVector P = Box.GetClosestPointTo(Near);
		const double Lo = Box.Min.Z + 30.0;
		P.Z = FMath::Clamp(Near.Z, Lo, FMath::Max(Lo, Box.Max.Z - 30.0));
		return P;
	};
	for (const FBox& Box : Boxes)
	{
		if (Nearest)
		{
			const FVector Enemy = Nearest->GetActorLocation();
			const FVector P = PointOn(Box, Enemy);
			V.Anchors.push_back({ToCore(P), OverAnEdge(Enemy, P, Enemy.Z - HalfHeightCm)});
		}
		V.Anchors.push_back({ToCore(PointOn(Box, At)), false});
	}
	return V;
}

FVector AKineticBotController::AimPoint(const AKineticCharacter& Me, const FVector& Target, const FVector& TargetVelocity) const
{
	// Lead the target over the Dog's flight, and hold over for its drop.
	const double Speed = Tables->Harness(Class).DogSpeed * 100.0;
	const double Time = FVector::Dist(Target, Me.Camera->GetComponentLocation()) / Speed;
	const double G = Tables->Globals().Gravity * 100.0;
	return Target + TargetVelocity * Time + FVector(0.0, 0.0, 0.5 * G * Time * Time);
}

void AKineticBotController::FaceToward(const AKineticCharacter& Me, const FVector& Point)
{
	if (AController* C = Me.GetController())
	{
		const FRotator R = (Point - Me.Camera->GetComponentLocation()).Rotation();
		C->SetControlRotation(FRotator(R.Pitch, R.Yaw, 0.0));
	}
}

void AKineticBotController::Think(AKineticCharacter& Me, const TArray<FKineticRiggerView>& Riggers,
                                  const TArray<FKineticSpoolEvent>& Events)
{
	UKineticSpoolWorld* World = Spool();
	if (!World || !Tables || !Defense)
	{
		return;
	}
	const double Now = World->Now();
	Me.Move(FVector2D::ZeroVector);  // Nothing pressed unless it walks this tick
	if (!World->IsStanding(&Me))
	{
		return;
	}
	LastThought = Now;
	for (const FKineticSpoolEvent& E : Events)
	{
		if (E.Kind == EKineticSpoolEventKind::Seat && E.Target.Get() == &Me && E.Owner.Get() != &Me)
		{
			Defense->Seated(E.LineId, Now);
			Defending.AddUnique(E.LineId);
		}
		if (E.Kind == EKineticSpoolEventKind::LineEnded)
		{
			if (Defending.Remove(E.LineId) > 0)
			{
				Defense->Ended(E.LineId);
			}
			if (E.LineId == LineId && Stage != EStage::Cooldown)
			{
				CoolDown(Now);
			}
		}
	}

	// Defense first: the cut reflex on its own plates.
	const Kinetic::FBotDefense::FActions Act = Defense->Tick(Now);
	const bool bDraw = !Defense->Plans().empty();
	if (bDraw != bDrawn)
	{
		World->SetCutterDrawn(&Me, bDraw);
		bDrawn = bDraw;
	}
	if (Act.Swing)
	{
		if (const FKineticLine* L = World->FindLine(*Act.Swing))
		{
			// Face along the line from its Dog in the plate, a metre out, and swing.
			const bool bMineA = L->A.Seat == EKineticSeat::Plate && L->A.Player.Get() == &Me;
			const FVector Plate = World->DrawnEndPosition(*L, bMineA);
			const FVector Far = World->DrawnEndPosition(*L, !bMineA);
			FaceToward(Me, Plate + (Far - Plate).GetSafeNormal() * 100.0);
			World->Swing(&Me);
		}
	}
	if (Defending.Num() > 0)
	{
		return;  // Cutting free comes before anything else
	}

	const FKineticLine* Mine = World->ActiveLine(&Me);
	const bool bOnTheMove = Me.IsClimbing() || Route.Step != ELadderStep::None || !Me.GetCharacterMovement()->IsMovingOnGround();
	switch (Stage)
	{
	case EStage::Idle:
	{
		if (Now < StageUntil || bOnTheMove || !World->LinesAllowed())
		{
			Walk(Me, Riggers);
			break;
		}
		Intent = Kinetic::ChooseRig(ViewOf(Me, Riggers), *Tables);
		if (Intent.Rig == Kinetic::EBotRig::Slingshot)
		{
			// The tree's own check: never launch over a drop. The Kick's V carries about V²/g.
			const double V = Tables->Harness(Class).KickLaunchSpeedV;
			const FVector At = Me.GetActorLocation();
			const FVector Dir = FVector(At - FromCore(Intent.DogB)).GetSafeNormal2D();
			if (OverAnEdge(At, At + Dir * (V * V / Tables->Globals().Gravity * 100.0), At.Z - HalfHeightCm))
			{
				Intent.Rig = Kinetic::EBotRig::None;
			}
		}
		if (Intent.Rig == Kinetic::EBotRig::None)
		{
			StageUntil = Now + RetryRigSeconds;
			Walk(Me, Riggers);
			break;
		}
		World->SelectMode(&Me, Intent.Mode == Kinetic::ELineMode::Set ? Kinetic::ELineMode::Hang : Intent.Mode);
		Stage = EStage::AimA;
		StageUntil = Now + AimSeconds;
		break;
	}
	case EStage::AimA:
	{
		FVector TargetV = FVector::ZeroVector;
		for (const FKineticRiggerView& R : Riggers)
		{
			if (R.Slot == Intent.TargetSlot && R.Character.IsValid())
			{
				TargetV = VelocityOf(*R.Character);
				if (!Intent.bSelfDogA && Intent.Rig != Kinetic::EBotRig::Clothesline)
				{
					Intent.DogA = ToCore(R.Character->GetActorLocation());  // Track the target while aiming
				}
			}
		}
		if (!Intent.bSelfDogA)
		{
			const FVector Err(Random->Uniform(-AimErrorCm, AimErrorCm), Random->Uniform(-AimErrorCm, AimErrorCm),
			                  Random->Uniform(-AimErrorCm, AimErrorCm));
			const bool bMoving = Intent.Rig != Kinetic::EBotRig::Clothesline;
			FaceToward(Me, AimPoint(Me, FromCore(Intent.DogA), bMoving ? TargetV : FVector::ZeroVector) + Err);
		}
		if (Now >= StageUntil)
		{
			if (Intent.bSelfDogA)
			{
				World->SelfDogNext(&Me);
			}
			else
			{
				World->FireDog(&Me);
			}
			++Fired;
			Stage = EStage::WaitA;
			StageUntil = Now + SeatTimeoutSeconds;
		}
		break;
	}
	case EStage::WaitA:
		if (Mine)
		{
			LineId = Mine->Id;
		}
		if (Mine && Mine->A.Seat != EKineticSeat::None)
		{
			Stage = EStage::AimB;
			StageUntil = Now + AimSeconds;
		}
		else if (Now >= StageUntil || !Mine)
		{
			World->Release(&Me);
			CoolDown(Now);
		}
		break;
	case EStage::AimB:
		FaceToward(Me, AimPoint(Me, FromCore(Intent.DogB), FVector::ZeroVector));
		if (Now >= StageUntil)
		{
			World->FireDog(&Me);
			++Fired;
			Stage = EStage::WaitB;
			StageUntil = Now + SeatTimeoutSeconds;
		}
		break;
	case EStage::WaitB:
		if (Mine && Mine->IsLive())
		{
			Stage = EStage::Holding;
			StageUntil = Now + HoldSeconds;
			if (Intent.bReleaseAfterSet && Mine->State.Mode == Kinetic::ELineMode::Hang)
			{
				World->Release(&Me);  // The Clothesline: Hang, then a set line
				CoolDown(Now);
			}
		}
		else if (Now >= StageUntil || !Mine)
		{
			World->Release(&Me);
			CoolDown(Now);
		}
		break;
	case EStage::Holding:
	{
		bool bTargetDown = false;
		for (const FKineticRiggerView& R : Riggers)
		{
			bTargetDown = bTargetDown || (R.Slot == Intent.TargetSlot && !R.bStanding);
		}
		if (!Mine || Now >= StageUntil || bTargetDown)
		{
			if (Mine)
			{
				World->Release(&Me);
			}
			CoolDown(Now);
		}
		break;
	}
	case EStage::Cooldown:
		Walk(Me, Riggers);
		if (Now >= StageUntil)
		{
			Stage = EStage::Idle;
			StageUntil = Now;
		}
		break;
	}
}

void AKineticBotController::Walk(AKineticCharacter& Me, const TArray<FKineticRiggerView>& Riggers)
{
	const FVector At = Me.GetActorLocation();
	const AKineticCharacter* Target = nullptr;
	int32 MyTeam = -1;
	for (const FKineticRiggerView& R : Riggers)
	{
		MyTeam = R.Character.Get() == &Me ? R.Team : MyTeam;
	}
	for (const FKineticRiggerView& R : Riggers)
	{
		const AKineticCharacter* O = R.Character.Get();
		if (O && R.Team != MyTeam && R.bStanding &&
		    (!Target || FVector::Dist(O->GetActorLocation(), At) < FVector::Dist(Target->GetActorLocation(), At)))
		{
			Target = O;
		}
	}
	if (!Target)
	{
		return;
	}
	const double Range = Tables->Harness(Class).LineLength * 100.0 * ApproachShare;
	const bool bClimbing = Me.IsClimbing() || Route.Step != ELadderStep::None;
	if (!bClimbing && FVector::Dist(Target->GetActorLocation(), At) <= Range && Me.GetCharacterMovement()->IsMovingOnGround())
	{
		FaceToward(Me, Target->GetActorLocation());
		return;  // Close enough to rig
	}
	FollowRoute(Me, Target->GetActorLocation());
}

void AKineticBotController::FollowRoute(AKineticCharacter& Me, const FVector& Goal)
{
	if (Climb(Me))
	{
		return;
	}
	const double Now = Spool()->Now();
	const FVector At = Me.GetActorLocation();
	const double Feet = At.Z - HalfHeightCm;
	const auto Head = [this, &Me](const FVector& Toward) {
		const FVector D = FVector(Toward - Me.GetActorLocation()).GetSafeNormal2D();
		if (AController* C = Me.GetController())
		{
			C->SetControlRotation(FRotator(0.0, FMath::RadiansToDegrees(FMath::Atan2(D.Y, D.X)), 0.0));
		}
		// Walk, not run, up to anyone ahead or to an edge (it stops in a step, not a metre and a half).
		const bool bEdge = !HasFloorAt(Me.GetActorLocation() + D * 200.0, Me.GetActorLocation().Z - HalfHeightCm);
		Me.Move(FVector2D(0.0, bEdge || CloseBehindSomeone(Me, D) ? 0.3 : 1.0));
	};
	const bool bReplan = (Route.Next >= Route.Points.Num() && Now - Route.PlannedAt > 0.5) || Now - Route.PlannedAt > ReplanSeconds ||
	                     FVector::Dist(Goal, Route.Goal) > 300.0;
	if (bReplan && Me.GetCharacterMovement()->IsMovingOnGround())
	{
		Route.Goal = Goal;
		Route.PlannedAt = Now;
		// The goal on the navmesh: under a target who is in the air or on a ladder.
		FVector To = Goal;
		const UNavigationSystemV1* Nav = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
		FNavLocation OnMesh;
		if (Nav && Nav->ProjectPointToNavigation(Goal, OnMesh, FVector(300.0, 300.0, 1200.0)))
		{
			To = OnMesh.Location;
		}
		const UNavigationPath* Path = UNavigationSystemV1::FindPathToLocationSynchronously(GetWorld(), At, To, &Me);
		if (Path && Path->IsValid())
		{
			Route.Points = Path->PathPoints;
			Route.Next = 1;
			Route.ProgressAt = Now;
			Route.BestDistance = TNumericLimits<double>::Max();
		}
		// No path from here: the last one stands, if any.
	}
	if (Route.Next >= Route.Points.Num())
	{
		// No path (no navmesh here): straight at it, never off a drop.
		const FVector Dir = FVector(Goal - At).GetSafeNormal2D();
		if (HasFloorAt(At + Dir * 120.0, Feet))
		{
			Head(Goal);
		}
		return;
	}
	const FVector P = Route.Points[Route.Next];
	const FVector Prev = Route.Points[Route.Next - 1];
	// A ladder's link: the segment rises or drops more than a storey over a short run.
	if (FMath::Abs(P.Z - Prev.Z) > 120.0 && FVector::Dist2D(P, Prev) < 300.0)
	{
		const AKineticLadder* Best = nullptr;
		const FVector Mid = (P + Prev) * 0.5;
		for (TActorIterator<AKineticLadder> It(GetWorld()); It; ++It)
		{
			if (FVector::Dist(It->GetActorLocation(), Mid) < 400.0 &&
			    (!Best || FVector::Dist(It->GetActorLocation(), Mid) < FVector::Dist(Best->GetActorLocation(), Mid)))
			{
				Best = *It;
			}
		}
		if (Best)
		{
			Route.Ladder = Best;
			Route.Step = P.Z > Prev.Z ? ELadderStep::Up : ELadderStep::OffTheTop;
			Route.LadderSince = Now;
			if (Climb(Me))
			{
				return;
			}
		}
	}
	const double D = FVector::Dist2D(At, P);
	if (D < ReachedCm && FMath::Abs(Feet - P.Z) < 150.0)
	{
		++Route.Next;
		Route.BestDistance = TNumericLimits<double>::Max();
		Route.ProgressAt = Now;
		return;
	}
	if (D < Route.BestDistance - 30.0)
	{
		Route.BestDistance = D;
		Route.ProgressAt = Now;
	}
	else if (Now - Route.ProgressAt > StuckSeconds)
	{
		Me.JumpPressed();  // Caught on something: hop it, and plan again
		Route.PlannedAt = -100.0;
		Route.ProgressAt = Now;
	}
	Head(P);
}

bool AKineticBotController::CloseBehindSomeone(const AKineticCharacter& Me, const FVector& Heading) const
{
	// Another rigger just ahead: walk up slowly rather than run into them.
	for (TActorIterator<AKineticCharacter> It(GetWorld()); It; ++It)
	{
		const FVector To = It->GetActorLocation() - Me.GetActorLocation();
		if (*It != &Me && !It->IsHidden() && To.Size2D() < 220.0 && FMath::Abs(To.Z) < 150.0 &&
		    FVector::DotProduct(To.GetSafeNormal2D(), Heading) > 0.5)
		{
			return true;
		}
	}
	return false;
}

bool AKineticBotController::Climb(AKineticCharacter& Me)
{
	if (Me.IsClimbing() && Route.Step == ELadderStep::None)
	{
		// On a ladder its route does not take (a reset, a knock): off at the nearer end.
		Route.Ladder = Me.GetLadder();
		const double Feet = Me.GetActorLocation().Z - HalfHeightCm;
		Route.Step = Route.Ladder->TopZ() - Feet < Feet - Route.Ladder->Foot().Z ? ELadderStep::Up : ELadderStep::Down;
		Route.LadderSince = Spool()->Now();
	}
	const AKineticLadder* L = Route.Ladder.Get();
	Me.SetTakesLadders(Route.Step != ELadderStep::None && L);  // Only the ladder it means to take
	if (Route.Step == ELadderStep::None || !L)
	{
		Route.Step = ELadderStep::None;
		return false;
	}
	const double Now = Spool()->Now();
	const FVector At = Me.GetActorLocation();
	const double Feet = At.Z - HalfHeightCm;
	const double IntoYaw = FMath::RadiansToDegrees(FMath::Atan2(L->IntoWall.Y, L->IntoWall.X));
	const auto Face = [&Me](double Yaw) {
		if (AController* C = Me.GetController())
		{
			C->SetControlRotation(FRotator(0.0, Yaw, 0.0));
		}
	};
	const auto Toward = [&](const FVector& P) {
		Face(FMath::RadiansToDegrees(FMath::Atan2(P.Y - At.Y, P.X - At.X)));
		const bool bSlow = CloseBehindSomeone(Me, FVector(P - At).GetSafeNormal2D()) || FVector::Dist2D(At, P) < 150.0;
		Me.Move(FVector2D(0.0, bSlow ? 0.3 : 1.0));
	};
	// Off the ladder: past the link's far end on the path, and plan on from where it got off.
	const auto Done = [this, &Me]() {
		Me.SetTakesLadders(false);
		Route.Step = ELadderStep::None;
		Route.Ladder = nullptr;
		Route.Next = FMath::Min(Route.Next + 1, Route.Points.Num());
		Route.PlannedAt = -100.0;
		return false;
	};
	// One climber at a time: wait while someone else is on the rungs, or another bot came to
	// this ladder first (the lower slot, if together).
	const auto Busy = [&]() {
		for (TActorIterator<AKineticCharacter> It(GetWorld()); It; ++It)
		{
			if (*It != &Me && It->GetLadder() == L)
			{
				return true;
			}
		}
		for (TActorIterator<AKineticBotController> It(GetWorld()); It; ++It)
		{
			const AKineticBotController* Other = *It;
			if (Other != this && Other->Route.Step != ELadderStep::None && Other->Route.Ladder.Get() == L &&
			    Other->LastThought >= Now - 0.2 &&  // A bot out of the round holds no place
			    (Other->Route.LadderSince < Route.LadderSince || (Other->Route.LadderSince == Route.LadderSince && Other->Slot < Slot)))
			{
				return true;
			}
		}
		return false;
	};
	if (!Me.IsClimbing() && Now - Route.LadderSince > LadderGiveUpSeconds)
	{
		return Done();
	}
	switch (Route.Step)
	{
	case ELadderStep::Up:
		if (Me.IsClimbing())
		{
			Face(IntoYaw);
			Me.Move(FVector2D(0.0, 1.0));
			return true;
		}
		if (Feet > L->TopZ() - 30.0)
		{
			return Done();  // Stepped off at the top
		}
		if (FVector::Dist2D(At, L->Foot()) > 70.0)
		{
			Toward(L->Foot());
			return true;
		}
		if (!Busy())
		{
			Face(IntoYaw);
			Me.Move(FVector2D(0.0, 1.0));  // Into the rungs: the character takes hold
		}
		return true;
	case ELadderStep::OffTheTop:
		if (Me.IsClimbing())
		{
			Route.Step = ELadderStep::Down;
			Face(IntoYaw);
			Me.Move(FVector2D(0.0, -1.0));
			return true;
		}
		if (Feet < L->TopZ() - 150.0)
		{
			return Done();  // Already down
		}
		if (Busy())
		{
			// Wait back from the top, clear of where a climber steps off.
			const FVector Clear = L->StepOff() + FVector(L->IntoWall.X, L->IntoWall.Y, 0.0).GetSafeNormal() * 200.0;
			if (FVector::Dist2D(At, Clear) > 60.0)
			{
				Toward(Clear);
			}
			return true;
		}
		if (FVector::Dist2D(At, L->StepOff()) > 80.0)
		{
			Toward(L->StepOff());
			return true;
		}
		Face(IntoYaw);
		Me.Move(FVector2D(0.0, -1.0));  // Back over the top, facing the wall: the character mounts the rungs
		return true;
	case ELadderStep::Down:
		if (Me.IsClimbing())
		{
			Face(IntoYaw);
			Me.Move(FVector2D(0.0, -1.0));
			return true;
		}
		return Done();  // Let go at the bottom
	default:
		return Done();
	}
}
