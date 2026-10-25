#include "KineticYardDirector.h"

#include "Components/PointLightComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kinetic/Impact.h"
#include "KineticCharacter.h"
#include "KineticGameMode.h"
#include "KineticHazardWorld.h"
#include "KineticKinematic.h"
#include "Engine/Engine.h"
#include "Kinetic/Tables.h"
#include "KineticProp.h"
#include "KineticRulesSubsystem.h"
#include "KineticSpoolWorld.h"
#include "Materials/MaterialInstanceDynamic.h"

DEFINE_LOG_CATEGORY_STATIC(LogKineticYard, Log, All);

using Kinetic::FVec3;

namespace
{
constexpr double Dt = 1.0 / 60.0;
constexpr double Inf = 1e300;
constexpr double DeckZ = -0.4;           // The Hauler deck rides here (layout zone 1)
constexpr double CurtainParkX = 86.0;    // Out past the end walls
constexpr double PlankLifeSeconds = 10.0;
constexpr double StripForceN = 1000.0;   // A Breaker's Haul: the plate is a rated attachment
constexpr double PinnedCrushSeconds = 0.5;
const FName PlankTag(TEXT("Kinetic.PlankBridge"));
const FName RamTag(TEXT("Kinetic.PressRam"));

// The layout's frame (metres, X east, Y north) to Unreal's (cm, Y flipped).
FVector L(double X, double Y, double Z) { return FVector(X * 100.0, -Y * 100.0, Z * 100.0); }
FVec3 Core(const FVector& Cm) { return {Cm.X / 100.0, Cm.Y / 100.0, Cm.Z / 100.0}; }
FVector Cm(const FVec3& M) { return FVector(M.X * 100.0, M.Y * 100.0, M.Z * 100.0); }
double Wrap(double T, double Period) { return T - Period * FMath::Floor(T / Period); }
bool Crossed(double From, double To, double Mark) { return From < Mark && To >= Mark; }

// A piecewise-linear key track in time: (t, value).
FVector Track(const TArray<TPair<double, FVector>>& Keys, double T)
{
	if (T <= Keys[0].Key)
	{
		return Keys[0].Value;
	}
	for (int32 I = 1; I < Keys.Num(); ++I)
	{
		if (T <= Keys[I].Key)
		{
			const double A = (T - Keys[I - 1].Key) / FMath::Max(1e-9, Keys[I].Key - Keys[I - 1].Key);
			return FMath::Lerp(Keys[I - 1].Value, Keys[I].Value, A);
		}
	}
	return Keys.Last().Value;
}

// The Gantry's 120 s cycle (events-and-moving-parts 2): bridge X, trolley Y, magnet underside Z
// (layout metres). The pick at c 0 is on the Pick Stack's 8 m top; the release at c 80.
const TArray<TPair<double, FVector>>& NormalCycle()
{
	static const TArray<TPair<double, FVector>> Keys = {
	    {0.0, FVector(-50, 14, 10.7)}, {2.0, FVector(-50, 14, 10.7)},  {10.0, FVector(-50, 14, 30)},
	    {70.0, FVector(45, 0, 30)},     {80.0, FVector(45, 0, 12)},      {85.0, FVector(45, 0, 34)},
	    {115.0, FVector(-50, 14, 34)},  {120.0, FVector(-50, 14, 10.7)}};
	return Keys;
}
constexpr double NormalToneC = 78.0, NormalReleaseC = 80.0;

// After Shift Change, at the East Pick Stack (5.4 m): pick at u 3, release over the press at u
// 18.3 (PROPOSED).
const TArray<TPair<double, FVector>>& EastCycle()
{
	static const TArray<TPair<double, FVector>> Keys = {
	    {0.0, FVector(62, 14, 34)},  {3.0, FVector(62, 14, 8.1)},  {8.0, FVector(62, 14, 30)},
	    {13.3, FVector(45, 0, 30)},  {18.3, FVector(45, 0, 12)},   {21.5, FVector(62, 14, 34)}};
	return Keys;
}
constexpr double EastPickU = 3.0, EastToneU = 16.3, EastReleaseU = 18.3;
}  // namespace

AKineticYardDirector::AKineticYardDirector()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickGroup = TG_PrePhysics;
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	// Trench, first nose-in, nose-in after Shift Change, Lockout stagger, direction, klaxon.
	Trenches[0] = {-30.0, 10.0, 3.0, 0.0, true, TEXT("KlaxonLow")};
	Trenches[1] = {0.0, 25.0, 10.5, 7.5, false, TEXT("KlaxonTwoTone")};
	Trenches[2] = {30.0, 40.0, 18.0, 15.0, true, TEXT("KlaxonHigh")};
}

