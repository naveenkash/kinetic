#include "KineticSpoolWorld.h"

#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerStart.h"
#include "Kinetic/Cutter.h"
#include "Kinetic/HangSolver.h"
#include "Kinetic/Tables.h"
#include "KineticCharacter.h"
#include "KineticKinematic.h"
#include "KineticLadder.h"
#include "KineticLineRenderer.h"
#include "KineticProp.h"
#include "KineticRulesSubsystem.h"

DEFINE_LOG_CATEGORY_STATIC(LogKineticSpool, Log, All);

using Kinetic::ELineMode;
using Kinetic::FVec3;

namespace
{
constexpr double BraceStillSpeed = 0.5;  // m/s (PROPOSED)
constexpr double BraceSeconds = 0.3;     // bible/05 section 5.4
constexpr double StaggerSeconds = 0.5;   // PROPOSED
constexpr double FlipSeconds = 1.2;
constexpr double LineStepOverCm = 30.0;  // A Hang line lower than this above the feet is stepped over (PROPOSED)
constexpr double PlayerRadiusCm = 40.0;
constexpr double PlayerHalfHeightCm = 95.0;

FVec3 ToCore(const FVector& Cm) { return {Cm.X / 100.0, Cm.Y / 100.0, Cm.Z / 100.0}; }
FVector FromCore(const FVec3& M) { return FVector(M.X * 100.0, M.Y * 100.0, M.Z * 100.0); }
FVec3 Flat(const FVec3& V) { return {V.X, V.Y, 0.0}; }
bool IsActive(ELineMode Mode) { return Mode == ELineMode::Haul || Mode == ELineMode::Jack; }
bool IsHang(ELineMode Mode) { return Mode == ELineMode::Hang || Mode == ELineMode::Set; }

double MassOf(const AKineticCharacter* Player, const Kinetic::FKineticTables& Tables)
{
	return Tables.Harness(KineticHarness::ToCore(Player->Harness)).MassKg;
}
}  // namespace

const Kinetic::FKineticTables* UKineticSpoolWorld::Tables() const
{
	const UKineticRulesSubsystem* Rules = GEngine ? GEngine->GetEngineSubsystem<UKineticRulesSubsystem>() : nullptr;
	return Rules ? Rules->Tables() : nullptr;
}

bool UKineticSpoolWorld::DoesSupportWorldType(const EWorldType::Type WorldType) const
{
	return WorldType == EWorldType::Game || WorldType == EWorldType::PIE;
}

TStatId UKineticSpoolWorld::GetStatId() const { RETURN_QUICK_DECLARE_CYCLE_STAT(UKineticSpoolWorld, STATGROUP_Tickables); }

void UKineticSpoolWorld::OnWorldBeginPlay(UWorld& InWorld)
{
	Super::OnWorldBeginPlay(InWorld);
	FActorSpawnParameters Params;
	Params.ObjectFlags |= RF_Transient;
	Renderer = InWorld.SpawnActor<AKineticLineRenderer>(Params);
}

FKineticSpoolPlayer& UKineticSpoolWorld::StateOf(AKineticCharacter* Player)
{
	FKineticSpoolPlayer& P = Players.FindOrAdd(Player);
	if (!P.Monitor && Tables())
	{
		P.Monitor = MakeShared<Kinetic::FImpactMonitor>(*Tables());
	}
	return P;
}

const FKineticSpoolPlayer* UKineticSpoolWorld::PlayerState(const AKineticCharacter* Player) const
{
	return Players.Find(TWeakObjectPtr<AKineticCharacter>(const_cast<AKineticCharacter*>(Player)));
}

FVector UKineticSpoolWorld::PlatePosition(const AKineticCharacter* Player, Kinetic::EPlate Plate)
{
	// The plate offsets from the body centre (05 section 5.5; PROPOSED), in cm.
	const FRotator Yaw(0.0, Player->GetActorRotation().Yaw, 0.0);
	const FVector F = FRotationMatrix(Yaw).GetUnitAxis(EAxis::X);
	const FVector R = FRotationMatrix(Yaw).GetUnitAxis(EAxis::Y);
	const FVector C = Player->GetActorLocation();
	switch (Plate)
	{
	case Kinetic::EPlate::Chest: return C + F * 32.0 + FVector(0, 0, 35.0);
	case Kinetic::EPlate::Back: return C - F * 32.0 + FVector(0, 0, 35.0);
	case Kinetic::EPlate::LeftShoulder: return C - R * 38.0 + FVector(0, 0, 60.0);
	case Kinetic::EPlate::RightShoulder: return C + R * 38.0 + FVector(0, 0, 60.0);
	case Kinetic::EPlate::None: break;
	}
	return C;
}

FVector UKineticSpoolWorld::EndPosition(const FKineticLineEnd& End) const
{
	switch (End.Seat)
	{
	case EKineticSeat::Static: return End.Point;
	case EKineticSeat::Prop: return End.Prop.IsValid() ? End.Prop->Mesh->GetCenterOfMass() : End.Point;
	case EKineticSeat::Kinematic:
		return End.Kinematic.IsValid() ? End.Kinematic->GetActorTransform().TransformPosition(End.Point) : End.Point;
	case EKineticSeat::Plate:
	case EKineticSeat::OwnPlate: return End.Player.IsValid() ? PlatePosition(End.Player.Get(), End.Plate) : End.Point;
	case EKineticSeat::None: break;
	}
	return End.Point;
}

FVector UKineticSpoolWorld::BodyPosition(const FKineticLineEnd& End) const
{
	switch (End.Seat)
	{
	case EKineticSeat::Prop: return End.Prop.IsValid() ? End.Prop->Mesh->GetCenterOfMass() : End.Point;
	case EKineticSeat::Plate:
	case EKineticSeat::OwnPlate: return End.Player.IsValid() ? End.Player->GetActorLocation() : End.Point;
	case EKineticSeat::Kinematic: return EndPosition(End);
	default: return End.Point;
	}
}

void UKineticSpoolWorld::Engage(FKineticLine& Line, ELineMode Mode)
{
	// Haul and Jack act between the Dogs (a plate, the point a Dog seated at); a Hang or Set
	// line's rest length is taken between what the Hang solve moves: body centres.
	const bool bHangFrame = IsHang(Mode);
	const FVector A = bHangFrame ? BodyPosition(Line.A) : DogPosition(Line.A);
	const FVector B = bHangFrame ? BodyPosition(Line.B) : DogPosition(Line.B);
	Kinetic::EngageMode(Line.State, Mode, Clock, ToCore(A), ToCore(B), Tables()->Globals());
}

FVector UKineticSpoolWorld::DogPosition(const FKineticLineEnd& End) const
{
	if (End.Seat == EKineticSeat::Prop && End.Prop.IsValid())
	{
		return End.Prop->GetActorTransform().TransformPosition(End.Point);
	}
	return EndPosition(End);
}