void AKineticYardDirector::BeginPlay()
{
	Super::BeginPlay();
	UWorld* World = GetWorld();
	if (AGameModeBase* Mode = World->GetAuthGameMode())
	{
		AddTickPrerequisiteActor(Mode);  // The match's clock is read after it has stepped
	}
	FActorSpawnParameters Params;
	Params.ObjectFlags |= RF_Transient;
	const auto Spawn = [&](const TCHAR* Label) {
		AKineticKinematic* K = World->SpawnActor<AKineticKinematic>(Params);
#if WITH_EDITOR
		K->SetActorLabel(Label);
#endif
		return K;
	};
	// The Haulers: the chassis on the rails under the deck, the hull plate load up to +3 m,
	// the headlamp on the nose. The pose is the nose; the train is behind it (local -X).
	const FLinearColor Chassis(0.18f, 0.18f, 0.2f), Plate(0.42f, 0.36f, 0.3f), Lamp(1.0f, 0.85f, 0.4f);
	for (int32 I = 0; I < 3; ++I)
	{
		AKineticKinematic* K = Spawn(*FString::Printf(TEXT("Hauler_%d"), I));
		K->AddBox(TEXT("Chassis"), FVector(-700.0, 0.0, -175.0), FVector(1400.0, 320.0, 350.0), Chassis);
		K->AddBox(TEXT("Load"), FVector(-700.0, 0.0, 170.0), FVector(1200.0, 500.0, 340.0), Plate);
		K->AddBox(TEXT("Headlamp"), FVector(-5.0, 0.0, -60.0), FVector(10.0, 160.0, 40.0), Lamp, false);
		K->Park(L(Trenches[I].bEastward ? -95.0 : 95.0, Trenches[I].YM, -10.0));
		Trains[I] = K;
	}
	// The Gantry: the bridge beam and the cab (along X), and the magnet on its hoist cable.
	Bridge = Spawn(TEXT("Gantry_Bridge"));
	Bridge->AddBox(TEXT("Beam"), FVector(0.0, 0.0, 3525.0), FVector(150.0, 6180.0, 150.0), FLinearColor(0.85f, 0.55f, 0.1f));
	Bridge->AddBox(TEXT("Cab"), FVector(0.0, -2850.0, 3800.0), FVector(300.0, 300.0, 400.0), FLinearColor(0.3f, 0.32f, 0.35f));
	Magnet = Spawn(TEXT("Gantry_Magnet"));
	Magnet->AddBox(TEXT("Magnet"), FVector(0.0, 0.0, 30.0), FVector(240.0, 240.0, 60.0), FLinearColor(0.25f, 0.25f, 0.27f));
	Cable = Magnet->AddBox(TEXT("Cable"), FVector(0.0, 0.0, 300.0), FVector(6.0, 6.0, 500.0), FLinearColor(0.1f, 0.1f, 0.1f), false);
	Cable->SetCollisionEnabled(ECollisionEnabled::QueryOnly);  // Doggable, not a wall
	// The Lockout curtains, parked out past the end walls.
	for (int32 Side = 0; Side < 2; ++Side)
	{
		AKineticKinematic* K = Spawn(Side == 0 ? TEXT("Lockout_West") : TEXT("Lockout_East"));
		K->AddBox(TEXT("Curtain"), FVector(0.0, 0.0, 1400.0), FVector(100.0, 9000.0, 3600.0), FLinearColor(0.12f, 0.12f, 0.14f));
		K->bHitsPlayers = false;  // It pushes and crushes instead
		K->Park(L(Side == 0 ? -CurtainParkX : CurtainParkX, 0.0, 0.0));
		Curtains[Side] = K;
	}
	// The trench lamps, every 10 m along both lips.
	LampHolder = World->SpawnActor<AActor>(Params);
	LampHolder->SetRootComponent(NewObject<USceneComponent>(LampHolder, TEXT("Root")));
	LampHolder->GetRootComponent()->RegisterComponent();
	UStaticMesh* Cube = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
	// The lamps glow (build_yard_greybox.py's M_Yard_Lamp); every fourth throws light up the
	// trench wall (lighting-and-mood.md: 2,700 K amber, red on alarm).
	UMaterialInterface* Glow = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/Arenas/YRD_Yard/Greybox/M_Yard_Lamp.M_Yard_Lamp"),
	                                                          nullptr, LOAD_NoWarn | LOAD_Quiet);
	for (int32 I = 0; I < 3; ++I)
	{
		for (const double Lip : {-3.15, 3.15})
		{
			for (int32 X = -75; X <= 75; X += 10)
			{
				UStaticMeshComponent* M = NewObject<UStaticMeshComponent>(LampHolder);
				M->SetStaticMesh(Cube);
				M->SetCollisionEnabled(ECollisionEnabled::NoCollision);
				M->SetCanEverAffectNavigation(false);
				M->SetupAttachment(LampHolder->GetRootComponent());
				M->RegisterComponent();
				M->SetWorldLocation(L(X, Trenches[I].YM + Lip, 0.1));
				M->SetWorldScale3D(FVector(0.3, 0.3, 0.2));
				if (Glow)
				{
					M->SetMaterial(0, Glow);
				}
				M->CreateDynamicMaterialInstance(0);
				UPointLightComponent* Light = nullptr;
				if ((X + 75) % 40 == 0)
				{
					Light = NewObject<UPointLightComponent>(LampHolder);
					Light->SetupAttachment(LampHolder->GetRootComponent());
					Light->RegisterComponent();
					// Just inboard of the lip and below it, lighting the wall.
					Light->SetWorldLocation(L(X, Trenches[I].YM + Lip * 0.8, -0.3));
					Light->SetIntensityUnits(ELightUnits::Candelas);
					Light->SetIntensity(600.0f);
					Light->SetAttenuationRadius(900.0f);
					Light->SetCastShadows(false);
				}
				Lamps.Add({M, Light, I, static_cast<double>(X), -1});
			}
		}
	}
	for (TActorIterator<AActor> It(World); It; ++It)
	{
		if (It->ActorHasTag(PlankTag))
		{
			Planks.Add(*It);
		}
		if (It->ActorHasTag(RamTag))
		{
			Ram = *It;
			RamRest = It->GetActorLocation();
			// It moves every step: kept out of the navmesh, which would rebuild round it forever.
			if (UPrimitiveComponent* Body = Cast<UPrimitiveComponent>(Ram->GetRootComponent()))
			{
				Body->SetCanEverAffectNavigation(false);
			}
		}
	}
	BuildRated();
	UE_LOG(LogKineticYard, Log, TEXT("Yard director: 3 Haulers, the Gantry, 2 curtains, %d lamps, %d plank bridges, ram %s"),
	       Lamps.Num(), Planks.Num(), Ram ? TEXT("found") : TEXT("missing"));
}

void AKineticYardDirector::ReadClock()
{
	PrevE = E;
	const AKineticGameMode* Mode = GetWorld()->GetAuthGameMode<AKineticGameMode>();
	if (const FKineticScrap* Scrap = Mode ? Mode->Scrap() : nullptr)
	{
		const FKineticRoundTimes T = Scrap->RoundTimes();
		const auto Rel = [&T](double At) { return At >= 0.0 ? At - T.LiveAt : -1.0; };
		E = T.Now - T.LiveAt;
		ESiren = Rel(T.SirenAt);
		EMid = Rel(T.MidMatchAt);
		EPlannedMid = T.PlannedMidMatchAt - T.LiveAt;
		ELock = Rel(T.SuddenDeathAt);
		if (T.Resets != Resets)
		{
			Resets = T.Resets;
			ResetRound();
		}
		return;
	}
	// The walk-the-arena mode: live from the start of play, the default Scrap rules' event.
	const UKineticSpoolWorld* Spool = GetWorld()->GetSubsystem<UKineticSpoolWorld>();
	E = Spool ? Spool->Now() : 0.0;
	EPlannedMid = 69.0;
	ESiren = E >= EPlannedMid - 5.0 ? EPlannedMid - 5.0 : -1.0;
	EMid = E >= EPlannedMid ? EPlannedMid : -1.0;
	ELock = -1.0;
	if (Resets < 0)
	{
		Resets = 0;
		ResetRound();
	}
}

void AKineticYardDirector::ResetRound()
{
	// Events-and-moving-parts 7: trains back in the plenums, loads off the Floor, curtains
	// parked, plank bridges re-laid; the Gantry restarts at its phase.
	for (int32 I = 0; I < 3; ++I)
	{
		Trains[I]->Park(L(Trenches[I].bEastward ? -95.0 : 95.0, Trenches[I].YM, -10.0));
	}
	for (int32 Side = 0; Side < 2; ++Side)
	{
		Curtains[Side]->Park(L(Side == 0 ? -CurtainParkX : CurtainParkX, 0.0, 0.0));
	}
	for (FLoad& Load : Loads)
	{
		if (AKineticProp* Prop = Load.Prop.Get())
		{
			Prop->Destroy();
		}
	}
	Loads.Reset();
	for (const TWeakObjectPtr<AActor>& Plank : Planks)
	{
		if (AActor* P = Plank.Get())
		{
			P->SetActorHiddenInGame(false);
			P->SetActorEnableCollision(true);
		}
	}
	Pinned.Reset();
	// The rated loads back on their blocks, the shack rebuilt on both columns.
	for (FRated& R : Rated)
	{
		R.Commit = Kinetic::FRatedCommit();
		R.GroanAt = R.FiredAt = -1.0;
		R.bCarried = R.bGone = false;
		R.Body->SetActorHiddenInGame(false);
		R.Body->SetActorEnableCollision(true);
		R.Body->MoveTo(R.Rest, FRotator::ZeroRotator, Dt);
		R.Body->MoveTo(R.Rest, FRotator::ZeroRotator, Dt);  // Twice: no velocity carried over
	}
	if (Shack)
	{
		Shack->MoveTo(ShackRest, FRotator::ZeroRotator, Dt);
		Shack->MoveTo(ShackRest, FRotator::ZeroRotator, Dt);
	}
	ShackFallAt = -1.0;
	bShackDown = false;
	bMidFired = bSirenFired = bLockFired = false;
	LastCycleC = LastEastU = -1.0;
	double Speed = 0.0;
	GantryAt = GantryTarget(Speed);
	// Mid-cycle at the reset with the plate up (the Scrap phase has it lowering into the press).
	const double C = Wrap(Wrap(32.0 - EPlannedMid, GantryCycle) + E, GantryCycle);
	if (C > 0.0 && C < NormalReleaseC)
	{
		Pick();
	}
}

Kinetic::FRailRun AKineticYardDirector::RunFor(int32 I) const
{
	const FTrench& T = Trenches[I];
	const double X0 = T.bEastward ? -80.0 : 80.0;
	const FVector A = L(X0, T.YM, DeckZ), B = L(-X0, T.YM, DeckZ);
	std::vector<Kinetic::FDepartureSegment> S;
	const double LockStart = ELock >= 0.0 ? ELock + 5.0 : Inf;
	S.push_back({T.First, 45.0, EMid >= 0.0 ? FMath::Min(EMid, LockStart) : LockStart});
	if (EMid >= 0.0)
	{
		// Shift Change: every 22.5 s for 60 s, then 45 s counted from this trench's last train.
		const double Fast0 = EMid + T.Offset;
		const double FastUntil = FMath::Min(EMid + 60.0, LockStart);
		S.push_back({Fast0, 22.5, FastUntil});
		const double Last = FastUntil > Fast0 ? Fast0 + 22.5 * FMath::Floor((FastUntil - 1e-6 - Fast0) / 22.5) : EMid;
		S.push_back({Last + 45.0, 45.0, LockStart});
	}
	if (ELock >= 0.0)
	{
		S.push_back({LockStart + T.Stagger, 22.5, Inf});  // Lockout: all three, continuously
	}
	return Kinetic::FRailRun(Kinetic::FPolylinePath({Core(A), Core(B)}), HaulerSpeed, Kinetic::FTimetable(S), TrainLengthM);
}

AKineticKinematic* AKineticYardDirector::Hauler(int32 Trench) const { return Trench >= 0 && Trench < 3 ? Trains[Trench].Get() : nullptr; }
AKineticKinematic* AKineticYardDirector::Curtain(int32 Side) const { return Side >= 0 && Side < 2 ? Curtains[Side].Get() : nullptr; }
bool AKineticYardDirector::IsHaulerRunning(int32 Trench) const { return RunFor(Trench).IsRunning(E); }
bool AKineticYardDirector::IsCarrying() const
{
	return Loads.ContainsByPredicate([](const FLoad& Load) { return Load.bOnMagnet && Load.Prop.IsValid(); });
}

std::vector<Kinetic::FBotHauler> AKineticYardDirector::HaulersForBots() const
{
	std::vector<Kinetic::FBotHauler> Out;
	for (int32 I = 0; I < 3; ++I)
	{
		const Kinetic::FRailRun Run = RunFor(I);
		const FVec3 End = Run.Route().PointAt(Run.Route().Length());
		if (const std::optional<Kinetic::FPose> Pose = Run.PoseAt(E))
		{
			Out.push_back({Pose->Position, End, 0.0, HaulerSpeed});
			continue;
		}
		const std::vector<double> Next = Run.Schedule().DeparturesBetween(E, E + 3.0);  // The klaxon has sounded
		if (!Next.empty())
		{
			Out.push_back({Run.Route().PointAt(0.0), End, Next.front() - E, HaulerSpeed});
		}
	}
	return Out;
}