FVector UKineticSpoolWorld::DrawnEndPosition(const FKineticLine& Line, bool bSideA) const
{
	const TOptional<FKineticDog>& Dog = bSideA ? Line.FlyingA : Line.FlyingB;
	if (Dog.IsSet())
	{
		return Dog->Position;
	}
	const FKineticLineEnd& End = bSideA ? Line.A : Line.B;
	if (End.Seat == EKineticSeat::Prop && End.Prop.IsValid())
	{
		return End.Prop->GetActorTransform().TransformPosition(End.Point);
	}
	return EndPosition(End);
}

int32 UKineticSpoolWorld::ActiveLineOf(const AKineticCharacter* Player) const
{
	for (int32 I = 0; I < LineList.Num(); ++I)
	{
		if (LineList[I].Owner.Get() == Player && !LineList[I].bSet)
		{
			return I;
		}
	}
	return INDEX_NONE;
}

void UKineticSpoolWorld::EndLine(int32 Index, EKineticLineEnded Why)
{
	UE_LOG(LogKineticSpool, Verbose, TEXT("line %d ended (%d)"), LineList[Index].Id, static_cast<int>(Why));
	FKineticSpoolEvent E;
	E.Kind = EKineticSpoolEventKind::LineEnded;
	E.LineId = LineList[Index].Id;
	E.Owner = LineList[Index].Owner;
	E.Why = Why;
	Emit(MoveTemp(E));
	LineList.RemoveAt(Index);
}

void UKineticSpoolWorld::Emit(FKineticSpoolEvent Event)
{
	if (Events.Num() >= 1024)
	{
		Events.RemoveAt(0);  // Nobody is reading (the walk-the-arena mode)
	}
	Events.Add(MoveTemp(Event));
}

TArray<FKineticSpoolEvent> UKineticSpoolWorld::TakeEvents()
{
	TArray<FKineticSpoolEvent> Out = MoveTemp(Events);
	Events.Reset();
	return Out;
}

bool UKineticSpoolWorld::IsStanding(const AKineticCharacter* Player) const
{
	const FKineticSpoolPlayer* P = PlayerState(Player);
	return !P || P->bStanding;
}

TArray<int32> UKineticSpoolWorld::LinesOnPlayer(const AKineticCharacter* Player) const
{
	TArray<int32> Out;
	for (const FKineticLine& L : LineList)
	{
		const bool bA = L.A.Seat == EKineticSeat::Plate && L.A.Player.Get() == Player;
		const bool bB = L.B.Seat == EKineticSeat::Plate && L.B.Player.Get() == Player;
		if (bA || bB)
		{
			Out.Add(L.Id);
		}
	}
	return Out;
}

const FKineticLine* UKineticSpoolWorld::FindLine(int32 Id) const
{
	return LineList.FindByPredicate([Id](const FKineticLine& L) { return L.Id == Id; });
}

const FKineticLine* UKineticSpoolWorld::ActiveLine(const AKineticCharacter* Player) const
{
	const int32 I = ActiveLineOf(Player);
	return I == INDEX_NONE ? nullptr : &LineList[I];
}

double UKineticSpoolWorld::CutterWarmup(const AKineticCharacter* Player) const
{
	const Kinetic::FKineticTables* T = Tables();
	return T && Player ? Kinetic::CutterWarmupSeconds(Kinetic::ECutterVariant::Standard, KineticHarness::ToCore(Player->Harness), *T)
	                   : 0.5;
}

int32 UKineticSpoolWorld::SeverLines(const AKineticKinematic& Kinematic)
{
	if (!Kinematic.HasSeverBox() || !Kinematic.IsActive())
	{
		return 0;
	}
	const Kinetic::FPose From = Kinematic.PoseBefore();
	const Kinetic::FPose To = Kinematic.PoseNow();
	int32 Severed = 0;
	for (int32 I = LineList.Num(); I-- > 0;)
	{
		const FKineticLine& L = LineList[I];
		const auto OnIt = [&Kinematic](const FKineticLineEnd& E) {
			return E.Seat == EKineticSeat::Kinematic && E.Kinematic.Get() == &Kinematic;
		};
		if (!L.IsLive() || OnIt(L.A) || OnIt(L.B))
		{
			continue;  // A line with a Dog in it is carried, not cut (the Caboose)
		}
		// Four samples per step (kinematics 1.3).
		if (Kinetic::SweptSever(Kinematic.SeverBox(), From, To, 4, ToCore(DogPosition(L.A)), ToCore(DogPosition(L.B))))
		{
			EndLine(I, EKineticLineEnded::Severed);
			++Severed;
		}
	}
	return Severed;
}

void UKineticSpoolWorld::Hit(AKineticCharacter* Player, Kinetic::EOutcome Outcome, Kinetic::EStallCause Cause, double StaggerFor)
{
	if (!Player || !IsStanding(Player) || Outcome == Kinetic::EOutcome::None)
	{
		return;
	}
	if (Outcome == Kinetic::EOutcome::Stall)
	{
		Stall(Player, Cause);
		return;
	}
	FKineticSpoolPlayer& P = StateOf(Player);
	const double Stun = Outcome == Kinetic::EOutcome::Flip ? FlipSeconds : StaggerFor > 0.0 ? StaggerFor : StaggerSeconds;
	P.StunnedUntil = FMath::Max(P.StunnedUntil, Clock + Stun);
	P.bBraced = false;
	P.LastOutcome = Outcome;
	P.LastOutcomeAt = Clock;
	Player->SetStunned(true);
}

void UKineticSpoolWorld::ClearLines()
{
	while (LineList.Num() > 0)
	{
		EndLine(LineList.Num() - 1, EKineticLineEnded::Released);
	}
}

void UKineticSpoolWorld::FireDog(AKineticCharacter* Player)
{
	const Kinetic::FKineticTables* T = Tables();
	if (!T || !Player || !bLinesAllowed || Player->IsStunned() || !IsStanding(Player))
	{
		return;  // Before Live, staggered, or out of the round
	}
	FKineticSpoolPlayer& P = StateOf(Player);
	const Kinetic::FHarnessTuning& H = T->Harness(KineticHarness::ToCore(Player->Harness));
	int32 Active = ActiveLineOf(Player);
	if (Active != INDEX_NONE)
	{
		const FKineticLine& L = LineList[Active];
		if (L.FlyingB.IsSet() || L.B.Seat != EKineticSeat::None)
		{
			// A third press releases the current line and launches a new Dog A (05 3.1).
			ReleaseActive(Player);
			Active = INDEX_NONE;
		}
	}
	const bool bSelf = P.bSelfDogNext;
	P.bSelfDogNext = false;
	const FVector Dir = Player->GetControlRotation().Vector();
	FKineticDog Dog;
	Dog.Position = Player->Camera->GetComponentLocation() + Dir * 50.0;
	Dog.Velocity = Dir * (H.DogSpeed * 100.0);
	FKineticLineEnd Self;
	Self.Seat = EKineticSeat::OwnPlate;
	Self.Player = Player;
	Self.Plate = Kinetic::EPlate::Chest;
	if (Active == INDEX_NONE)
	{
		FKineticLine L;
		L.Id = NextLineId++;
		L.Owner = Player;
		L.Params = Kinetic::FLineParams::FromHarness(H);
		L.TensionN = H.TensionN;
		L.Preselect = P.Preselect;
		L.CreatedAt = Clock;
		if (bSelf)
		{
			L.A = Self;
		}
		else
		{
			L.FlyingA = Dog;
		}
		LineList.Add(MoveTemp(L));
		return;
	}
	FKineticLine& L = LineList[Active];
	if (bSelf)
	{
		L.B = Self;
	}
	else
	{
		L.FlyingB = Dog;
	}
}

void UKineticSpoolWorld::SelfDogNext(AKineticCharacter* Player)
{
	if (Player && bLinesAllowed)
	{
		StateOf(Player).bSelfDogNext = true;
		FireDog(Player);
	}
}

void UKineticSpoolWorld::SelectMode(AKineticCharacter* Player, ELineMode Mode)
{
	const Kinetic::FKineticTables* T = Tables();
	if (!T || !Player)
	{
		return;
	}
	StateOf(Player).Preselect = Mode;
	const int32 Active = ActiveLineOf(Player);
	if (Active == INDEX_NONE)
	{
		return;
	}
	FKineticLine& L = LineList[Active];
	L.Preselect = Mode;
	if (L.IsLive() && L.State.Mode != Mode)
	{
		L.PendingMode = Mode;
		L.PendingAt = Clock + T->Harness(KineticHarness::ToCore(Player->Harness)).ModeSwitch;
	}
	else if (L.IsLive())
	{
		L.PendingAt = -1.0;  // Back to the mode it is in: the switch is called off
	}
}

void UKineticSpoolWorld::Release(AKineticCharacter* Player) { ReleaseActive(Player); }

void UKineticSpoolWorld::ReleaseActive(AKineticCharacter* Player)
{
	const Kinetic::FKineticTables* T = Tables();
	const int32 Active = ActiveLineOf(Player);
	if (!T || Active == INDEX_NONE)
	{
		return;
	}
	if (!(LineList[Active].IsLive() && LineList[Active].State.Mode == ELineMode::Hang))
	{
		EndLine(Active, EKineticLineEnded::Released);
		return;
	}
	const Kinetic::FHarnessTuning& H = T->Harness(KineticHarness::ToCore(Player->Harness));
	int32 SetCount = 0;
	for (const FKineticLine& O : LineList)
	{
		SetCount += O.Owner.Get() == Player && O.bSet ? 1 : 0;
	}
	if (SetCount >= H.SetLines)
	{
		for (int32 J = 0; J < LineList.Num(); ++J)
		{
			if (LineList[J].Owner.Get() == Player && LineList[J].bSet)
			{
				EndLine(J, EKineticLineEnded::Expired);  // Setting another retires the oldest (05 3.6)
				break;
			}
		}
	}
	FKineticLine& Kept = LineList[ActiveLineOf(Player)];
	Kept.bSet = true;
	Kept.SetExpiresAt = Clock + H.SetLineLife;
	Kept.PendingAt = -1.0;
	Engage(Kept, ELineMode::Set);
}

void UKineticSpoolWorld::SetBrace(AKineticCharacter* Player, bool bHeld)
{
	if (Player)
	{
		StateOf(Player).bBraceInput = bHeld;
	}
}

void UKineticSpoolWorld::SetCutterDrawn(AKineticCharacter* Player, bool bDrawn)
{
	if (!Player)
	{
		return;
	}
	FKineticSpoolPlayer& P = StateOf(Player);
	if (bDrawn && !P.bDrawn)
	{
		P.DrawnAt = Clock;
	}
	P.bDrawn = bDrawn;
}

void UKineticSpoolWorld::Swing(AKineticCharacter* Player)
{
	const Kinetic::FKineticTables* T = Tables();
	if (!T || !Player || !bLinesAllowed || Player->IsStunned() || !IsStanding(Player))
	{
		return;
	}
	FKineticSpoolPlayer& P = StateOf(Player);
	const Kinetic::FCutterTuning& Cutter = T->Cutter(Kinetic::ECutterVariant::Standard);
	const double Warmup = Kinetic::CutterWarmupSeconds(Kinetic::ECutterVariant::Standard,
	                                                   KineticHarness::ToCore(Player->Harness), *T);
	if (!P.bDrawn || Clock < P.DrawnAt + Warmup - 1e-9 || Clock < P.NextSwingAt - 1e-9)
	{
		return;  // Not drawn, still warming up, or recovering from the last swing (05 4.1)
	}
	P.NextSwingAt = Clock + Cutter.Swing + Cutter.Recovery;
	const FVector Forward = Player->GetControlRotation().Vector();  // A direction: not scaled to metres
	const Kinetic::FCutQuery Q{ToCore(Player->Camera->GetComponentLocation()), FVec3{Forward.X, Forward.Y, Forward.Z},
	                           Cutter.Reach, Cutter.ConeDeg};
	double Best = TNumericLimits<double>::Max();
	int32 BestLine = INDEX_NONE;
	for (int32 I = 0; I < LineList.Num(); ++I)
	{
		const FKineticLine& L = LineList[I];
		if (!L.IsLive())
		{
			continue;
		}
		const FVec3 A = ToCore(DogPosition(L.A));
		const FVec3 B = ToCore(DogPosition(L.B));
		const std::optional<double> Cut = Kinetic::FindCutPoint(Q, A, B);
		if (Cut)
		{
			const double D = (A + (B - A) * *Cut - Q.Eye).Length();
			if (D < Best)
			{
				Best = D;
				BestLine = I;
			}
		}
	}
	if (BestLine != INDEX_NONE)
	{
		EndLine(BestLine, EKineticLineEnded::Cut);
	}
}