void AKineticYardDirector::Strike(AKineticKinematic& K)
{
	UKineticSpoolWorld* Spool = GetWorld()->GetSubsystem<UKineticSpoolWorld>();
	if (Spool && Spool->SeverLines(K) > 0)
	{
		const FVector At = K.GetActorLocation();
		UKineticHazardWorld::PlayCue(GetWorld(), TEXT("Snap"), &At);
	}
	// Whoever is in its way is hit at its speed (the kinematic table: a Stall at 9 m/s), and
	// thrown along; whoever stands on it rides.
	if (K.bHitsPlayers)
	{
		const FVector V = K.Velocity();
		const FVector Dir = V.GetSafeNormal();
		for (TActorIterator<AKineticCharacter> It(GetWorld()); It; ++It)
		{
			AKineticCharacter* C = *It;
			const UPrimitiveComponent* Base = C->GetMovementBase();
			if ((Spool && !Spool->IsStanding(C)) || (Base && Base->GetOwner() == &K) || !K.Overlaps(*C))
			{
				continue;
			}
			Kinetic::FContact Hit;
			Hit.Kind = Kinetic::EContactKind::Kinematic;
			Hit.OtherId = static_cast<int>(K.GetUniqueID());
			Hit.Normal = {Dir.X, Dir.Y, Dir.Z};
			Hit.RelativeSpeed = FMath::Max(0.0, FVector::DotProduct(V - C->GetVelocity(), Dir)) / 100.0;
			C->AddContact(Hit);
			C->LaunchCharacter(V + FVector(0.0, 0.0, 150.0), true, true);
		}
	}
	// A train's first pass through a plank bridge takes it.
	const FBox Swept = K.GetComponentsBoundingBox();
	for (const TWeakObjectPtr<AActor>& Plank : Planks)
	{
		AActor* P = Plank.Get();
		if (P && !P->IsHidden() && Swept.Intersect(P->GetComponentsBoundingBox()))
		{
			Fracture(P);
		}
	}
}