TOptional<FKineticLineEnd> UKineticSpoolWorld::SweepDog(const AKineticCharacter* Owner, const FVector& From, const FVector& To,
                                                        FVector& Hit) const
{
	const Kinetic::FKineticTables* T = Tables();
	const float Radius = T ? static_cast<float>(T->Globals().DogRadius * 100.0) : 8.0f;
	FCollisionQueryParams Params(SCENE_QUERY_STAT(KineticDog), false, Owner);
	if (Renderer)
	{
		Params.AddIgnoredActor(Renderer);
	}
	TArray<FHitResult> Hits;
	GetWorld()->SweepMultiByObjectType(Hits, From, To, FQuat::Identity, FCollisionObjectQueryParams::AllObjects,
	                                   FCollisionShape::MakeSphere(Radius), Params);
	for (const FHitResult& H : Hits)
	{
		AActor* Actor = H.GetActor();
		if (Actor && Actor->IsA<AKineticLadder>())
		{
			continue;  // A ladder's climbing volume is not a surface
		}
		if (H.GetComponent() && H.GetComponent()->GetCollisionEnabled() == ECollisionEnabled::NoCollision)
		{
			continue;
		}
		Hit = H.bStartPenetrating ? From : FVector(H.Location);
		FKineticLineEnd End;
		if (AKineticCharacter* Player = Cast<AKineticCharacter>(Actor))
		{
			// A Dog that reaches the body anywhere seats in the plate nearest the impact (05 5.5).
			End.Seat = EKineticSeat::Plate;
			End.Player = Player;
			double Nearest = TNumericLimits<double>::Max();
			for (const Kinetic::EPlate Plate : {Kinetic::EPlate::Chest, Kinetic::EPlate::Back, Kinetic::EPlate::LeftShoulder,
			                                    Kinetic::EPlate::RightShoulder})
			{
				const double D = FVector::Dist(PlatePosition(Player, Plate), Hit);
				if (D < Nearest)
				{
					Nearest = D;
					End.Plate = Plate;
				}
			}
		}
		else if (AKineticProp* Prop = Cast<AKineticProp>(Actor))
		{
			End.Seat = EKineticSeat::Prop;
			End.Prop = Prop;
			End.Point = Prop->GetActorTransform().InverseTransformPosition(H.ImpactPoint);
		}
		else if (AKineticKinematic* Kinematic = Cast<AKineticKinematic>(Actor))
		{
			End.Seat = EKineticSeat::Kinematic;  // A Hauler, the Gantry: the Dog rides with it
			End.Kinematic = Kinematic;
			End.Point = Kinematic->GetActorTransform().InverseTransformPosition(H.ImpactPoint);
		}
		else
		{
			End.Seat = EKineticSeat::Static;
			End.Point = H.ImpactPoint;
			End.Actor = Actor;
		}
		return End;
	}
	return {};
}

void UKineticSpoolWorld::UpdateDogs()
{
	const Kinetic::FKineticTables* T = Tables();
	const double Gravity = T->Globals().Gravity * 100.0;
	for (int32 I = 0; I < LineList.Num();)
	{
		FKineticLine& L = LineList[I];
		bool bEnded = false;
		for (const bool bSideA : {true, false})
		{
			TOptional<FKineticDog>& Dog = bSideA ? L.FlyingA : L.FlyingB;
			if (!Dog.IsSet())
			{
				continue;
			}
			const FVector From = Dog->Position;
			Dog->Velocity.Z -= Gravity * Dt;
			const FVector To = From + Dog->Velocity * Dt;
			Dog->Travelled += FVector::Dist(From, To) / 100.0;
			FVector Hit;
			const TOptional<FKineticLineEnd> Seat = SweepDog(L.Owner.Get(), From, To, Hit);
			if (Seat.IsSet())
			{
				(bSideA ? L.A : L.B) = Seat.GetValue();
				Dog.Reset();
				if (Seat->Seat == EKineticSeat::Plate)
				{
					FKineticSpoolEvent E;
					E.Kind = EKineticSpoolEventKind::Seat;
					E.LineId = L.Id;
					E.Owner = L.Owner;
					E.Target = Seat->Player;
					Emit(MoveTemp(E));
				}
				continue;
			}
			Dog->Position = To;
			// Past the class line length: it falls harmless (05 3.1).
			if (Dog->Travelled > L.Params.LineLength)
			{
				bEnded = true;
				break;
			}
		}
		if (bEnded)
		{
			EndLine(I, EKineticLineEnded::Missed);
			continue;
		}
		++I;
	}
}

void UKineticSpoolWorld::UpdateLines()
{
	const Kinetic::FSpoolGlobals& G = Tables()->Globals();
	for (int32 I = 0; I < LineList.Num();)
	{
		FKineticLine& L = LineList[I];
		const auto Down = [this](const TWeakObjectPtr<AKineticCharacter>& Player) {
			const FKineticSpoolPlayer* P = Player.IsValid() ? Players.Find(Player) : nullptr;
			return !Player.IsValid() || (P && !P->bStanding);
		};
		const auto Gone = [&](const FKineticLineEnd& E) {
			return (E.Seat == EKineticSeat::Prop && (!E.Prop.IsValid() || E.Prop->IsSpent())) ||
			       (E.Seat == EKineticSeat::Kinematic && (!E.Kinematic.IsValid() || !E.Kinematic->IsActive())) ||
			       ((E.Seat == EKineticSeat::Plate || E.Seat == EKineticSeat::OwnPlate) && Down(E.Player));
		};
		// A Stalled body takes its lines with it; a set line outlives its owner (05 3.6).
		if ((Down(L.Owner) && !L.bSet) || Gone(L.A) || Gone(L.B))
		{
			EndLine(I, EKineticLineEnded::Released);
			continue;
		}
		if (L.bSet && Clock >= L.SetExpiresAt)
		{
			EndLine(I, EKineticLineEnded::Expired);
			continue;
		}
		const bool bAOnly = L.A.Seat != EKineticSeat::None && !L.FlyingB.IsSet() && L.B.Seat == EKineticSeat::None;
		if (bAOnly && Clock - L.CreatedAt > G.SlackTimeout)
		{
			EndLine(I, EKineticLineEnded::Released);  // Four seconds to fire B, or A recalls (05 3.1)
			continue;
		}
		if (L.IsLive() && L.State.Mode == ELineMode::Slack && L.PendingAt < 0.0 && !L.bSet)
		{
			Engage(L, L.Preselect);
		}
		if (L.PendingAt >= 0.0 && Clock >= L.PendingAt)
		{
			Engage(L, L.PendingMode);
			L.PendingAt = -1.0;
		}
		++I;
	}
}

Kinetic::FLineEndpoint UKineticSpoolWorld::EndpointOf(const FKineticLineEnd& End, bool bHeld) const
{
	const Kinetic::FKineticTables& T = *Tables();
	Kinetic::FLineEndpoint E;
	switch (End.Seat)
	{
	case EKineticSeat::Static:
		E.Seat = Kinetic::ESeat::Static;
		E.Position = ToCore(End.Point);
		break;
	case EKineticSeat::Kinematic:
		E.Seat = Kinetic::ESeat::Kinematic;  // Moves on its schedule, never under a line
		E.Position = ToCore(EndPosition(End));
		E.Velocity = End.Kinematic.IsValid() ? ToCore(End.Kinematic->Velocity()) : FVec3{};
		break;
	case EKineticSeat::Prop:
	{
		const AKineticProp* Prop = End.Prop.Get();
		const FVector Seat = Prop->GetActorTransform().TransformPosition(End.Point);
		E.Seat = Kinetic::ESeat::Body;
		E.Position = ToCore(Seat);
		E.Velocity = Prop->IsSimulating() ? ToCore(Prop->Mesh->GetPhysicsLinearVelocityAtPoint(Seat)) : FVec3{};
		E.InvMass = bHeld || !Kinetic::CanEverMoveUnderALine(Prop->Body()) ? 0.0 : 1.0 / Prop->MassKg;
		E.bSmallPropBonus = Kinetic::IsSmallPropBonusTarget(Prop->MassKg, T);
		break;
	}
	case EKineticSeat::Plate:
	case EKineticSeat::OwnPlate:
	{
		const AKineticCharacter* P = End.Player.Get();
		E.Seat = End.Seat == EKineticSeat::Plate ? Kinetic::ESeat::OtherPlate : Kinetic::ESeat::OwnPlate;
		E.Position = ToCore(PlatePosition(P, End.Plate));
		E.Velocity = ToCore(P->GetCharacterMovement()->Velocity);
		E.InvMass = 1.0 / MassOf(P, T);
		break;
	}
	case EKineticSeat::None:
		break;
	}
	return E;
}