void AKineticYardDirector::Fracture(AActor* Plank)
{
	if (!Plank || Plank->IsHidden())
	{
		return;
	}
	Plank->SetActorHiddenInGame(true);
	Plank->SetActorEnableCollision(false);
	const FVector At = Plank->GetActorLocation();
	UKineticHazardWorld::PlayCue(GetWorld(), TEXT("Snap"), &At);
	// It breaks into Light planks (05 6.5) that last 10 s.
	UStaticMesh* PlankMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Game/Objects/Light/SM_Light_Plank/SM_Light_Plank"));
	if (!PlankMesh)
	{
		return;
	}
	const FVector Along = Plank->GetActorRightVector();
	for (int32 I = -1; I <= 1; ++I)
	{
		const FTransform T(FRotator(0.0, Plank->GetActorRotation().Yaw + 90.0, 0.0), At + Along * (I * 200.0) + FVector(0, 0, 20));
		AKineticProp* Piece = GetWorld()->SpawnActorDeferred<AKineticProp>(AKineticProp::StaticClass(), T, nullptr, nullptr,
		                                                                   ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
		Piece->Mesh->SetStaticMesh(PlankMesh);
		Piece->MassKg = 12.0f;
		Piece->FinishSpawning(T);
		Piece->SetLifeSpan(PlankLifeSeconds);
	}
}

void AKineticYardDirector::UpdateHaulers()
{
	for (int32 I = 0; I < 3; ++I)
	{
		const Kinetic::FRailRun Run = RunFor(I);
		// The klaxon, arena-wide, 3 s before each nose-in.
		if (!Run.Schedule().DeparturesBetween(PrevE + 3.0, E + 3.0).empty() && PrevE > -50.0)
		{
			const FVector At = L(0.0, Trenches[I].YM, 2.0);  // From the trench, heard across the Yard
			UKineticHazardWorld::PlayCue(GetWorld(), Trenches[I].Klaxon, &At);
		}
		AKineticKinematic* K = Trains[I];
		if (const std::optional<Kinetic::FPose> Pose = Run.PoseAt(E))
		{
			K->MoveTo(Cm(Pose->Position), FRotator(0.0, FMath::RadiansToDegrees(Pose->Angle), 0.0), Dt);
			Strike(*K);
		}
		else
		{
			K->Park(L(Trenches[I].bEastward ? -95.0 : 95.0, Trenches[I].YM, -10.0));
		}
	}
}

FVector AKineticYardDirector::GantryTarget(double& OutHoistSpeed) const
{
	OutHoistSpeed = 5.0;
	if (ELock >= 0.0 && E >= ELock)
	{
		return FVector(0.0, 0.0, 34.0);  // Lockout: parked at X 0, the magnet hoisted
	}
	if (EMid >= 0.0 && E >= EMid)
	{
		// Shift Change: east at double speed, then the East Pick Stack's cycles.
		const double S = E - EMid;
		const double Run = FMath::Abs(62.0 - XAtMid) / 5.4;
		if (S < Run)
		{
			return FVector(62.0, 14.0, 34.0);
		}
		double Start = Run;
		for (;;)
		{
			const double Period = Start < 60.0 ? 35.0 : 60.0;
			if (S < Start + Period)
			{
				return Track(EastCycle(), S - Start);
			}
			Start += Period;
		}
	}
	return Track(NormalCycle(), Wrap(Wrap(32.0 - EPlannedMid, GantryCycle) + E, GantryCycle));
}

void AKineticYardDirector::Pick()
{
	if (IsCarrying())
	{
		return;
	}
	// A fresh hull plate on the magnet: 12 m by 6 m by 2.7 m, 2,800 kg, a rated attachment.
	const FVector At = L(GantryAt.X, GantryAt.Y, GantryAt.Z - 1.35);
	const FTransform T(FRotator::ZeroRotator, At, FVector(12.0, 6.0, 2.7));
	AKineticProp* Plate = GetWorld()->SpawnActorDeferred<AKineticProp>(AKineticProp::StaticClass(), T, nullptr, nullptr,
	                                                                   ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
	Plate->Mesh->SetStaticMesh(LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube")));
	Plate->MassKg = 2800.0f;
	Plate->Placement = EKineticPlacement::Rated;
	Plate->FinishSpawning(T);
	if (UMaterialInstanceDynamic* M = Plate->Mesh->CreateDynamicMaterialInstance(0))
	{
		M->SetVectorParameterValue(TEXT("Color"), FLinearColor::FromSRGBColor(FColor(0xE8, 0x62, 0x0C)));  // HTR orange
	}
	Loads.Add({Plate});
}

void AKineticYardDirector::Release(bool bByShiftChange)
{
	for (FLoad& Load : Loads)
	{
		AKineticProp* Prop = Load.Prop.Get();
		if (!Load.bOnMagnet || !Prop)
		{
			continue;
		}
		Load.bOnMagnet = false;
		Load.LastVelocity = Magnet->Velocity();
		Prop->BreakFree();  // Chaos has it from here: a 2,800 kg plate falling
		Prop->Mesh->SetPhysicsLinearVelocity(Magnet->Velocity());
		++Drops;
		const FVector At = Prop->GetActorLocation();
		UKineticHazardWorld::PlayCue(GetWorld(), TEXT("MagnetRelease"), &At);
		UE_LOG(LogKineticYard, Log, TEXT("Gantry release at %s (%s)"), *At.ToCompactString(),
		       bByShiftChange ? TEXT("Shift Change") : TEXT("cycle"));
	}
}

void AKineticYardDirector::WatchLoads()
{
	UKineticSpoolWorld* Spool = GetWorld()->GetSubsystem<UKineticSpoolWorld>();
	UKineticHazardWorld* Hazards = GetWorld()->GetSubsystem<UKineticHazardWorld>();
	for (int32 I = Loads.Num(); I-- > 0;)
	{
		FLoad& Load = Loads[I];
		AKineticProp* Prop = Load.Prop.Get();
		if (!Prop)
		{
			Loads.RemoveAt(I);
			continue;
		}
		if (Load.bOnMagnet)
		{
			Prop->SetActorLocation(L(GantryAt.X, GantryAt.Y, GantryAt.Z - 1.35), false, nullptr, ETeleportType::None);
			// A Breaker's Haul over 1,000 N strips it: a 1.0 s groan, then it falls (05 6.7).
			bool bStripped = false;
			static const TArray<FKineticLine> NoLines;
			for (const FKineticLine& Line : Spool ? Spool->Lines() : NoLines)
			{
				const bool bOn = (Line.A.Seat == EKineticSeat::Prop && Line.A.Prop.Get() == Prop) ||
				                 (Line.B.Seat == EKineticSeat::Prop && Line.B.Prop.Get() == Prop);
				bStripped |= bOn && Line.IsLive() && Line.State.Mode == Kinetic::ELineMode::Haul &&
				             Line.State.AppliedForce >= StripForceN - 1e-6;
			}
			if (bStripped && Load.StripCuedAt < 0.0)
			{
				Load.StripCuedAt = Now;
				const FVector At = Prop->GetActorLocation();
				UKineticHazardWorld::PlayCue(GetWorld(), TEXT("Groan"), &At);
			}
			if (Load.StripCuedAt >= 0.0 && Now - Load.StripCuedAt >= 1.0)
			{
				Load.bOnMagnet = false;
				Prop->BreakFree();
				Prop->Mesh->SetPhysicsLinearVelocity(Magnet->Velocity());
				++Drops;
			}
			continue;
		}
		if (Prop->GetActorLocation().Z < -500.0)
		{
			Prop->Destroy();  // The press takes it
			Loads.RemoveAt(I);
			continue;
		}
		if (!Prop->IsSimulating())
		{
			continue;  // Landed and fixed
		}
		const FVector V = Prop->Mesh->GetPhysicsLinearVelocity();
		if (!Load.bLanded && (V - Load.LastVelocity).Size() > 500.0)
		{
			// The landing: a 3 m debris shove, 12 m/s² for 0.5 s (05 6.6).
			Load.bLanded = true;
			const FVector At = Prop->GetActorLocation();
			UKineticHazardWorld::PlayCue(GetWorld(), TEXT("PlateLand"), &At);
			if (Hazards)
			{
				Hazards->Shove(At, 3.0 + 6.0, 12.0, 0.5);  // From the plate's edge: half its 12 m length out
			}
		}
		Load.LastVelocity = V;
		Load.StillFor = V.Size() < 30.0 ? Load.StillFor + Dt : 0.0;
		if (Load.bLanded && Load.StillFor > 0.5)
		{
			Prop->bFixedScenery = true;  // Where it lies it stays, as scenery (05 6.1a)
			Prop->ApplyBody();
		}
	}
}

void AKineticYardDirector::UpdateGantry()
{
	// The one-shot moments: the drop tone 2 s before each release, picks and releases.
	if (EMid >= 0.0 && !bMidFired && E >= EMid)
	{
		bMidFired = true;
		XAtMid = GantryAt.X;
		Release(true);  // Shift Change: wherever it is
	}
	if (ESiren >= 0.0 && !bSirenFired && E >= ESiren)
	{
		bSirenFired = true;
		UKineticHazardWorld::PlayCue(GetWorld(), TEXT("Siren"));
	}
	if (ESiren >= 0.0 && Crossed(PrevE, E, ESiren + 2.0) && IsCarrying())
	{
		const FVector At = Magnet->GetActorLocation();
		UKineticHazardWorld::PlayCue(GetWorld(), TEXT("DropTone"), &At);
	}
	const bool bLockout = ELock >= 0.0 && E >= ELock;
	if (bLockout && !bLockFired)
	{
		bLockFired = true;
		Release(false);
		UKineticHazardWorld::PlayCue(GetWorld(), TEXT("Siren"));
	}
	if (!bLockout && (EMid < 0.0 || E < EMid))
	{
		const double C = Wrap(Wrap(32.0 - EPlannedMid, GantryCycle) + E, GantryCycle);
		if (LastCycleC >= 0.0)
		{
			const bool bWrapped = C < LastCycleC;
			if (bWrapped)
			{
				Pick();
			}
			if (!bWrapped && Crossed(LastCycleC, C, NormalToneC) && IsCarrying())
			{
				const FVector At = Magnet->GetActorLocation();
				UKineticHazardWorld::PlayCue(GetWorld(), TEXT("DropTone"), &At);
			}
			if (!bWrapped && Crossed(LastCycleC, C, NormalReleaseC))
			{
				Release(false);
			}
		}
		LastCycleC = C;
	}
	else if (!bLockout)
	{
		const double S = E - EMid;
		const double Run = FMath::Abs(62.0 - XAtMid) / 5.4;
		double U = -1.0;
		for (double Start = Run; S >= Run;)
		{
			const double Period = Start < 60.0 ? 35.0 : 60.0;
			if (S < Start + Period)
			{
				U = S - Start;
				break;
			}
			Start += Period;
		}
		if (U >= 0.0 && LastEastU >= 0.0 && U >= LastEastU)
		{
			if (Crossed(LastEastU, U, EastPickU))
			{
				Pick();
			}
			if (Crossed(LastEastU, U, EastToneU) && IsCarrying())
			{
				const FVector At = Magnet->GetActorLocation();
				UKineticHazardWorld::PlayCue(GetWorld(), TEXT("DropTone"), &At);
			}
			if (Crossed(LastEastU, U, EastReleaseU))
			{
				Release(false);
			}
		}
		LastEastU = U;
	}
	// Follow the schedule (smoothly through a change of plan).
	double Hoist = 5.0;
	const FVector Target = GantryTarget(Hoist);
	GantryAt.X = FMath::FInterpConstantTo(GantryAt.X, Target.X, Dt, 6.0);
	GantryAt.Y = FMath::FInterpConstantTo(GantryAt.Y, Target.Y, Dt, 3.0);
	GantryAt.Z = FMath::FInterpConstantTo(GantryAt.Z, Target.Z, Dt, Hoist);
	Bridge->MoveTo(L(GantryAt.X, 0.0, 0.0), FRotator::ZeroRotator, Dt);
	Magnet->MoveTo(L(GantryAt.X, GantryAt.Y, GantryAt.Z), FRotator::ZeroRotator, Dt);
	const double CableCm = FMath::Max(10.0, (34.5 - GantryAt.Z) * 100.0 - 60.0);
	Cable->SetRelativeLocation(FVector(0.0, 0.0, 60.0 + CableCm * 0.5));
	Cable->SetRelativeScale3D(FVector(0.06, 0.06, CableCm / 100.0));
	Strike(*Bridge);
	Strike(*Magnet);
	WatchLoads();
}

void AKineticYardDirector::UpdateCurtains()
{
	UKineticSpoolWorld* Spool = GetWorld()->GetSubsystem<UKineticSpoolWorld>();
	const double S = ELock >= 0.0 ? E - ELock : -1.0;
	for (int32 Side = 0; Side < 2; ++Side)
	{
		AKineticKinematic* K = Curtains[Side];
		const double Sign = Side == 0 ? -1.0 : 1.0;
		if (S < 5.0)
		{
			K->Park(L(Sign * CurtainParkX, 0.0, 0.0));
			continue;
		}
		// From X ±80 at 1 m/s to ±10 by T+75, hold; from T+90, on to ±1 by T+99.
		double X = 80.0 - FMath::Min(S - 5.0, 70.0);
		if (S > 90.0)
		{
			X = 10.0 - FMath::Min(S - 90.0, 9.0);
		}
		K->MoveTo(L(Sign * X, 0.0, 0.0), FRotator::ZeroRotator, Dt);
		Strike(*K);
		// Push riggers ahead of it; one pinned against something solid for 0.5 s is crushed.
		const FVector Inboard(-Sign, 0.0, 0.0);
		for (TActorIterator<AKineticCharacter> It(GetWorld()); It; ++It)
		{
			AKineticCharacter* C = *It;
			if (Spool && !Spool->IsStanding(C))
			{
				continue;
			}
			double& Held = Pinned.FindOrAdd(C);
			if (!K->Overlaps(*C))
			{
				Held = FMath::Max(0.0, Held - 2.0 * Dt);
				continue;
			}
			FVector To = C->GetActorLocation();
			To.X = K->GetActorLocation().X + Inboard.X * (50.0 + 45.0);
			FHitResult Blocked;
			C->SetActorLocation(To, true, &Blocked);
			Held = Blocked.bBlockingHit ? Held + Dt : FMath::Max(0.0, Held - 2.0 * Dt);
			if (Held >= PinnedCrushSeconds && Spool)
			{
				Spool->Hit(C, Kinetic::EOutcome::Stall, Kinetic::EStallCause::Crush);
				Held = 0.0;
			}
		}
	}
}

void AKineticYardDirector::UpdateRam()
{
	if (!Ram)
	{
		return;
	}
	// 20 s: up 3.5 m over 8 s, a 2 s hold with the clang, down over 10 s.
	const double T = Wrap(E, 20.0);
	const double Rise = T < 8.0 ? 350.0 * T / 8.0 : T < 10.0 ? 350.0 : 350.0 * (1.0 - (T - 10.0) / 10.0);
	Ram->SetActorLocation(RamRest + FVector(0.0, 0.0, Rise), false, nullptr, ETeleportType::None);
	if (Crossed(Wrap(PrevE, 20.0), T, 8.0))
	{
		const FVector At = Ram->GetActorLocation();
		UKineticHazardWorld::PlayCue(GetWorld(), TEXT("RamClang"), &At);
	}
}

void AKineticYardDirector::UpdateLamps()
{
	const bool bRed = (ESiren >= 0.0 && E >= ESiren && (EMid < 0.0 || E < EMid + 60.0)) || (ELock >= 0.0 && E >= ELock);
	double Nose[3];
	bool bRunning[3], bEntry[3];
	for (int32 I = 0; I < 3; ++I)
	{
		const Kinetic::FRailRun Run = RunFor(I);
		const std::optional<double> D = Run.NoseAt(E);
		bRunning[I] = D.has_value();
		Nose[I] = D ? *D : 0.0;
		bEntry[I] = !Run.Schedule().DeparturesBetween(E - 2.0, E + 3.0).empty();
	}
	const FLinearColor Colours[] = {FLinearColor(0.22f, 0.11f, 0.0f), FLinearColor(1.0f, 0.55f, 0.0f), FLinearColor(0.9f, 0.04f, 0.02f)};
	for (FLamp& Lamp : Lamps)
	{
		const FTrench& T = Trenches[Lamp.Trench];
		// Distance of the lamp along the trench from the entry end.
		const double Along = T.bEastward ? Lamp.XM + 80.0 : 80.0 - Lamp.XM;
		int32 State = 0;
		if (bRed)
		{
			State = 2;
		}
		else if ((bRunning[Lamp.Trench] && Along >= Nose[Lamp.Trench] && Along <= Nose[Lamp.Trench] + 24.0) ||
		         (bEntry[Lamp.Trench] && Along <= 24.0))
		{
			State = 1;  // 24 m ahead of the nose; the entry end from the klaxon
		}
		if (State != Lamp.State)
		{
			Lamp.State = State;
			if (UMaterialInstanceDynamic* M = Cast<UMaterialInstanceDynamic>(Lamp.Mesh->GetMaterial(0)))
			{
				M->SetVectorParameterValue(TEXT("Color"), Colours[State]);
				M->SetScalarParameterValue(TEXT("Strength"), State == 0 ? 6.0f : 30.0f);
			}
			if (Lamp.Light)
			{
				Lamp.Light->SetLightColor(Colours[State] * (1.0f / FMath::Max(0.01f, Colours[State].GetMax())));
				Lamp.Light->SetIntensity(State == 0 ? 600.0f : 2500.0f);
			}
		}
	}
}

void AKineticYardDirector::BuildRated()
{
	FActorSpawnParameters Params;
	Params.ObjectFlags |= RF_Transient;
	const FLinearColor RatedColour(0.80f, 0.62f, 0.10f), Structure(0.40f, 0.40f, 0.42f);
	const auto Make = [&](const FVector& At) {
		AKineticKinematic* K = GetWorld()->SpawnActor<AKineticKinematic>(Params);
		K->MoveTo(At, FRotator::ZeroRotator, Dt);
		K->Tags.Add(TEXT("Kinetic.Anchor"));  // Bots Haul enemies into them
		return K;
	};
	// The hull sections, 12 m by 6 m by 2.7 m, each sliding into its nearest trench.
	const FVector2D Sections[] = {{-70.0, 16.0}, {36.0, 22.0}, {-70.0, -6.0}, {68.0, -22.0}};
	for (const FVector2D& C : Sections)
	{
		FRated R;
		R.Kind = ERated::Plow;
		R.Rest = L(C.X, C.Y, 1.35);
		R.Body = Make(R.Rest);
		R.Body->AddBox(TEXT("Section"), FVector::ZeroVector, FVector(1200.0, 600.0, 270.0), RatedColour);
		R.Body->SetAffectsNavigation(true);
		double Best = Inf;
		for (int32 I = 0; I < 3; ++I)
		{
			const double Yc = Trenches[I].YM;
			const double Gap = C.Y - 3.0 > Yc + 3.0 ? (C.Y - 3.0) - (Yc + 3.0) : (Yc - 3.0) - (C.Y + 3.0);
			if (Gap >= -1e-6 && Gap < Best)
			{
				Best = Gap;
				R.Trench = I;
			}
		}
		const double Dir = Trenches[R.Trench].YM > C.Y ? 1.0 : -1.0;
		R.Axis = FVector(0.0, -Dir, 0.0);  // Layout north is Unreal -Y
		R.Lip = L(C.X, C.Y + Dir * (Best + 4.0), 1.35);
		Rated.Add(R);
	}
	// The control shack, 8 m by 10 m, floor 10.3 to 10.5 m, roof at 14 m, on two rated columns.
	ShackRest = L(-62.0, 38.0, 10.3);
	Shack = GetWorld()->SpawnActor<AKineticKinematic>(Params);
	Shack->MoveTo(ShackRest, FRotator::ZeroRotator, Dt);
	Shack->AddBox(TEXT("Floor"), FVector(0, 0, 10), FVector(800, 1000, 20), Structure);
	Shack->AddBox(TEXT("Roof"), FVector(0, 0, 360), FVector(800, 1000, 20), Structure);
	Shack->AddBox(TEXT("WallN"), FVector(0, -490, 185), FVector(800, 20, 330), Structure);
	Shack->AddBox(TEXT("WallW"), FVector(-390, 0, 185), FVector(20, 1000, 330), Structure);
	Shack->AddBox(TEXT("WallE"), FVector(390, 0, 185), FVector(20, 1000, 330), Structure);
	Shack->AddBox(TEXT("SillS"), FVector(0, 490, 65), FVector(800, 20, 90), Structure);  // Windows south
	Shack->SetAffectsNavigation(true);
	Shack->Tags.Add(TEXT("Kinetic.Anchor"));
	for (const double Y : {35.0, 41.0})
	{
		FRated R;
		R.Kind = ERated::ShackColumn;
		R.Rest = L(-62.0, Y, 5.15);
		R.Body = Make(R.Rest);
		R.Body->AddBox(TEXT("Column"), FVector::ZeroVector, FVector(60.0, 60.0, 1030.0), RatedColour);
		R.Body->SetAffectsNavigation(true);
		Rated.Add(R);
	}
	// The press counterweight, 6,000 kg, hanging from the frame's arm with its underside at 6 m.
	FRated W;
	W.Kind = ERated::Counterweight;
	W.Rest = L(45.0, 9.0, 7.5);
	W.Body = Make(W.Rest);
	W.Body->AddBox(TEXT("Block"), FVector::ZeroVector, FVector(200.0, 200.0, 300.0), RatedColour);
	W.Body->AddBox(TEXT("Cable"), FVector(0.0, 0.0, 300.0), FVector(10.0, 10.0, 300.0), FLinearColor(0.1f, 0.1f, 0.1f), false);
	Rated.Add(W);
}

FVector AKineticYardDirector::PullOn(const AKineticKinematic& Body) const
{
	// The active pull on it this step (N): every Haul toward its far end, every Jack push away.
	FVector Sum = FVector::ZeroVector;
	const UKineticSpoolWorld* Spool = GetWorld()->GetSubsystem<UKineticSpoolWorld>();
	if (!Spool)
	{
		return Sum;
	}
	for (const FKineticLine& Line : Spool->Lines())
	{
		const bool bJack = Line.State.Mode == Kinetic::ELineMode::Jack;
		if (!Line.IsLive() || (Line.State.Mode != Kinetic::ELineMode::Haul && !bJack))
		{
			continue;
		}
		for (const bool bA : {true, false})
		{
			const FKineticLineEnd& Mine = bA ? Line.A : Line.B;
			if (Mine.Seat == EKineticSeat::Kinematic && Mine.Kinematic.Get() == &Body)
			{
				const FVector Toward = (Spool->DogPosition(bA ? Line.B : Line.A) - Spool->DogPosition(Mine)).GetSafeNormal();
				Sum += Toward * (bJack ? -Line.State.AppliedForce : Line.State.AppliedForce);
			}
		}
	}
	return Sum;
}

void AKineticYardDirector::UpdateRated()
{
	const UKineticRulesSubsystem* Rules = GEngine->GetEngineSubsystem<UKineticRulesSubsystem>();
	const Kinetic::FKineticTables* T = Rules ? Rules->Tables() : nullptr;
	UKineticSpoolWorld* Spool = GetWorld()->GetSubsystem<UKineticSpoolWorld>();
	if (!T)
	{
		return;
	}
	for (FRated& R : Rated)
	{
		AKineticKinematic* K = R.Body;
		if (R.FiredAt < 0.0)
		{
			// Hold until the pull along its axis has held the rated threshold; groan; then go.
			const FVector Pull = PullOn(*K);
			// Along its axis; a pull within 45° of it counts whole (05 6.2: a Breaker's 1,000 N
			// starts a rated load on any axis, a Dog seated low or high included).
			const double Dot = FVector::DotProduct(Pull, R.Axis);
			const double Along = R.Axis.IsZero() || Dot >= Pull.Size() * UE_HALF_SQRT_2 ? Pull.Size() : Dot;
			if (R.Commit.Update(Along, Dt, *T) && R.GroanAt < 0.0)
			{
				R.GroanAt = Now;
				const FVector At = K->GetActorLocation();
				UKineticHazardWorld::PlayCue(GetWorld(), TEXT("Groan"), &At);
				UE_LOG(LogKineticYard, Log, TEXT("%s committed"), *K->GetName());
			}
			if (R.GroanAt >= 0.0 && Now - R.GroanAt >= 1.0)
			{
				R.FiredAt = Now;
				if (R.Kind == ERated::ShackColumn && ShackFallAt < 0.0)
				{
					ShackFallAt = Now;  // Either column: the shack comes down
				}
			}
			else
			{
				K->MoveTo(K->GetActorLocation(), K->GetActorRotation(), Dt);  // At rest: no velocity
				continue;
			}
		}
		const double S = Now - R.FiredAt;
		switch (R.Kind)
		{
		case ERated::Plow:
		{
			if (R.bGone)
			{
				break;
			}
			const FTrench& Tr = Trenches[R.Trench];
			const FVector Across = L(R.Rest.X / 100.0, Tr.YM, 1.35);
			if (!R.bCarried)
			{
				// Off its greased skids to the lip (4 s, visible), then over it: across the trench,
				// its ends on both lips.
				FVector To = S < 4.0 ? FMath::Lerp(R.Rest, R.Lip, FMath::Square(S / 4.0)) : Across;
				const FRotator Yaw(0.0, S < 4.0 ? 0.0 : 90.0, 0.0);
				if (S >= 4.0 && S < 5.0)
				{
					To = FMath::Lerp(R.Lip, Across, S - 4.0);
				}
				K->MoveTo(To, Yaw, Dt);
				Strike(*K);
				// The next train takes it.
				AKineticKinematic* Train = Trains[R.Trench];
				if (S >= 5.0 && Train->IsActive())
				{
					const double Ahead = (K->GetActorLocation().X - Train->GetActorLocation().X) * (Tr.bEastward ? 1.0 : -1.0);
					R.bCarried = Ahead <= 320.0;
				}
			}
			else
			{
				AKineticKinematic* Train = Trains[R.Trench];
				if (!Train->IsActive())
				{
					R.bGone = true;  // Dumped in the end plenum and removed
					K->Park(L(Tr.bEastward ? 95.0 : -95.0, Tr.YM, -20.0));
					break;
				}
				K->MoveTo(Train->GetActorLocation() + Train->GetActorForwardVector() * 300.0 + FVector(0.0, 0.0, 175.0),
				          FRotator(0.0, 90.0, 0.0), Dt);
				Strike(*K);  // A 12 m wall at 12 m/s: its ends overhang both lips by 3 m
			}
			break;
		}
		case ERated::ShackColumn:
			K->Park(R.Rest - FVector(0.0, 0.0, 2000.0));  // Buckled
			break;
		case ERated::Counterweight:
		{
			// The cable pays out to 8 m, the block swings north twice over 6 s, then lands.
			const FVector Pivot = L(45.0, 9.0, 12.0);
			if (S < 6.0)
			{
				const double Radius = 450.0 + 350.0 * FMath::Min(S, 1.0);
				const double Swing = FMath::DegreesToRadians(60.0) * FMath::Sin(2.0 * PI * S / 3.0) * (1.0 - S / 6.0);
				const double North = FMath::Abs(Swing);  // Over Marker Flat, never into the frame
				K->MoveTo(Pivot + FVector(0.0, -FMath::Sin(North) * Radius, -FMath::Cos(North) * Radius), FRotator::ZeroRotator, Dt);
			}
			else
			{
				const FVector Rest = L(45.0, 14.0, 1.5);
				K->MoveTo(S < 6.5 ? FMath::Lerp(K->GetActorLocation(), Rest, (S - 6.0) / 0.5) : Rest, FRotator::ZeroRotator, Dt);
			}
			Strike(*K);
			break;
		}
		}
	}
	// The shack: 10.5 m down onto the North Dock; whoever rides it lands at 14.3 m/s.
	if (Shack && ShackFallAt >= 0.0 && !bShackDown)
	{
		const double S = Now - ShackFallAt;
		const double Drop = FMath::Min(0.5 * 981.0 * S * S, 1030.0);
		Shack->MoveTo(ShackRest - FVector(0.0, 0.0, Drop), FRotator::ZeroRotator, Dt);
		Strike(*Shack);
		if (Drop >= 1030.0)
		{
			bShackDown = true;
			for (TActorIterator<AKineticCharacter> It(GetWorld()); It; ++It)
			{
				const UPrimitiveComponent* Base = It->GetMovementBase();
				if (Base && Base->GetOwner() == Shack && Spool)
				{
					Spool->Hit(*It, Kinetic::EOutcome::Stall, Kinetic::EStallCause::WorldImpact);
				}
			}
			const FVector At = Shack->GetActorLocation();
			UKineticHazardWorld::PlayCue(GetWorld(), TEXT("PlateLand"), &At);
		}
	}
	else if (Shack)
	{
		Shack->MoveTo(Shack->GetActorLocation(), FRotator::ZeroRotator, Dt);
	}
}

void AKineticYardDirector::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	if (!GetWorld()->HasBegunPlay())
	{
		return;
	}
	ReadClock();
	Now += Dt;
	UpdateHaulers();
	UpdateRated();
	UpdateGantry();
	UpdateCurtains();
	UpdateRam();
	UpdateLamps();
}