void UKineticSpoolWorld::Simulate()
{
	const Kinetic::FKineticTables& T = *Tables();
	const Kinetic::FSpoolGlobals& G = T.Globals();
	UWorld* World = GetWorld();

	TArray<AKineticCharacter*> PlayerBodies;
	for (TActorIterator<AKineticCharacter> It(World); It; ++It)
	{
		if (StateOf(*It).bStanding)
		{
			PlayerBodies.Add(*It);
		}
	}
	TArray<AKineticProp*> PropBodies;
	for (TActorIterator<AKineticProp> It(World); It; ++It)
	{
		if (!It->IsSpent())  // A volatile that went off is gone until the reset
		{
			PropBodies.Add(*It);
		}
	}
	const int32 NP = PlayerBodies.Num();
	const int32 NB = NP + PropBodies.Num();
	const auto BodyOf = [&](const FKineticLineEnd& E) -> int32 {
		switch (E.Seat)
		{
		case EKineticSeat::Prop: return E.Prop.IsValid() ? NP + PropBodies.IndexOfByKey(E.Prop.Get()) : INDEX_NONE;
		case EKineticSeat::Plate:
		case EKineticSeat::OwnPlate: return E.Player.IsValid() ? PlayerBodies.IndexOfByKey(E.Player.Get()) : INDEX_NONE;
		default: return INDEX_NONE;
		}
	};

	// A placed prop with nothing left under it (the one below was pulled out) falls.
	for (AKineticProp* Prop : PropBodies)
	{
		if (Prop->IsSimulating() || Prop->Placement == EKineticPlacement::Loose || Prop->bFixedScenery)
		{
			continue;
		}
		FVector Origin, Extent;
		Prop->GetActorBounds(true, Origin, Extent);
		FCollisionQueryParams Params(SCENE_QUERY_STAT(KineticSupport), false, Prop);
		FHitResult Under;
		const FVector Bottom = Origin - FVector(0, 0, Extent.Z);
		if (!World->LineTraceSingleByChannel(Under, Origin, Bottom - FVector(0, 0, 10.0), ECC_Visibility, Params))
		{
			Prop->BreakFree();
		}
	}

	// Which props hold this step: the thresholds see T, without the Scrapper bonus, so a trial
	// step on copies decides first.
	TArray<FVec3> Threshold;
	Threshold.SetNum(NB);
	TArray<bool> Pulled;
	Pulled.Init(false, NB);
	for (const FKineticLine& L : LineList)
	{
		if (!L.IsLive() || !IsActive(L.State.Mode))
		{
			continue;
		}
		Kinetic::FLineState Trial = L.State;
		const Kinetic::FLineStepResult R =
		    Kinetic::StepLine(Trial, L.Params, L.TensionN, EndpointOf(L.A, false), EndpointOf(L.B, false), G, Dt, Clock);
		const int32 A = BodyOf(L.A), B = BodyOf(L.B);
		if (A != INDEX_NONE)
		{
			Threshold[A] = Threshold[A] + R.BreakFreeOnA;
			Pulled[A] = true;
		}
		if (B != INDEX_NONE)
		{
			Threshold[B] = Threshold[B] + R.BreakFreeOnB;
			Pulled[B] = true;
		}
	}
	TArray<bool> Held;
	Held.Init(false, PropBodies.Num());
	for (int32 C = 0; C < PropBodies.Num(); ++C)
	{
		AKineticProp* Prop = PropBodies[C];
		if (!Kinetic::CanEverMoveUnderALine(Prop->Body()))
		{
			Held[C] = true;
			continue;
		}
		const Kinetic::FObjectLineResponse R = Kinetic::ResolveObjectUnderLines(Prop->Body(), Threshold[NP + C], !Pulled[NP + C], T);
		if (R.bBreaksFreeNow)
		{
			Prop->BreakFree();
		}
		Held[C] = !Prop->IsSimulating();
	}

	// Haul and Jack forces.
	struct FSeatPull
	{
		int32 Prop;
		FVector At;
		FVec3 Force;
	};
	TArray<FSeatPull> SeatPulls;
	TArray<FVec3> Pull;
	Pull.SetNum(NB);
	TArray<bool> Kick;
	Kick.Init(false, NB);
	for (int32 I = 0; I < LineList.Num();)
	{
		FKineticLine& L = LineList[I];
		if (!L.IsLive() || !IsActive(L.State.Mode))
		{
			++I;
			continue;
		}
		const int32 A = BodyOf(L.A), B = BodyOf(L.B);
		const bool bHeldA = A >= NP && Held[A - NP];
		const bool bHeldB = B >= NP && Held[B - NP];
		const Kinetic::FLineStepResult R =
		    Kinetic::StepLine(L.State, L.Params, L.TensionN, EndpointOf(L.A, bHeldA), EndpointOf(L.B, bHeldB), G, Dt, Clock);
		const bool bKick = (R.Events & Kinetic::LineEvents::KickActive) != 0;
		if (A != INDEX_NONE)
		{
			Pull[A] = Pull[A] + R.ForceOnA;
			Kick[A] = Kick[A] || bKick;
		}
		if (B != INDEX_NONE)
		{
			Pull[B] = Pull[B] + R.ForceOnB;
			Kick[B] = Kick[B] || bKick;
		}
		// On a prop, the pull acts where the Dog seated: it turns the prop as well as moving it.
		for (const auto& [End, Force] : {std::pair(&L.A, R.ForceOnA), std::pair(&L.B, R.ForceOnB)})
		{
			const int32 Body = BodyOf(*End);
			if (Body >= NP && Force.Length() > 1e-6)
			{
				SeatPulls.Add({Body - NP, DogPosition(*End), Force});
			}
		}
		if (R.Events & (Kinetic::LineEvents::DogsMet | Kinetic::LineEvents::AutoRelease))
		{
			EndLine(I, (R.Events & Kinetic::LineEvents::DogsMet) ? EKineticLineEnded::DogsMet : EKineticLineEnded::Released);
			continue;
		}
		++I;
	}

	// Which bodies a Hang or Set line holds this step.
	TArray<bool> InHang;
	InHang.Init(false, NB);
	for (const FKineticLine& L : LineList)
	{
		if (L.IsLive() && IsHang(L.State.Mode))
		{
			for (const int32 Body : {BodyOf(L.A), BodyOf(L.B)})
			{
				if (Body != INDEX_NONE)
				{
					InHang[Body] = true;
				}
			}
		}
	}

	// Players: footing under the line, then their velocity (05 sections 5.1 to 5.4).
	TArray<FVec3> PlayerVelocity;
	PlayerVelocity.SetNum(NP);
	for (int32 I = 0; I < NP; ++I)
	{
		AKineticCharacter* Character = PlayerBodies[I];
		UCharacterMovementComponent* Movement = Character->GetCharacterMovement();
		FKineticSpoolPlayer& P = StateOf(Character);
		const Kinetic::FHarnessTuning& H = T.Harness(KineticHarness::ToCore(Character->Harness));
		FVec3 V = ToCore(Movement->Velocity);
		const bool bGrounded = Movement->IsMovingOnGround();

		// Brace: crouch while still for 0.3 s (05 5.4).
		const bool bStill = Flat(V).Length() < BraceStillSpeed;
		if (H.bCanBrace && P.bBraceInput && bGrounded && bStill)
		{
			P.BraceHeld += Dt;
			P.bBraced = P.bBraced || P.BraceHeld >= BraceSeconds;
		}
		else
		{
			P.BraceHeld = 0.0;
			P.bBraced = false;
		}

		const FVec3 LineForce = Pull[I] + FVec3{P.LastHangForce.X, P.LastHangForce.Y, P.LastHangForce.Z};
		const bool bAnyLine = LineForce.Length() > 1e-6 || P.bUnderLine;
		const Kinetic::FPlayerLineResponse Resp =
		    Kinetic::ResolvePlayerUnderLine(Kinetic::PlayerBodyFor(H, P.bBraced, bGrounded), LineForce, Kick[I], G.Gravity);
		if (Resp.bBraceBroken)
		{
			P.bBraced = false;
		}
		const bool bLine = Flat(Pull[I]).Length() > 1e-6;
		if (bAnyLine)
		{
			switch (Resp.Footing)
			{
			case Kinetic::EFooting::Braced:
				V = {0.0, 0.0, V.Z};
				break;
			case Kinetic::EFooting::Skid:
				if (bGrounded)
				{
					FVec3 F2 = Flat(V);
					if (bLine && !P.bUnderLine)
					{
						// The line takes over: the feet plant and only motion along the pull is kept.
						const FVec3 Dir = Flat(Pull[I]).SafeNormal();
						F2 = Dir * FMath::Max(0.0, F2.Dot(Dir));
					}
					if (bLine)
					{
						F2 = F2 + Flat(Pull[I]) * (Dt / H.MassKg);  // The skid: the line's acceleration, nothing else
					}
					V = {F2.X, F2.Y, 0.0};
					break;
				}
				[[fallthrough]];
			case Kinetic::EFooting::Airborne:
			case Kinetic::EFooting::Hoisted:
				if (bGrounded)
				{
					Movement->SetMovementMode(MOVE_Falling);
					P.bBraced = false;
					P.Monitor->NotifyHandoff();  // Grounded to flight body, once: the settle is not a hit
				}
				V = V + Pull[I] * (Dt / H.MassKg);
				break;
			}
		}
		P.Footing = Resp.Footing;
		P.bUnderLine = bLine;
		Character->SetUnderLine(bLine, P.bBraced);
		PlayerVelocity[I] = V;
		if (bAnyLine && !InHang[I])
		{
			Movement->Velocity = FromCore(V);
		}
	}

	// Props under a Haul or Jack and no Hang: each pull as an impulse at its seat; Chaos
	// integrates.
	for (const FSeatPull& Seat : SeatPulls)
	{
		AKineticProp* Prop = PropBodies[Seat.Prop];
		if (!Held[Seat.Prop] && !InHang[NP + Seat.Prop])
		{
			Prop->Mesh->AddImpulseAtLocation(FromCore(Seat.Force * Dt), Seat.At);  // N·s to kg·cm/s
		}
	}

	// The Hang solve over every body, the floor under each grounded player (HangSolver.h). It
	// integrates as the rules do; the engine then gets the step's velocity, plus back the
	// gravity it will add itself over the step.
	TArray<Kinetic::FHangLine> HangLines;
	TArray<int32> HangOwners;
	std::vector<Kinetic::FHangBody> Solve;
	Solve.reserve(static_cast<size_t>(NB) + 8);
	const FVec3 GravityStep{0.0, 0.0, -G.Gravity * Dt};
	for (int32 I = 0; I < NP; ++I)
	{
		AKineticCharacter* Character = PlayerBodies[I];
		const bool bGrounded = Character->GetCharacterMovement()->IsMovingOnGround();
		const FVec3 V = bGrounded ? FVec3{PlayerVelocity[I].X, PlayerVelocity[I].Y, GravityStep.Z} : PlayerVelocity[I] + GravityStep;
		const FKineticSpoolPlayer& P = StateOf(Character);
		Solve.push_back({ToCore(Character->GetActorLocation()), V, P.bBraced ? 0.0 : 1.0 / MassOf(Character, T),
		                 bGrounded ? std::optional<double>(Character->GetActorLocation().Z / 100.0) : std::nullopt});
	}
	for (int32 C = 0; C < PropBodies.Num(); ++C)
	{
		AKineticProp* Prop = PropBodies[C];
		const bool bMoves = !Held[C] && Prop->IsSimulating();
		const FVec3 V = bMoves ? ToCore(Prop->Mesh->GetPhysicsLinearVelocity()) + Pull[NP + C] * (Dt / Prop->MassKg) + GravityStep
		                       : FVec3{};
		Solve.push_back({ToCore(Prop->Mesh->GetCenterOfMass()), V, bMoves ? 1.0 / Prop->MassKg : 0.0, std::nullopt});
	}
	for (int32 I = 0; I < LineList.Num(); ++I)
	{
		FKineticLine& L = LineList[I];
		if (!L.IsLive() || !IsHang(L.State.Mode))
		{
			continue;
		}
		const auto Index = [&](const FKineticLineEnd& E) {
			const int32 B = BodyOf(E);
			if (B != INDEX_NONE)
			{
				return B;
			}
			// A static Dog, or one riding a kinematic: a point that moves with it.
			const FVec3 V = E.Seat == EKineticSeat::Kinematic && E.Kinematic.IsValid() ? ToCore(E.Kinematic->Velocity()) : FVec3{};
			Solve.push_back({ToCore(DogPosition(E)), V, 0.0, std::nullopt});
			return static_cast<int32>(Solve.size()) - 1;
		};
		Kinetic::FHangLine Hang;
		Hang.BodyA = Index(L.A);
		Hang.BodyB = Index(L.B);
		Hang.State = &L.State;
		Hang.Params = L.Params;
		Hang.HangLoadN = L.Params.HangLoadN;
		HangLines.Add(Hang);
		HangOwners.Add(I);
	}
	if (HangLines.Num() > 0)
	{
		std::vector<Kinetic::FHangLine> Lines(HangLines.GetData(), HangLines.GetData() + HangLines.Num());
		std::vector<Kinetic::FHangBody> Before = Solve;
		Kinetic::SolveHangLines(Solve, Lines, G, Dt);
		for (int32 I = 0; I < NP; ++I)
		{
			StateOf(PlayerBodies[I]).LastHangForce = FVector::ZeroVector;
		}
		for (const Kinetic::FHangLine& Hang : Lines)
		{
			if (Hang.BodyA < NP)
			{
				StateOf(PlayerBodies[Hang.BodyA]).LastHangForce += FVector(Hang.ForceOnA.X, Hang.ForceOnA.Y, Hang.ForceOnA.Z);
			}
			if (Hang.BodyB < NP)
			{
				StateOf(PlayerBodies[Hang.BodyB]).LastHangForce += FVector(Hang.ForceOnB.X, Hang.ForceOnB.Y, Hang.ForceOnB.Z);
			}
		}
		const FVector Lift(0.0, 0.0, G.Gravity * Dt * 100.0);
		for (int32 Body = 0; Body < NB; ++Body)
		{
			if (!InHang[Body] || Solve[Body].InvMass <= 0.0)
			{
				continue;
			}
			// The step's velocity, drift correction included.
			const FVec3 Step = (Solve[Body].Position - Before[Body].Position) * (1.0 / Dt);
			if (Body < NP)
			{
				UCharacterMovementComponent* Movement = PlayerBodies[Body]->GetCharacterMovement();
				Movement->Velocity = Movement->IsMovingOnGround() ? FVector(Step.X * 100.0, Step.Y * 100.0, 0.0)
				                                                  : FromCore(Step) + Lift;
			}
			else
			{
				PropBodies[Body - NP]->Mesh->SetPhysicsLinearVelocity(FromCore(Step) + Lift);
			}
		}
		for (int32 K = Lines.size(); K-- > 0;)
		{
			if (Lines[K].Events & Kinetic::LineEvents::RanOut)
			{
				EndLine(HangOwners[K], EKineticLineEnded::RanOut);
			}
		}
	}
	else
	{
		for (AKineticCharacter* Character : PlayerBodies)
		{
			StateOf(Character).LastHangForce = FVector::ZeroVector;
		}
	}

	// The hard velocity cap (05 7.1).
	for (AKineticCharacter* Character : PlayerBodies)
	{
		UCharacterMovementComponent* Movement = Character->GetCharacterMovement();
		const double Cap = G.HardVelocityCap * 100.0;
		if (Movement->Velocity.Size() > Cap)
		{
			Movement->Velocity *= Cap / Movement->Velocity.Size();
		}
	}
}

void UKineticSpoolWorld::Stall(AKineticCharacter* Player, Kinetic::EStallCause Cause)
{
	FKineticSpoolPlayer& P = StateOf(Player);
	if (!P.bStanding)
	{
		return;
	}
	P.bStanding = false;
	P.StallCause = Cause;
	P.StalledAt = Clock;
	P.LastOutcome = Kinetic::EOutcome::Stall;
	P.LastOutcomeAt = Clock;
	P.bDrawn = false;
	P.bBraced = false;
	UE_LOG(LogKineticSpool, Log, TEXT("%s Stalled (%d)"), *Player->GetName(), static_cast<int>(Cause));
	FKineticSpoolEvent E;
	E.Kind = EKineticSpoolEventKind::Stall;
	E.Target = Player;
	E.Cause = Cause;
	Emit(MoveTemp(E));
	// The Hooks lift them out: off the board until they come back.
	Player->LeaveLadder();
	Player->SetStunned(true);
	Player->SetUnderLine(false, false);
	Player->GetCharacterMovement()->StopMovementImmediately();
	Player->GetCharacterMovement()->DisableMovement();
	Player->SetActorEnableCollision(false);
	Player->SetActorHiddenInGame(true);
}

void UKineticSpoolWorld::Respawn(AKineticCharacter* Player)
{
	for (TActorIterator<APlayerStart> It(GetWorld()); It; ++It)
	{
		if (It->PlayerStartTag == FName(TEXT("Spawn_A")))
		{
			PlaceStanding(Player, It->GetActorLocation(), It->GetActorRotation());
			return;
		}
	}
	PlaceStanding(Player, FVector::ZeroVector, FRotator::ZeroRotator);
}

void UKineticSpoolWorld::PlaceStanding(AKineticCharacter* Player, const FVector& At, const FRotator& Facing)
{
	if (!Player)
	{
		return;
	}
	FKineticSpoolPlayer& P = StateOf(Player);
	Player->LeaveLadder();
	Player->SetActorEnableCollision(true);
	Player->SetActorHiddenInGame(false);
	Player->TeleportTo(At, Facing);
	if (AController* Controller = Player->GetController())
	{
		Controller->SetControlRotation(Facing);
	}
	UCharacterMovementComponent* Movement = Player->GetCharacterMovement();
	Movement->StopMovementImmediately();
	Movement->SetMovementMode(MOVE_Walking);
	Player->SetStunned(false);
	Player->SetUnderLine(false, false);
	Player->TakeContacts();
	P.bStanding = true;
	P.StunnedUntil = -1.0;
	P.LastOutcome = Kinetic::EOutcome::None;
	P.LastHangForce = FVector::ZeroVector;
	P.bDrawn = false;
	P.bBraced = false;
	P.BraceHeld = 0.0;
	P.bSelfDogNext = false;
	P.Monitor = MakeShared<Kinetic::FImpactMonitor>(*Tables());
}

void UKineticSpoolWorld::Impacts()
{
	const Kinetic::FKineticTables& T = *Tables();
	for (TActorIterator<AKineticCharacter> It(GetWorld()); It; ++It)
	{
		AKineticCharacter* Character = *It;
		FKineticSpoolPlayer& P = StateOf(Character);
		std::vector<Kinetic::FContact> Contacts = Character->TakeContacts();
		if (!P.bStanding)
		{
			if (bAutoRespawn && Clock - P.StalledAt >= RespawnSeconds)
			{
				Respawn(Character);
			}
			continue;
		}
		// Ring-out: into the Well's kill volume, or off the edge of the arena (05 7.5).
		if (Character->GetActorLocation().Z - PlayerHalfHeightCm < KillZCm)
		{
			Stall(Character, Kinetic::EStallCause::RingOut);
			continue;
		}
		const Kinetic::EOutcome Outcome = P.Monitor->EndStep(Contacts, Clock, Dt);
		if (Outcome == Kinetic::EOutcome::Stall)
		{
			Kinetic::EStallCause Cause = Kinetic::EStallCause::WorldImpact;
			const FVector At = Character->GetActorLocation() / 100.0;
			for (const Kinetic::FContact& C : Contacts)
			{
				UE_LOG(LogKineticSpool, Log, TEXT("%s hit at (%.1f, %.1f, %.1f) m: kind %d, %.1f m/s, normal (%.2f, %.2f, %.2f)"),
				       *Character->GetName(), At.X, -At.Y, At.Z, static_cast<int>(C.Kind), C.RelativeSpeed, C.Normal.X, C.Normal.Y,
				       C.Normal.Z);
				Cause = C.Kind == Kinetic::EContactKind::Object      ? Kinetic::EStallCause::ObjectImpact
				        : C.Kind == Kinetic::EContactKind::Player    ? Kinetic::EStallCause::PlayerImpact
				        : C.Kind == Kinetic::EContactKind::Kinematic ? Kinetic::EStallCause::KinematicImpact
				                                                     : Cause;
			}
			Stall(Character, Cause);
			continue;
		}
		if (Outcome == Kinetic::EOutcome::Stagger || Outcome == Kinetic::EOutcome::Flip)
		{
			P.StunnedUntil = Clock + (Outcome == Kinetic::EOutcome::Flip ? FlipSeconds : StaggerSeconds);
			P.bBraced = false;
			P.LastOutcome = Outcome;
			P.LastOutcomeAt = Clock;
		}
		Character->SetStunned(Clock < P.StunnedUntil);
	}
}

void UKineticSpoolWorld::HangLineContacts()
{
	// Hang and set lines are solid to players (05 3.5): walking into one stops you; over 3 m/s
	// it staggers you (the line owner's class time); over 6 m/s it flips you over it with your
	// momentum. Haul and Jack lines are not solid. A line within a step of the feet is stepped
	// over, measured on the capsule's axis (PROPOSED).
	const Kinetic::FKineticTables& T = *Tables();
	FCollisionQueryParams Params(SCENE_QUERY_STAT(KineticFlipOver), false);
	for (const FKineticLine& L : LineList)
	{
		if (!L.IsLive() || !IsHang(L.State.Mode))
		{
			continue;
		}
		const FVector A = DogPosition(L.A);
		const FVector B = DogPosition(L.B);
		for (TActorIterator<AKineticCharacter> It(GetWorld()); It; ++It)
		{
			AKineticCharacter* Character = *It;
			FKineticSpoolPlayer& P = StateOf(Character);
			const bool bOnIt = L.A.Player.Get() == Character || L.B.Player.Get() == Character;
			if (!P.bStanding || bOnIt)
			{
				continue;
			}
			const FVector C = Character->GetActorLocation();
			const FVector Low = C - FVector(0, 0, PlayerHalfHeightCm - LineStepOverCm);
			const FVector High = C + FVector(0, 0, PlayerHalfHeightCm);
			FVector OnLine, OnBody;
			FMath::SegmentDistToSegmentSafe(A, B, Low, High, OnLine, OnBody);
			if (FVector::Dist(OnLine, OnBody) > PlayerRadiusCm)
			{
				continue;
			}
			// The way out across the line, on the ground.
			const FVector Across = FVector(B - A).GetSafeNormal2D();
			FVector Out = FVector(C - OnLine).GetSafeNormal2D();
			Out = (Out - Across * FVector::DotProduct(Out, Across)).GetSafeNormal();
			if (Out.IsNearlyZero())
			{
				continue;
			}
			UCharacterMovementComponent* Movement = Character->GetCharacterMovement();
			const double Speed = -FVector::DotProduct(Movement->Velocity, Out) / 100.0;  // Into the line, m/s
			if (Speed <= 0.0)
			{
				continue;
			}
			// Already staggered or flipped: the line still blocks, but does not hit again.
			Kinetic::EOutcome Hit = Clock < P.StunnedUntil ? Kinetic::EOutcome::None
			                                               : Kinetic::Classify(Kinetic::EImpactTable::HangLineHit, Speed,
			                                                                   Kinetic::ESurface::Hard, T);
			const FVector Over = C - Out * (2.0 * PlayerRadiusCm + 10.0) + FVector(0, 0, 40.0);
			Params.ClearIgnoredSourceObjects();
			Params.AddIgnoredActor(Character);
			const bool bClear = !GetWorld()->OverlapBlockingTestByChannel(
			    Over, FQuat::Identity, ECC_Pawn, FCollisionShape::MakeCapsule(PlayerRadiusCm, PlayerHalfHeightCm), Params);
			if (Hit == Kinetic::EOutcome::Flip && !bClear)
			{
				Hit = Kinetic::EOutcome::Stagger;  // No room on the far side: it stops them instead
			}
			if (Hit == Kinetic::EOutcome::Flip)
			{
				// Over it, landing on the far side with momentum kept.
				Character->TeleportTo(Over, Character->GetActorRotation());
				Movement->SetMovementMode(MOVE_Falling);
				Movement->Velocity.Z = FMath::Max(Movement->Velocity.Z, 300.0);
				P.StunnedUntil = Clock + FlipSeconds;
			}
			else
			{
				Movement->Velocity += Out * (Speed * 100.0);
				Character->SetActorLocation(FVector(OnLine.X, OnLine.Y, C.Z) + Out * (PlayerRadiusCm + 2.0));
				if (Hit == Kinetic::EOutcome::Stagger && L.Owner.IsValid())
				{
					P.StunnedUntil = Clock + T.Harness(KineticHarness::ToCore(L.Owner->Harness)).HangStaggerSeconds;
				}
			}
			if (Hit != Kinetic::EOutcome::None)
			{
				P.LastOutcome = Hit;
				P.LastOutcomeAt = Clock;
				Character->SetStunned(true);
			}
		}
	}
}

void UKineticSpoolWorld::Step()
{
	if (!Tables())
	{
		return;
	}
	UpdateDogs();
	UpdateLines();
	Simulate();
	HangLineContacts();
	Impacts();
	UpdateLines();
	Clock += Dt;
	if (Renderer)
	{
		Renderer->Draw(*this);
	}
}

void UKineticSpoolWorld::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (GetWorld() && GetWorld()->HasBegunPlay())
	{
		Step();
	}
}
