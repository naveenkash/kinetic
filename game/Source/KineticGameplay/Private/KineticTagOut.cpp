#include "KineticTagOut.h"

#include "Camera/CameraComponent.h"
#include "Components/AudioComponent.h"
#include "Engine/DirectionalLight.h"
#include "Kismet/GameplayStatics.h"
#include "KineticKinematic.h"
#include "KineticYardDirector.h"
#include "NavigationPath.h"
#include "NavigationSystem.h"
#include "Sound/SoundAttenuation.h"
#include "Sound/SoundBase.h"
#include "Components/StaticMeshComponent.h"
#include "Components/TextRenderComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerStart.h"
#include "Kinetic/Tables.h"
#include "KineticBotController.h"
#include "KineticCharacter.h"
#include "KineticRulesSubsystem.h"
#include "KineticSpoolWorld.h"
#include "Materials/MaterialInstanceDynamic.h"

DEFINE_LOG_CATEGORY_STATIC(LogKineticTagOut, Log, All);

namespace
{
using Kinetic::FVec3;

FVec3 ToCore(const FVector& Cm) { return {Cm.X / 100.0, Cm.Y / 100.0, Cm.Z / 100.0}; }
FVector ToCm(const FVec3& M) { return FVector(M.X * 100.0, M.Y * 100.0, M.Z * 100.0); }
FVec3 ToCoreDirection(const FVector& D) { return {D.X, D.Y, D.Z}; }

constexpr double ChestAboveCentreCm = 35.0;   // UKineticSpoolWorld::PlatePosition's plates
constexpr double BeaconAboveGroundCm = 100.0; // The beacon on its stand
constexpr double LockerAboveGroundCm = 120.0; // Where a rigger touches a locker
constexpr double CapsuleAboveGroundCm = 100.0;
constexpr double GravityCm = 980.0;
const FLinearColor Grey(0.32f, 0.32f, 0.30f);
const TCHAR* const Cylinder = TEXT("/Engine/BasicShapes/Cylinder.Cylinder");
const TCHAR* const Cube = TEXT("/Engine/BasicShapes/Cube.Cube");
const TCHAR* const Sphere = TEXT("/Engine/BasicShapes/Sphere.Sphere");
// make import-tagout's materials; without them the shapes fall back to the basic shape material.
const TCHAR* const GlintMaterial = TEXT("/Game/TagOut/M_TagGlint.M_TagGlint");
const TCHAR* const DustMaterial = TEXT("/Game/TagOut/M_TagDust.M_TagDust");
const TCHAR* const HazeMaterial = TEXT("/Game/TagOut/M_TagHaze.M_TagHaze");
const FLinearColor TarpBlue(0.06f, 0.16f, 0.26f);
const FVector PressHazeCm(4150.0, 0.0, 600.0);  // The press frame over the pit's west half (layout zone 6)
constexpr double PressHazeRadiusM = 7.0;
constexpr double ExhaustHazeRadiusM = 2.5;

// The grate footbridges (layout.md, Grey-box findings): each trench's layout Y and its four X.
struct FGrates
{
	double YM;
	double X[4];
};
const FGrates GrateRows[3] = {{-30.0, {-50.0, -2.0, 40.0, 76.0}}, {0.0, {-50.0, -6.0, 34.0, 70.0}}, {30.0, {-50.0, -14.0, 28.0, 54.0}}};

USoundBase* LoadCue(const TCHAR* Name)
{
	const FString Path = FString::Printf(TEXT("/Game/Audio/SW_%s.SW_%s"), Name, Name);
	return LoadObject<USoundBase>(nullptr, *Path, nullptr, LOAD_NoWarn | LOAD_Quiet);
}

// Heard full within InnerCm, gone by InnerCm + FalloffCm, from where it is.
USoundAttenuation* Falloff(double InnerCm, float FalloffCm)
{
	static TMap<int64, USoundAttenuation*> Made;
	const int64 Key = static_cast<int64>(InnerCm) * 1000000 + static_cast<int64>(FalloffCm);
	if (USoundAttenuation** Found = Made.Find(Key))
	{
		return *Found;
	}
	USoundAttenuation* A = NewObject<USoundAttenuation>(GetTransientPackage());
	A->AddToRoot();
	A->Attenuation.bAttenuate = true;
	A->Attenuation.bSpatialize = true;
	A->Attenuation.DistanceAlgorithm = EAttenuationDistanceModel::NaturalSound;
	A->Attenuation.AttenuationShape = EAttenuationShape::Sphere;
	A->Attenuation.AttenuationShapeExtents = FVector(InnerCm, 0.0, 0.0);
	A->Attenuation.FalloffDistance = FalloffCm;
	Made.Add(Key, A);
	return A;
}

double DegreesBetween(const FVector& A, const FVector& B)
{
	return FMath::RadiansToDegrees(FMath::Acos(FMath::Clamp(FVector::DotProduct(A.GetSafeNormal(), B.GetSafeNormal()), -1.0, 1.0)));
}

Kinetic::EPlate CorePlate(int32 Plate)
{
	switch (static_cast<Kinetic::ETagPlate>(Plate))
	{
	case Kinetic::ETagPlate::LeftShoulder: return Kinetic::EPlate::LeftShoulder;
	case Kinetic::ETagPlate::RightShoulder: return Kinetic::EPlate::RightShoulder;
	case Kinetic::ETagPlate::Back:
	default: return Kinetic::EPlate::Back;
	}
}

// The digits sit just proud of the plates, facing out (a text faces its +X).
const FVector PlateTextAt[Kinetic::PlateCount] = {FVector(0.0, -42.0, 60.0), FVector(-36.0, 0.0, 35.0), FVector(0.0, 42.0, 60.0)};
const double PlateTextYaw[Kinetic::PlateCount] = {-90.0, 180.0, 90.0};
}  // namespace

FKineticTagOut::FKineticTagOut(UWorld& InWorld, const FSettings& InSettings) : World(InWorld), Settings(InSettings)
{
	const EKineticHarness Classes[CrewSize * 2] = {Settings.HumanClass,       EKineticHarness::Climber, EKineticHarness::Scrapper,
	                                               EKineticHarness::Breaker,  EKineticHarness::Doubler, EKineticHarness::Climber,
	                                               EKineticHarness::Scrapper, EKineticHarness::Breaker};
	for (int32 Slot = 0; Slot < CrewSize * 2; ++Slot)
	{
		Kinetic::FMatchPlayer P;
		P.Slot = Slot;
		P.Team = TeamOf(Slot);
		P.bIsBot = Slot != HumanSlot();
		P.AccountId = P.bIsBot ? "" : "local-player";
		P.Harness = KineticHarness::ToCore(Classes[Slot]);
		Roster.Add(P);
	}
	if (UKineticSpoolWorld* S = Spool())
	{
		S->SetAutoRespawn(false);  // The reserve brings them back, at their pad
		S->SetLinesAllowed(false);
	}
	FindSite();
	BuildSite();
	BuildHiding();
	SpawnRiggers();
	StartMatch();
}

FKineticTagOut::~FKineticTagOut() = default;

UKineticSpoolWorld* FKineticTagOut::Spool() const { return World.GetSubsystem<UKineticSpoolWorld>(); }

const Kinetic::FKineticTables* FKineticTagOut::Tables() const
{
	const UKineticRulesSubsystem* Rules = GEngine ? GEngine->GetEngineSubsystem<UKineticRulesSubsystem>() : nullptr;
	return Rules ? Rules->Tables() : nullptr;
}

AKineticCharacter* FKineticTagOut::Rigger(int32 Slot) const
{
	return Seats.IsValidIndex(Slot) ? Seats[Slot].Character.Get() : nullptr;
}

int32 FKineticTagOut::SlotOf(const AKineticCharacter* Character) const
{
	for (int32 Slot = 0; Slot < Seats.Num(); ++Slot)
	{
		if (Seats[Slot].Character.Get() == Character)
		{
			return Slot;
		}
	}
	return INDEX_NONE;
}

FString FKineticTagOut::Who(int32 Slot) const
{
	if (Slot == HumanSlot())
	{
		return TEXT("You");
	}
	const FString Crew = Settings.bHumanPlays ? (TeamOf(Slot) == 0 ? TEXT("(your crew)") : TEXT("(their crew)"))
	                                          : FString::Printf(TEXT("(crew %c)"), TeamOf(Slot) == 0 ? TEXT('A') : TEXT('B'));
	return KineticHarness::Name(KineticHarness::FromCore(Roster[Slot].Harness)) + TEXT(" ") + Crew;
}

FString FKineticTagOut::Code(const Kinetic::FTagCode& C) const
{
	return FString::Printf(TEXT("%d · %d · %d"), C.Pips[0], C.Pips[1], C.Pips[2]);
}

void FKineticTagOut::Say(FString Text)
{
	UE_LOG(LogKineticTagOut, Log, TEXT("%.1f s: %s"), TheMatch ? TheMatch->Time() : 0.0, *Text);
	FeedLines.Add(MoveTemp(Text));
	while (FeedLines.Num() > 6)
	{
		FeedLines.RemoveAt(0);
	}
}

FVector FKineticTagOut::ChestCm(int32 Slot) const
{
	const AKineticCharacter* R = Rigger(Slot);
	return R ? R->GetActorLocation() + FVector(0.0, 0.0, ChestAboveCentreCm) : FVector::ZeroVector;
}

FVector FKineticTagOut::BeaconCm(int32 Team) const { return BeaconProps[Team].At; }

double FKineticTagOut::ThrowWindup(int32 Slot) const
{
	return Seats[Slot].ThrowAt < 0.0 ? -1.0 : FMath::Max(0.0, Seats[Slot].ThrowAt - TheMatch->Time());
}

FKineticRoundTimes FKineticTagOut::RoundTimes() const
{
	const Kinetic::FTagOutRules& R = Settings.Rules;
	FKineticRoundTimes T;
	T.Now = TheMatch ? TheMatch->Time() : 0.0;
	T.ResetAt = 0.0;
	T.LiveAt = R.DropSeconds;
	T.SirenAt = SirenAt;
	T.MidMatchAt = MidMatchAt;
	T.PlannedMidMatchAt = R.DropSeconds + R.RegulationSeconds - R.MidEventRemaining;
	T.SuddenDeathAt = OvertimeAt;  // The Lockout
	T.Resets = Started;
	return T;
}

FVector FKineticTagOut::Ground(const FVector& Above, double Fallback) const
{
	FHitResult Hit;
	FCollisionQueryParams Params(SCENE_QUERY_STAT(KineticTagGround), false);
	for (const TWeakObjectPtr<AStaticMeshActor>& A : Site)
	{
		Params.AddIgnoredActor(A.Get());
	}
	for (const FSeat& Seat : Seats)
	{
		Params.AddIgnoredActor(Seat.Character.Get());
	}
	if (World.LineTraceSingleByChannel(Hit, Above + FVector(0.0, 0.0, 300.0), Above - FVector(0.0, 0.0, 3000.0), ECC_Visibility, Params))
	{
		return Hit.Location;
	}
	return FVector(Above.X, Above.Y, Fallback);
}

void FKineticTagOut::FindSite()
{
	// The home pads at the spawns (section 9: West Mesa and East Mesa tops). The layout's metres
	// have Y the other way round from the level's.
	const FVector Proposed[2] = {FVector(-6800.0, 3900.0, 800.0), FVector(6800.0, -3900.0, 800.0)};
	const double ProposedYaw[2] = {30.0, 210.0};
	for (int32 Team = 0; Team < 2; ++Team)
	{
		const FName Tag(Team == 0 ? TEXT("Spawn_A") : TEXT("Spawn_B"));
		FVector Start = Proposed[Team];
		double Yaw = ProposedYaw[Team];
		for (TActorIterator<APlayerStart> It(&World); It; ++It)
		{
			if (It->PlayerStartTag == Tag)
			{
				Start = It->GetActorLocation();
				Yaw = It->GetActorRotation().Yaw;
				break;
			}
		}
		Pads[Team] = Ground(Start, Proposed[Team].Z);
		PadFacing[Team] = FRotator(0.0, Yaw, 0.0);
	}
	// A locker beside each pad, on the same floor, and the two map lockers (NC2 and SC4 tops).
	LockerCm.Reset();
	for (int32 Team = 0; Team < 2; ++Team)
	{
		const FVector F = PadFacing[Team].Vector();
		const FVector R = FRotationMatrix(PadFacing[Team]).GetUnitAxis(EAxis::Y);
		FVector Chosen = Pads[Team] + R * 550.0;
		for (const FVector& Offset : {R * 550.0, -R * 550.0, -F * 350.0, F * 450.0})
		{
			const FVector At = Ground(Pads[Team] + Offset, Pads[Team].Z);
			if (FMath::Abs(At.Z - Pads[Team].Z) < 80.0)
			{
				Chosen = At;
				break;
			}
		}
		LockerCm.Add(Chosen);
	}
	LockerCm.Add(Ground(FVector(-600.0, -2100.0, 1300.0), 800.0));  // NC2, layout (-6, +21)
	LockerCm.Add(Ground(FVector(2800.0, 2100.0, 1300.0), 800.0));   // SC4, layout (+28, -21)
	UE_LOG(LogKineticTagOut, Log, TEXT("Tag-Out site: pads at %s and %s, %d lockers"), *Pads[0].ToCompactString(),
	       *Pads[1].ToCompactString(), LockerCm.Num());
}

AStaticMeshActor* FKineticTagOut::Shape(const TCHAR* MeshPath, const FVector& At, const FVector& Scale, const FLinearColor& Colour,
                                        UMaterialInstanceDynamic** OutMaterial, const TCHAR* Material)
{
	FActorSpawnParameters Params;
	Params.ObjectFlags |= RF_Transient;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	AStaticMeshActor* A = World.SpawnActor<AStaticMeshActor>(At, FRotator::ZeroRotator, Params);
	if (!A)
	{
		return nullptr;
	}
	A->SetMobility(EComponentMobility::Movable);
	UStaticMeshComponent* C = A->GetStaticMeshComponent();
	C->SetStaticMesh(LoadObject<UStaticMesh>(nullptr, MeshPath));
	C->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	C->SetCanEverAffectNavigation(false);
	A->SetActorScale3D(Scale);
	Site.Add(A);
	if (UMaterialInterface* Own = Material ? LoadObject<UMaterialInterface>(nullptr, Material, nullptr, LOAD_NoWarn | LOAD_Quiet) : nullptr)
	{
		C->SetMaterial(0, Own);
		C->SetCastShadow(false);
		return A;
	}
	if (UMaterialInterface* Base = LoadObject<UMaterialInterface>(nullptr, TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial")))
	{
		UMaterialInstanceDynamic* M = UMaterialInstanceDynamic::Create(Base, A);
		M->SetVectorParameterValue(TEXT("Color"), Colour);
		C->SetMaterial(0, M);
		if (OutMaterial)
		{
			*OutMaterial = M;
		}
	}
	return A;
}

UStaticMeshComponent* FKineticTagOut::Part(AActor* Owner, USceneComponent* Parent, const TCHAR* Mesh, const TCHAR* Material,
                                           const FVector& At, const FVector& Scale)
{
	UStaticMeshComponent* C = NewObject<UStaticMeshComponent>(Owner);
	C->SetStaticMesh(LoadObject<UStaticMesh>(nullptr, Mesh));
	C->SetupAttachment(Parent);
	C->SetRelativeLocation(At);
	C->SetRelativeScale3D(Scale);
	C->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	C->SetCastShadow(false);
	C->SetCanEverAffectNavigation(false);
	if (UMaterialInterface* M = LoadObject<UMaterialInterface>(nullptr, Material, nullptr, LOAD_NoWarn | LOAD_Quiet))
	{
		C->SetMaterial(0, M);
	}
	C->SetVisibility(false);
	C->RegisterComponent();
	return C;
}

void FKineticTagOut::BuildHiding()
{
	// The sun: the level's directional light shines along its forward vector.
	if (TActorIterator<ADirectionalLight> It(&World); It)
	{
		ToSun = -It->GetActorForwardVector();
	}
	// A rolled tarp at the side of each grate, lashed under its deck.
	for (int32 T = 0; T < 3; ++T)
	{
		for (const double XM : GrateRows[T].X)
		{
			const FVector Over(XM * 100.0, -GrateRows[T].YM * 100.0, 600.0);
			const FVector Deck = Ground(Over, 350.0);
			const double DeckZ = Deck.Z > 200.0 && Deck.Z < 500.0 ? Deck.Z : 350.0;
			FTarp Tarp;
			Tarp.Trench = T;
			Tarp.Lashing = FVector(Over.X + 90.0, Over.Y, DeckZ - 20.0);
			Tarp.Curtain = FBox(FVector(Over.X + 88.0, Over.Y - 300.0, -400.0), FVector(Over.X + 92.0, Over.Y + 300.0, DeckZ - 30.0));
			if (AStaticMeshActor* Rolled = Shape(Cylinder, Tarp.Lashing, FVector(0.3, 0.3, 5.8), TarpBlue))
			{
				Rolled->SetActorRotation(FRotator(0.0, 0.0, 90.0));  // Along the grate, across the trench
				Tarp.Rolled = Rolled;
			}
			if (AStaticMeshActor* Hung = Shape(Cube, Tarp.Curtain.GetCenter(), Tarp.Curtain.GetSize() / 100.0, TarpBlue))
			{
				// It stops sightlines (and so reads and calls), not riggers.
				UStaticMeshComponent* C = Hung->GetStaticMeshComponent();
				C->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
				C->SetCollisionResponseToAllChannels(ECR_Ignore);
				C->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
				Tarp.Hung = Hung;
			}
			Tarps.Add(Tarp);
		}
	}
	// Heat haze over the press frame, and over each Hauler's exhaust while it runs.
	PressHaze = Shape(Sphere, PressHazeCm, FVector(PressHazeRadiusM * 2.0), FLinearColor::Black, nullptr, HazeMaterial);
	for (TWeakObjectPtr<AStaticMeshActor>& Haze : ExhaustHaze)
	{
		Haze = Shape(Sphere, FVector::ZeroVector, FVector(ExhaustHazeRadiusM * 2.0), FLinearColor::Black, nullptr, HazeMaterial);
		if (Haze.IsValid())
		{
			Haze->SetActorHiddenInGame(true);
		}
	}
	ResetTarps();
	UE_LOG(LogKineticTagOut, Log, TEXT("Tag-Out hiding: %d tarps, the sun toward %s"), Tarps.Num(), *ToSun.ToCompactString());
}

void FKineticTagOut::ResetTarps()
{
	for (FTarp& Tarp : Tarps)
	{
		Tarp.bDown = Tarp.bCut = false;
	}
}

bool FKineticTagOut::SwingAtTarp(const FVector& Eye, const FVector& Look, double ReachCm)
{
	const double Cone = (Tables() ? Tables()->Cutter(Kinetic::ECutterVariant::Standard).ConeDeg * 0.5 : 35.0) + 15.0;
	for (FTarp& Tarp : Tarps)
	{
		if (Tarp.bCut)
		{
			continue;
		}
		if (!Tarp.bDown)
		{
			if (FVector::Dist(Tarp.Lashing, Eye) <= ReachCm + 80.0 && DegreesBetween(Look, Tarp.Lashing - Eye) <= Cone)
			{
				Tarp.bDown = true;  // The lashing cut: it drops across the trench
				++Dropped;
				Cue(TEXT("Snap"), &Tarp.Lashing);
				Say(TEXT("A tarp drops across the trench."));
				return true;
			}
			continue;
		}
		const FVector Near = Tarp.Curtain.GetClosestPointTo(Eye);
		if (FVector::Dist(Near, Eye) <= ReachCm + 50.0)
		{
			Tarp.bCut = true;  // Cut through
			Cue(TEXT("Snap"), &Near);
			return true;
		}
	}
	return false;
}

void FKineticTagOut::Hiding()
{
	// The sun and the hot steel, for glare, the sun in a caller's eyes and the haze.
	TheMatch->SetSun(ToCoreDirection(ToSun));
	std::vector<Kinetic::FTagHaze> Zones{{ToCore(PressHazeCm), PressHazeRadiusM}};
	TActorIterator<AKineticYardDirector> Found(&World);
	AKineticYardDirector* Director = Found ? *Found : nullptr;
	for (int32 T = 0; T < 3; ++T)
	{
		const AKineticKinematic* Hauler = Director ? Director->Hauler(T) : nullptr;
		const bool bRunning = Hauler && Director->IsHaulerRunning(T);
		if (AStaticMeshActor* Haze = ExhaustHaze[T].Get())
		{
			Haze->SetActorHiddenInGame(!bRunning);
			if (bRunning)
			{
				const FVector Exhaust = Hauler->GetActorLocation() + FVector(0.0, 0.0, 350.0);
				Haze->SetActorLocation(Exhaust);
				Zones.push_back({ToCore(Exhaust), ExhaustHazeRadiusM});
			}
		}
		// A Hauler tears through a tarp hanging in its trench.
		for (FTarp& Tarp : Tarps)
		{
			if (bRunning && Tarp.Trench == T && Tarp.bDown && !Tarp.bCut && FMath::Abs(Hauler->GetActorLocation().X - Tarp.Lashing.X) < 800.0)
			{
				Tarp.bCut = true;
			}
		}
	}
	TheMatch->SetHaze(MoveTemp(Zones));
	for (FTarp& Tarp : Tarps)
	{
		if (AStaticMeshActor* Rolled = Tarp.Rolled.Get())
		{
			Rolled->SetActorHiddenInGame(Tarp.bDown);
		}
		if (AStaticMeshActor* Hung = Tarp.Hung.Get())
		{
			const bool bHanging = Tarp.bDown && !Tarp.bCut;
			Hung->SetActorHiddenInGame(!bHanging);
			Hung->SetActorEnableCollision(bHanging);
		}
	}
	// The dust clouds.
	const std::vector<Kinetic::FTagCloud>& Clouds = TheMatch->Clouds();
	while (CloudProps.Num() < static_cast<int32>(Clouds.size()))
	{
		CloudProps.Add(Shape(Sphere, FVector::ZeroVector, FVector(TheMatch->Rules().DustRadiusM * 2.0), FLinearColor(0.4f, 0.35f, 0.28f),
		                     nullptr, DustMaterial));
	}
	for (int32 I = 0; I < CloudProps.Num(); ++I)
	{
		if (AStaticMeshActor* Cloud = CloudProps[I].Get())
		{
			const bool bOn = I < static_cast<int32>(Clouds.size());
			Cloud->SetActorHiddenInGame(!bOn);
			if (bOn)
			{
				Cloud->SetActorLocation(ToCm(Clouds[I].At));
			}
		}
	}
}

void FKineticTagOut::Cue(const TCHAR* Name, const FVector* At, double InnerCm, float FalloffCm)
{
	USoundBase* Sound = LoadCue(Name);
	if (!Sound)
	{
		return;  // Not imported (make import-audio): silent
	}
	++Sounds;
	if (At)
	{
		UGameplayStatics::PlaySoundAtLocation(&World, Sound, *At, FRotator::ZeroRotator, 1.0f, 1.0f, 0.0f, Falloff(InnerCm, FalloffCm));
	}
	else
	{
		UGameplayStatics::PlaySound2D(&World, Sound);
	}
}

UAudioComponent* FKineticTagOut::CueOn(const TCHAR* Name, USceneComponent* On, double InnerCm, float FalloffCm)
{
	USoundBase* Sound = LoadCue(Name);
	if (!Sound || !On)
	{
		return nullptr;
	}
	++Sounds;
	return UGameplayStatics::SpawnSoundAttached(Sound, On, NAME_None, FVector::ZeroVector, EAttachLocation::KeepRelativeOffset, true, 1.0f, 1.0f,
	                                            0.0f, Falloff(InnerCm, FalloffCm));
}

void FKineticTagOut::Holds()
{
	// The plant's clip tone across the pad, the return's tone at the beacon, while they are held.
	for (int32 Slot = 0; Slot < Seats.Num(); ++Slot)
	{
		FSeat& Seat = Seats[Slot];
		const Kinetic::FTagRigger& R = TheMatch->Rigger(Slot);
		const int32 Want = R.UseOn == Kinetic::FTagOutMatch::UsePlant                ? 0
		                   : R.UseOn == Kinetic::FTagOutMatch::UseBeacon + R.Team ? 1
		                                                                            : -1;
		if (Want == Seat.HoldSoundFor)
		{
			continue;
		}
		if (UAudioComponent* Old = Seat.HoldSound.Get())
		{
			Old->Stop();
		}
		Seat.HoldSound = nullptr;
		Seat.HoldSoundFor = Want;
		if (Want >= 0 && Rigger(Slot))
		{
			Seat.HoldSound = CueOn(Want == 0 ? TEXT("PlantTone") : TEXT("ReturnTone"), Rigger(Slot)->GetRootComponent(), 600.0, 3000.0f);
		}
	}
}

void FKineticTagOut::BuildSite()
{
	for (int32 Team = 0; Team < 2; ++Team)
	{
		const FLinearColor Tint = AKineticCharacter::TeamTint(Team);
		Shape(Cylinder, Pads[Team] + FVector(0.0, 0.0, 1.0), FVector(4.0, 4.0, 0.02), Tint * 0.6f);  // The 4 m pad
		Shape(Cylinder, Pads[Team] + FVector(0.0, 0.0, 40.0), FVector(0.25, 0.25, 0.8), Grey);        // The stand
		FBeaconProp& B = BeaconProps[Team];
		B.Body = Shape(Cube, Pads[Team] + FVector(0.0, 0.0, BeaconAboveGroundCm), FVector(0.4, 0.4, 0.55), Tint);
		B.Reflector = Shape(Sphere, Pads[Team] + FVector(0.0, 0.0, BeaconAboveGroundCm + 40.0), FVector(0.16), FLinearColor(4.0f, 4.0f, 3.6f),
		                    nullptr, GlintMaterial);
		if (AStaticMeshActor* Body = B.Body.Get())
		{
			// The flag panel: the carrier's back pip, twice the size (5.3).
			UTextRenderComponent* Flag = NewObject<UTextRenderComponent>(Body, TEXT("Flag"));
			Flag->SetupAttachment(Body->GetRootComponent());
			Flag->SetUsingAbsoluteScale(true);
			Flag->SetRelativeLocation(FVector(21.0, 0.0, 0.0));
			Flag->SetHorizontalAlignment(EHTA_Center);
			Flag->SetVerticalAlignment(EVRTA_TextCenter);
			Flag->SetWorldSize(44.0f);
			Flag->SetTextRenderColor(FColor(255, 232, 90));
			Flag->SetText(FText::GetEmpty());
			Flag->RegisterComponent();
			B.Flag = Flag;
		}
	}
	for (const FVector& At : LockerCm)
	{
		Shape(Cube, At + FVector(0.0, 0.0, 100.0), FVector(0.8, 0.6, 2.0), Grey);
		UMaterialInstanceDynamic* Light = nullptr;
		Shape(Sphere, At + FVector(0.0, 0.0, 215.0), FVector(0.22), FLinearColor::Green, &Light);
		LockerLights.Add(Light);
		LockerRedUntil.Add(-1.0);
	}
}

void FKineticTagOut::SpawnRiggers()
{
	for (int32 Slot = 0; Slot < Roster.Num(); ++Slot)
	{
		const FTransform At(PadFacing[TeamOf(Slot)], Pads[TeamOf(Slot)] + FVector(0.0, 0.0, CapsuleAboveGroundCm));
		AKineticCharacter* R = World.SpawnActorDeferred<AKineticCharacter>(AKineticCharacter::StaticClass(), At, nullptr, nullptr,
		                                                                   ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
		R->Harness = KineticHarness::FromCore(Roster[Slot].Harness);
		R->FinishSpawning(At);
		R->SetCrew(TeamOf(Slot));
		R->SetTagOut(true);
		FSeat Seat;
		Seat.Character = R;
		const int32 Place = Slot % CrewSize;
		Seat.Role = Place == 1 ? ERole::Runner : Place == 3 ? ERole::Anchor : ERole::Hunter;
		Seat.Random.Emplace(Settings.Seed ^ 0x7A6D3C1Bull, Slot);
		FActorSpawnParameters Params;
		Params.ObjectFlags |= RF_Transient;
		AKineticBotController* Brain = World.SpawnActor<AKineticBotController>(Params);
		Brain->Init(Slot, Settings.Seed, Roster[Slot].Harness);
		Seat.Brain = Brain;
		if (Slot != HumanSlot())
		{
			Brain->Possess(R);
			R->SetTakesLadders(false);
			UCharacterMovementComponent* Movement = R->GetCharacterMovement();
			Movement->AvoidanceConsiderationRadius = 300.0f;
			Movement->SetAvoidanceEnabled(true);
		}
		for (int32 I = 0; I < Kinetic::PlateCount; ++I)
		{
			UTextRenderComponent* Text = NewObject<UTextRenderComponent>(R, *FString::Printf(TEXT("Pip%d"), I));
			Text->SetupAttachment(R->GetRootComponent());
			Text->SetRelativeLocationAndRotation(PlateTextAt[I], FRotator(0.0, PlateTextYaw[I], 0.0));
			Text->SetHorizontalAlignment(EHTA_Center);
			Text->SetVerticalAlignment(EVRTA_TextCenter);
			Text->SetWorldSize(22.0f);
			Text->SetTextRenderColor(FColor(255, 232, 90));
			Text->SetText(FText::GetEmpty());
			Text->SetOwnerNoSee(true);
			Text->SetCastShadow(false);
			Text->RegisterComponent();
			Seat.Plates[I] = Text;
			// The sun's flare off this plate, to the player's eyes.
			Seat.Flares[I] = Part(R, R->GetRootComponent(), Sphere, GlintMaterial, PlateTextAt[I] * 1.08, FVector(0.08));
		}
		// The sighting level's lens glint, at the eyes (2.2), for everyone else to see.
		Seat.Glint = Part(R, R->Camera, Sphere, GlintMaterial, FVector(18.0, 0.0, 0.0), FVector(0.05));
		if (UStaticMeshComponent* Glint = Seat.Glint.Get())
		{
			Glint->SetOwnerNoSee(true);
		}
		Seats.Add(MoveTemp(Seat));
	}
}

void FKineticTagOut::AddHuman(APlayerController* Player)
{
	AKineticCharacter* Mine = Rigger(0);
	if (!Player || !Mine)
	{
		return;
	}
	if (Settings.bHumanPlays)
	{
		Player->Possess(Mine);
	}
	else
	{
		Player->SetViewTarget(Mine);
	}
}

void FKineticTagOut::PlaceAtPad(int32 Slot)
{
	AKineticCharacter* R = Rigger(Slot);
	UKineticSpoolWorld* S = Spool();
	if (!R || !S)
	{
		return;
	}
	// The crew in a line across the pad's facing, a step in front of the stand.
	const int32 Team = TeamOf(Slot);
	const FVector F = PadFacing[Team].Vector();
	const FVector Right = FRotationMatrix(PadFacing[Team]).GetUnitAxis(EAxis::Y);
	const double Across = (static_cast<double>(Slot % CrewSize) - 1.5) * 250.0;
	S->PlaceStanding(R, Pads[Team] + F * 250.0 + Right * Across + FVector(0.0, 0.0, CapsuleAboveGroundCm), PadFacing[Team]);
	FSeat& Seat = Seats[Slot];
	Seat.ThrowAt = -1.0;
	Seat.bStripOpen = false;
}

void FKineticTagOut::StartMatch()
{
	MatchSeed = Settings.Seed + static_cast<uint64>(Started) * 0x9E3779B97F4A7C15ull;
	std::vector<FVec3> Lockers;
	for (const FVector& At : LockerCm)
	{
		Lockers.push_back(ToCore(At + FVector(0.0, 0.0, LockerAboveGroundCm)));
	}
	const std::array<FVec3, 2> PadsM{ToCore(Pads[0] + FVector(0.0, 0.0, BeaconAboveGroundCm)),
	                                 ToCore(Pads[1] + FVector(0.0, 0.0, BeaconAboveGroundCm))};
	TheMatch = MakeUnique<Kinetic::FTagOutMatch>(std::vector<Kinetic::FMatchPlayer>(Roster.GetData(), Roster.GetData() + Roster.Num()),
	                                             PadsM, Lockers, MatchSeed, Settings.Rules);
	for (int32 Slot = 0; Slot < Roster.Num(); ++Slot)
	{
		// The player's choice; the bots by job: the runner greases, the anchor and the second
		// hunter carry dust.
		const int32 Place = Slot % CrewSize;
		const Kinetic::ETagGadget Gadget = Slot == HumanSlot() ? Settings.HumanGadget
		                                   : Place == 3 || Place == 2 ? Kinetic::ETagGadget::Dust
		                                                              : Kinetic::ETagGadget::Grease;
		TheMatch->SetLoadout(Slot, Gadget);
	}
	ResetTarps();
	++Started;
	MatchOverAt = SirenAt = MidMatchAt = OvertimeAt = -1.0;
	NextRevealAt = 0.0;
	for (int32 Slot = 0; Slot < Seats.Num(); ++Slot)
	{
		PlaceAtPad(Slot);
		FSeat& Seat = Seats[Slot];
		Seat.Sight = FKineticTagSight();
		// The clock starts again from the drop.
		Seat.NextSwingAt = Seat.SpunUntil = Seat.BotSwingAt = Seat.WatchUntil = Seat.MoveUntil = Seat.GambleAt = 0.0;
		Seat.ReactTo = INDEX_NONE;
		Seat.DustAt = -1.0;
	}
	for (int32 Team = 0; Team < 2; ++Team)
	{
		BeaconProps[Team].At = ToCm(TheMatch->Beacon(Team).Stand);
		BeaconProps[Team].bFlying = false;
	}
	for (double& Red : LockerRedUntil)
	{
		Red = -1.0;
	}
	Say(TEXT("Crews on the claim."));
}

void FKineticTagOut::Tick()
{
	UKineticSpoolWorld* S = Spool();
	if (!S || !TheMatch)
	{
		return;
	}
	S->SetLinesAllowed(false);
	// The arena's Stalls since the Spool's last step: those riggers go to the reserve.
	for (const FKineticSpoolEvent& E : S->TakeEvents())
	{
		const int32 Slot = E.Kind == EKineticSpoolEventKind::Stall ? SlotOf(E.Target.Get()) : INDEX_NONE;
		if (Slot != INDEX_NONE)
		{
			TheMatch->Stall(Slot);
		}
	}
	if (TheMatch->Phase() == Kinetic::ETagPhase::Over && MatchOverAt >= 0.0 && TheMatch->Time() - MatchOverAt >= RestartSeconds)
	{
		StartMatch();
		return;
	}
	for (int32 Slot = 0; Slot < Seats.Num(); ++Slot)
	{
		if (Slot != HumanSlot())
		{
			Think(Slot);
		}
	}
	Poses();
	Hiding();
	if (TheMatch->Time() >= NextRevealAt - 1e-6)
	{
		Reveal();
		NextRevealAt = TheMatch->Time() + 1.0 / RevealHz;
	}
	Inputs();
	std::vector<bool> InView(Seats.Num(), false);
	for (int32 Slot = 0; Slot < Seats.Num(); ++Slot)
	{
		const Kinetic::FTagRigger& R = TheMatch->Rigger(Slot);
		InView[Slot] = R.Call == Kinetic::ECallState::Locking && InCrosshair(Slot, R.CallTarget, CrosshairHoldDeg);
	}
	TheMatch->Step(UKineticSpoolWorld::Dt, InView);
	Events();
	Beacons();
	Show();
	Holds();
}

void FKineticTagOut::Poses()
{
	for (int32 Slot = 0; Slot < Seats.Num(); ++Slot)
	{
		const AKineticCharacter* R = Rigger(Slot);
		if (!R)
		{
			continue;
		}
		const FRotator Yaw(0.0, R->GetActorRotation().Yaw, 0.0);
		Kinetic::FTagPose Pose;
		Pose.Chest = ToCore(ChestCm(Slot));
		Pose.Eye = ToCore(R->Camera->GetComponentLocation());
		Pose.Forward = ToCoreDirection(Yaw.Vector());
		Pose.Right = ToCoreDirection(FRotationMatrix(Yaw).GetUnitAxis(EAxis::Y));
		TheMatch->SetPose(Slot, Pose, R->GetVelocity().Size2D() / 100.0, Seats[Slot].bLevel);
	}
}

bool FKineticTagOut::Clear(const FVector& From, const FVector& To, const AActor* Viewer, const AActor* Target) const
{
	FCollisionQueryParams Params(SCENE_QUERY_STAT(KineticTagSight), false, Viewer);
	Params.AddIgnoredActor(Target);
	for (const FBeaconProp& B : BeaconProps)
	{
		Params.AddIgnoredActor(B.Body.Get());
	}
	FHitResult Hit;
	return !World.LineTraceSingleByChannel(Hit, From, To, ECC_Visibility, Params);
}

void FKineticTagOut::Reveal()
{
	// 10.1: each rigger's eyes to the plates in front of them. Enemies' plates go on the crew's
	// board; the watched rigger's view of everyone (a little early: 10 degrees and 5 m) decides
	// which digits are drawn.
	Kinetic::FTagOutRules Early = Settings.Rules;
	Early.PlateConeDegrees += 10.0;
	Early.NakedRangeM += 5.0;
	Early.LevelRangeM += 5.0;
	Early.CarrierBackRangeM += 5.0;
	const std::array<bool, Kinetic::PlateCount> Open{true, true, true};
	for (int32 V = 0; V < Seats.Num(); ++V)
	{
		const AKineticCharacter* Viewer = Rigger(V);
		const bool bLocal = V == ViewSlot();
		if (!Viewer || !TheMatch->Rigger(V).bStanding)
		{
			if (bLocal)
			{
				for (FSeat& Seat : Seats)
				{
					Seat.Sight = FKineticTagSight();
				}
			}
			continue;
		}
		const FVector Eye = Viewer->Camera->GetComponentLocation();
		const FVector Look = Viewer->GetControlRotation().Vector();
		const bool bLevel = TheMatch->LevelReady(V);
		const double HalfView = bLevel ? 20.0 : 50.0;
		for (int32 T = 0; T < Seats.Num(); ++T)
		{
			const bool bEnemy = TeamOf(T) != TeamOf(V);
			if (T == V || (!bEnemy && !bLocal))
			{
				continue;
			}
			FKineticTagSight Sight;
			const AKineticCharacter* Target = Rigger(T);
			const FVector Chest = ChestCm(T);
			const bool bInFront = Target && TheMatch->Rigger(T).bStanding &&
			                      FMath::RadiansToDegrees(FMath::Acos(FMath::Clamp(FVector::DotProduct(Look, (Chest - Eye).GetSafeNormal()), -1.0, 1.0))) <=
			                          HalfView;
			if (bInFront)
			{
				const Kinetic::FTagPose& Pose = TheMatch->Rigger(T).Pose;
				const bool bCarrier = TheMatch->Carrying(T) != Kinetic::NoPlayer;
				const uint8 Strict = bEnemy ? Kinetic::ReadablePlates(ToCore(Eye), bLevel, Pose, Open, Settings.Rules, bCarrier) : 0;
				const uint8 Margin = bLocal ? Kinetic::ReadablePlates(ToCore(Eye), bLevel, Pose, Open, Early, bCarrier) : 0;
				uint8 Seen = 0;
				for (int32 I = 0; I < Kinetic::PlateCount; ++I)
				{
					if (((Strict | Margin) & (1u << I)) &&
					    Clear(Eye, UKineticSpoolWorld::PlatePosition(Target, CorePlate(I)), Viewer, Target))
					{
						Seen |= static_cast<uint8>(1u << I);
					}
				}
				if (Strict & Seen)
				{
					TheMatch->Show(V, T, Strict & Seen);
				}
				Sight.bVisible = Clear(Eye, Chest, Viewer, Target);
				Sight.Facing = Margin & Seen;
				Sight.Plates = TheMatch->Legible(V, T, Sight.Facing);
				for (int32 I = 0; I < Kinetic::PlateCount; ++I)
				{
					if ((Sight.Facing & (1u << I)) && TheMatch->Glares(V, T, static_cast<Kinetic::ETagPlate>(I)))
					{
						Sight.Flaring |= static_cast<uint8>(1u << I);
					}
				}
			}
			if (bLocal)
			{
				Seats[T].Sight = Sight;
			}
		}
	}
}

bool FKineticTagOut::InCrosshair(int32 Caller, int32 Target, double MaxDeg) const
{
	const AKineticCharacter* Me = Rigger(Caller);
	const AKineticCharacter* Them = Rigger(Target);
	if (!Me || !Them || Target < 0)
	{
		return false;
	}
	const FVector Eye = Me->Camera->GetComponentLocation();
	const FVector Chest = ChestCm(Target);
	const double Distance = FVector::Dist(Eye, Chest);
	if (Distance > Settings.Rules.CallRangeM * 100.0)
	{
		return false;
	}
	// The torso under the crosshair: about 45 cm either side of the chest, or MaxDeg.
	const double Allowed = FMath::Max(MaxDeg, FMath::RadiansToDegrees(FMath::Atan2(45.0, Distance)));
	const double Off = FMath::RadiansToDegrees(
	    FMath::Acos(FMath::Clamp(FVector::DotProduct(Me->GetControlRotation().Vector(), (Chest - Eye).GetSafeNormal()), -1.0, 1.0)));
	return Off <= Allowed && Clear(Eye, Chest, Me, Them);
}

int32 FKineticTagOut::UnderCrosshair(int32 Caller) const
{
	const AKineticCharacter* Me = Rigger(Caller);
	if (!Me)
	{
		return INDEX_NONE;
	}
	int32 Best = INDEX_NONE;
	double BestOff = TNumericLimits<double>::Max();
	const FVector Eye = Me->Camera->GetComponentLocation();
	for (int32 T = 0; T < Seats.Num(); ++T)
	{
		if (TeamOf(T) == TeamOf(Caller) || !TheMatch->Rigger(T).bStanding || !InCrosshair(Caller, T, 4.0))
		{
			continue;
		}
		const double Off = FVector::DotProduct(Me->GetControlRotation().Vector(), (ChestCm(T) - Eye).GetSafeNormal());
		if (-Off < BestOff)
		{
			BestOff = -Off;
			Best = T;
		}
	}
	return Best;
}

void FKineticTagOut::Inputs()
{
	const double Now = TheMatch->Time();
	const Kinetic::FTagOutRules& Rules = TheMatch->Rules();
	for (int32 Slot = 0; Slot < Seats.Num(); ++Slot)
	{
		AKineticCharacter* R = Rigger(Slot);
		if (!R)
		{
			continue;
		}
		FSeat& Seat = Seats[Slot];
		const AKineticCharacter::FTagInput In = R->TakeTagInput();
		if (!TheMatch->Rigger(Slot).bStanding)
		{
			Seat.bStripOpen = false;
			Seat.bLevel = false;
			Seat.ThrowAt = -1.0;
			Seat.DustAt = -1.0;
			continue;
		}
		Seat.bLevel = In.bLevel;
		if (In.bClear)
		{
			TheMatch->ClearCall(Slot);
			Seat.bStripOpen = false;
		}
		for (const int32 Pip : In.Pips)
		{
			TheMatch->EnterPip(Slot, Pip);
			Seat.bStripOpen = true;
		}
		if (In.bCall)
		{
			if (TheMatch->Rigger(Slot).Entered.size() == Kinetic::PlateCount)
			{
				const int32 Target = UnderCrosshair(Slot);
				if (Target != INDEX_NONE && TheMatch->SendCall(Slot, Target, true))
				{
					++Calls;
					Seat.bStripOpen = false;
				}
			}
			else
			{
				Seat.bStripOpen = true;
			}
		}
		if (In.bUse)
		{
			TheMatch->HoldUse(Slot, UKineticSpoolWorld::Dt);
		}
		const bool bCarrying = TheMatch->Carrying(Slot) != Kinetic::NoPlayer;
		if (In.bThrow && bCarrying && Seat.ThrowAt < 0.0)
		{
			Seat.ThrowAt = Now + ThrowWindupSeconds;
		}
		if (!bCarrying)
		{
			Seat.ThrowAt = -1.0;
		}
		else if (Seat.ThrowAt >= 0.0 && Now >= Seat.ThrowAt)
		{
			Seat.ThrowAt = -1.0;
			Throw(Slot);
		}
		if (In.bSwing)
		{
			Swing(Slot);
		}
		if (In.bGadget)
		{
			// A rag goes on now; a bag is cut open with a Cutter swing.
			const Kinetic::FTagRigger& Me = TheMatch->Rigger(Slot);
			if (Me.Gadget == Kinetic::ETagGadget::Grease)
			{
				TheMatch->UseGadget(Slot);
			}
			else if (Seat.DustAt < 0.0 && Me.GadgetsLeft > 0 && !bCarrying)
			{
				const Kinetic::FKineticTables* T = Tables();
				Seat.DustAt = Now + (T ? T->Cutter(Kinetic::ECutterVariant::Standard).Swing : 0.25);
			}
		}
		if (Seat.DustAt >= 0.0 && Now >= Seat.DustAt)
		{
			Seat.DustAt = -1.0;
			TheMatch->UseGadget(Slot, ToCore(R->GetActorLocation() + FVector(0.0, 0.0, 60.0)));
		}
		const bool bNowCarrying = TheMatch->Carrying(Slot) != Kinetic::NoPlayer;
		Seat.bCarrying = bNowCarrying;
		R->SetTagSlowdown(bNowCarrying ? static_cast<float>(Rules.CarrierSprintScale) : 1.0f,
		                  bNowCarrying ? static_cast<float>(Rules.CarrierLadderScale) : 1.0f, Seat.bLevel);
	}
}

void FKineticTagOut::Swing(int32 Slot)
{
	// 4: a Cutter swing on a plate staggers the target 0.3 s and spins them 70 degrees.
	const Kinetic::FKineticTables* T = Tables();
	AKineticCharacter* Me = Rigger(Slot);
	UKineticSpoolWorld* S = Spool();
	FSeat& Seat = Seats[Slot];
	const double Now = TheMatch->Time();
	if (!T || !Me || !S || Now < Seat.NextSwingAt || !TheMatch->CanCall())
	{
		return;
	}
	const Kinetic::FCutterTuning& Cutter = T->Cutter(Kinetic::ECutterVariant::Standard);
	Seat.NextSwingAt = Now + Cutter.Swing + Cutter.Recovery;
	++Swung;
	const FVector Eye = Me->Camera->GetComponentLocation();
	const FVector Look = Me->GetControlRotation().Vector();
	int32 Hit = INDEX_NONE;
	double Nearest = TNumericLimits<double>::Max();
	for (int32 O = 0; O < Seats.Num(); ++O)
	{
		const AKineticCharacter* Them = Rigger(O);
		if (!Them || TeamOf(O) == TeamOf(Slot) || !TheMatch->Rigger(O).bStanding)
		{
			continue;
		}
		const FVector To = ChestCm(O) - Eye;
		const double Distance = To.Size();
		const double Off = FMath::RadiansToDegrees(FMath::Acos(FMath::Clamp(FVector::DotProduct(Look, To.GetSafeNormal()), -1.0, 1.0)));
		if (Distance <= Cutter.Reach * 100.0 + 40.0 && Off <= Cutter.ConeDeg * 0.5 && Distance < Nearest)
		{
			Nearest = Distance;
			Hit = O;
		}
	}
	AKineticCharacter* Them = Rigger(Hit);
	if (!Them)
	{
		SwingAtTarp(Eye, Look, Cutter.Reach * 100.0);  // A lashing, or a tarp to cut through
		return;
	}
	const FVector Clang = ChestCm(Hit);
	Cue(TEXT("PlateClang"), &Clang);
	if (Now < Seats[Hit].SpunUntil)
	{
		return;  // Still turning from the last one: no stun-lock (PROPOSED)
	}
	Seats[Hit].SpunUntil = Now + CutterSpinGuardSeconds;
	S->Hit(Them, Kinetic::EOutcome::Stagger, Kinetic::EStallCause::PlayerImpact, CutterStaggerSeconds);
	// The spin that turns their back toward a teammate on the swinger's left (PROPOSED reading
	// of 4: whichever way round brings the back nearer that side).
	const FRotator MyYaw(0.0, Me->GetActorRotation().Yaw, 0.0);
	const FVector MateSide = Me->GetActorLocation() - FRotationMatrix(MyYaw).GetUnitAxis(EAxis::Y) * 500.0;
	const FVector ToMate = (MateSide - Them->GetActorLocation()).GetSafeNormal2D();
	const double TheirYaw = Them->GetActorRotation().Yaw;
	const auto BackToward = [&](double Spin) { return FVector::DotProduct(-FRotator(0.0, TheirYaw + Spin, 0.0).Vector(), ToMate); };
	const double Spin = BackToward(CutterSpinDeg) >= BackToward(-CutterSpinDeg) ? CutterSpinDeg : -CutterSpinDeg;
	Them->SetActorRotation(FRotator(0.0, TheirYaw + Spin, 0.0));
	if (AController* C = Them->GetController())
	{
		C->SetControlRotation(C->GetControlRotation() + FRotator(0.0, Spin, 0.0));
	}
	UE_LOG(LogKineticTagOut, Log, TEXT("%s's Cutter spins %s %+.0f degrees"), *Who(Slot), *Who(Hit), Spin);
}

void FKineticTagOut::Throw(int32 Slot)
{
	const int32 Team = TheMatch->Carrying(Slot);
	AKineticCharacter* Me = Rigger(Slot);
	if (Team == Kinetic::NoPlayer || !Me)
	{
		return;
	}
	TheMatch->Throw(Slot);
	// About 12 m on the flat: 10 m/s out, 5.9 m/s up.
	const FVector Out = FRotator(0.0, Me->GetControlRotation().Yaw, 0.0).Vector();
	FBeaconProp& B = BeaconProps[Team];
	B.At = ChestCm(Slot) + Out * 60.0;
	B.Velocity = Out * 1000.0 + FVector(0.0, 0.0, 590.0);
	B.bFlying = true;
	Say(FString::Printf(TEXT("%s threw the beacon."), *Who(Slot)));
}

void FKineticTagOut::Think(int32 Slot)
{
	FSeat& Seat = Seats[Slot];
	AKineticCharacter* Me = Seat.Character.Get();
	AKineticBotController* Brain = Seat.Brain.Get();
	if (!Me || !Brain || !Seat.Random || Me->GetController() != Brain)
	{
		return;
	}
	AKineticCharacter::FTagInput& In = Me->TagInput();
	In.bUse = false;
	In.bLevel = false;
	Me->Move(FVector2D::ZeroVector);
	const Kinetic::FTagOutMatch& M = *TheMatch;
	if (!bBotsActive || !M.CanMove(Slot) || Me->IsStunned())
	{
		return;
	}
	Kinetic::FBotRandom& Random = *Seat.Random;
	const Kinetic::FTagOutRules& Rules = M.Rules();
	const Kinetic::FTagRigger& R = M.Rigger(Slot);
	const double Now = M.Time();
	const int32 Mine = R.Team;
	const int32 Theirs = 1 - Mine;
	const FVector At = Me->GetActorLocation();
	const FVector Eye = Me->Camera->GetComponentLocation();

	// Called: turn to face the caller, when this bot is quick enough this time (the lock's escape).
	const int32 Caller = M.CalledBy(Slot);
	if (Caller != Kinetic::NoPlayer)
	{
		if (Seat.ReactTo != Caller)
		{
			Seat.ReactTo = Caller;
			Seat.ReactAt = Now + Random.Uniform(0.2, 0.5);
			Seat.bWillReact = Random.Uniform() < 0.5;
		}
		if (Seat.bWillReact && Now >= Seat.ReactAt)
		{
			Brain->Face(*Me, ChestCm(Caller));
			return;
		}
		if (!Seat.bWillReact && R.Gadget == Kinetic::ETagGadget::Dust && R.GadgetsLeft > 0 && Seat.DustAt < 0.0 && Now >= Seat.ReactAt)
		{
			In.bGadget = true;  // No time to turn: cut the bag and stand in it
			return;
		}
	}
	else
	{
		Seat.ReactTo = INDEX_NONE;
		Seat.DustAt = -1.0;
	}
	if (R.Call == Kinetic::ECallState::Locking)
	{
		Brain->Face(*Me, ChestCm(R.CallTarget));  // Hold the lock
		return;
	}
	if (Seat.ThrowAt >= 0.0)
	{
		return;  // Winding up
	}

	const auto HoldAt = [&](const FVector& Point) {
		if (FVector::Dist2D(At, Point) < 110.0 && FMath::Abs(At.Z - Point.Z) < 220.0)
		{
			In.bUse = true;
			return;
		}
		Brain->GoTo(*Me, Point);
	};
	const Kinetic::FTagBeacon& Ours = M.Beacon(Mine);
	const Kinetic::FTagBeacon& Enemy = M.Beacon(Theirs);
	if (M.Carrying(Slot) == Theirs)
	{
		HoldAt(Pads[Mine]);  // Home, and plant
		return;
	}
	if (Ours.State == Kinetic::EBeaconState::Dropped &&
	    (Seat.Role == ERole::Anchor || FVector::Dist(At, ToCm(Ours.At)) < 2500.0))
	{
		HoldAt(ToCm(Ours.At) - FVector(0.0, 0.0, BeaconAboveGroundCm * 0.5));
		return;
	}
	if (Enemy.State != Kinetic::EBeaconState::Carried &&
	    (Seat.Role == ERole::Runner || (Enemy.State == Kinetic::EBeaconState::Dropped && FVector::Dist(At, ToCm(Enemy.At)) < 2000.0)))
	{
		HoldAt(Enemy.State == Kinetic::EBeaconState::OnStand ? Pads[Theirs] : ToCm(Enemy.At) - FVector(0.0, 0.0, BeaconAboveGroundCm * 0.5));
		return;
	}
	// An enemy close behind: a rag on the back, or a dust bag cut to stand in.
	const bool bGreaseReady = R.Gadget == Kinetic::ETagGadget::Grease && R.SmearingUntil < 0.0 && !M.Smeared(Slot, Kinetic::ETagPlate::Back);
	const bool bDustReady = R.Gadget == Kinetic::ETagGadget::Dust && Seat.DustAt < 0.0 && !M.ThroughDust(R.Pose.Eye, R.Pose.Chest + Kinetic::FVec3{0.0, 0.0, 0.1});
	if ((bGreaseReady || bDustReady) && R.GadgetsLeft > 0 && M.Carrying(Slot) == Kinetic::NoPlayer)
	{
		for (int32 O = 0; O < Seats.Num(); ++O)
		{
			if (TeamOf(O) != Mine && M.Rigger(O).bStanding && Rigger(O) && FVector::Dist(At, Rigger(O)->GetActorLocation()) < 2500.0 &&
			    DegreesBetween(Me->GetActorForwardVector(), Rigger(O)->GetActorLocation() - At) > 110.0)
			{
				In.bGadget = true;
				return;
			}
		}
	}
	if (R.SmearingUntil >= 0.0)
	{
		return;  // Both hands on the rag
	}
	// Re-stamp once the other crew has two of these pips.
	if (M.LockersOpen() && M.Read(Theirs, Slot).Known() >= 2 && M.Carrying(Slot) == Kinetic::NoPlayer)
	{
		int32 Locker = INDEX_NONE;
		for (int32 I = 0; I < LockerCm.Num(); ++I)
		{
			if (FVector::Dist(At, LockerCm[I]) < 4500.0 && (Locker == INDEX_NONE || FVector::Dist(At, LockerCm[I]) < FVector::Dist(At, LockerCm[Locker])))
			{
				Locker = I;
			}
		}
		if (Locker != INDEX_NONE)
		{
			HoldAt(LockerCm[Locker]);
			return;
		}
	}

	// The hunt: whoever has our beacon first, then the nearest enemy standing.
	int32 Target = INDEX_NONE;
	double Best = TNumericLimits<double>::Max();
	for (int32 O = 0; O < Seats.Num(); ++O)
	{
		if (TeamOf(O) == Mine || !M.Rigger(O).bStanding || !Rigger(O))
		{
			continue;
		}
		const double D = FVector::Dist(At, Rigger(O)->GetActorLocation()) * (Ours.Carrier == O ? 0.3 : 1.0);
		if (D < Best)
		{
			Best = D;
			Target = O;
		}
	}
	if (Target == INDEX_NONE)
	{
		Brain->GoTo(*Me, Seat.Role == ERole::Anchor ? Pads[Mine] : Pads[Theirs]);
		return;
	}
	const AKineticCharacter* Them = Rigger(Target);
	const FVector Chest = ChestCm(Target);
	const double D = FVector::Dist(Eye, Chest) / 100.0;
	const bool bSight = Clear(Eye, Chest, Me, Them);
	const Kinetic::FTagRead& Read = M.Read(Mine, Target);
	if (bSight && D <= Rules.CallRangeM - 1.0 && Now >= R.CallCooldownUntil && M.CanCall())
	{
		const int32 Known = Read.Known();
		bool bGo = Known == Kinetic::PlateCount;
		if (!bGo && Known == 2 && Now >= Seat.GambleAt)
		{
			Seat.GambleAt = Now + 4.0;
			bGo = Random.Uniform() < 0.3;  // Two of three: a one-in-four guess
		}
		if (bGo)
		{
			In.bClear = true;
			In.Pips.Reset();
			for (int32 I = 0; I < Kinetic::PlateCount; ++I)
			{
				In.Pips.Add(Read.Pips[I] && !Read.bStale ? static_cast<int32>(*Read.Pips[I]) : FMath::Clamp(1 + static_cast<int32>(Random.Uniform() * 4.0), 1, 4));
			}
			In.bCall = true;
			Brain->Face(*Me, Chest);
			return;
		}
	}
	// Up close, the Cutter turns them.
	const Kinetic::FKineticTables* T = Tables();
	const double Reach = T ? T->Cutter(Kinetic::ECutterVariant::Standard).Reach : 2.0;
	if (D <= Reach && Now >= Seat.NextSwingAt && Now >= Seat.BotSwingAt && Now >= Seats[Target].SpunUntil)
	{
		Brain->Face(*Me, Chest);
		In.bSwing = true;
		Seat.BotSwingAt = Now + Random.Uniform(2.0, 3.5);
		return;
	}
	// Watch for a moment (the level up past 18 m), then move round again.
	if (bSight && D <= 30.0 && Now < Seat.WatchUntil)
	{
		Brain->Face(*Me, Chest);
		In.bLevel = D > 18.0;
		return;
	}
	if (bSight && D <= 30.0 && Now >= Seat.MoveUntil)
	{
		Seat.WatchUntil = Now + Random.Uniform(1.0, 2.0);
		Seat.MoveUntil = Seat.WatchUntil + Random.Uniform(1.5, 3.0);
		Seat.FlankSide = Random.Uniform() < 0.5 ? -1.0 : 1.0;
		Brain->Face(*Me, Chest);
		return;
	}
	// Round behind them: 10 m off their back, to one side. The anchor keeps to its pad.
	const FRotator TheirYaw(0.0, Them->GetActorRotation().Yaw, 0.0);
	FVector Goal = Them->GetActorLocation() - TheirYaw.Vector() * 1000.0 +
	               FRotationMatrix(TheirYaw).GetUnitAxis(EAxis::Y) * 500.0 * Seat.FlankSide;
	if (Seat.Role == ERole::Anchor && FVector::Dist2D(Goal, Pads[Mine]) > 1500.0)
	{
		Goal = Pads[Mine] + (Them->GetActorLocation() - Pads[Mine]).GetSafeNormal2D() * 600.0;
		if (FVector::Dist2D(At, Goal) < 200.0)
		{
			Brain->Face(*Me, Chest);
			In.bLevel = bSight && D > 18.0;
			return;
		}
	}
	Brain->GoTo(*Me, Goal);
}

void FKineticTagOut::Events()
{
	UKineticSpoolWorld* S = Spool();
	const int32 Mine = TeamOf(ViewSlot());
	const auto Whose = [this, Mine](int32 Team) {
		return Settings.bHumanPlays ? FString(Team == Mine ? TEXT("Your") : TEXT("Their"))
		                            : FString::Printf(TEXT("Crew %c's"), Team == 0 ? TEXT('A') : TEXT('B'));
	};
	for (const Kinetic::FTagEventRecord& E : TheMatch->TakeEvents())
	{
		AKineticCharacter* R = Rigger(E.Slot);
		switch (E.Kind)
		{
		case Kinetic::ETagEvent::Live:
			Say(TEXT("Live."));
			break;
		case Kinetic::ETagEvent::MidEvent:
			MidMatchAt = E.Time;
			Say(TEXT("Shift change."));
			break;
		case Kinetic::ETagEvent::Overtime:
			OvertimeAt = E.Time;
			Say(TEXT("Lockout. Carriers' codes are up; the lockers are shut."));
			break;
		case Kinetic::ETagEvent::Revealed:
			++Revealed;
			break;
		case Kinetic::ETagEvent::CallSent:
			// The target's own harness ticks fast through the lock (8).
			if (AKineticCharacter* Target = Rigger(E.Other))
			{
				if (UAudioComponent* Old = Seats[E.Other].TickSound.Get())
				{
					Old->Stop();
				}
				Seats[E.Other].TickSound = CueOn(TEXT("CallTick"), Target->GetRootComponent(), 150.0, 1500.0f);
			}
			break;
		case Kinetic::ETagEvent::LostLock:
			if (UAudioComponent* Tick = Seats.IsValidIndex(E.Other) ? Seats[E.Other].TickSound.Get() : nullptr)
			{
				Tick->Stop();
			}
			break;
		case Kinetic::ETagEvent::Smearing:
			if (R)
			{
				CueOn(TEXT("GreaseScrub"), R->GetRootComponent(), 150.0, 1200.0f);
			}
			break;
		case Kinetic::ETagEvent::Smeared:
			++Smeared;
			break;
		case Kinetic::ETagEvent::DustCloud:
		{
			++Dusted;
			const FVector At = ToCm(TheMatch->Clouds().back().At);
			Cue(TEXT("DustBurst"), &At);
			Say(FString::Printf(TEXT("%s cut a dust bag."), *Who(E.Slot)));
			break;
		}
		case Kinetic::ETagEvent::TaggedOut:
			++Tagged;
			if (UAudioComponent* Tick = Seats[E.Other].TickSound.Get())
			{
				Tick->Stop();
			}
			Say(FString::Printf(TEXT("%s tagged out %s: %s."), *Who(E.Slot), *Who(E.Other), *Code(TheMatch->Rigger(E.Other).Code)));
			if (S && Rigger(E.Other))
			{
				S->Stall(Rigger(E.Other), Kinetic::EStallCause::PlayerImpact);
			}
			break;
		case Kinetic::ETagEvent::BadCall:
			++Bad;
			if (UAudioComponent* Tick = Seats[E.Other].TickSound.Get())
			{
				Tick->Stop();
			}
			if (R)
			{
				const FVector At = R->GetActorLocation();
				Cue(TEXT("BadCallBuzz"), &At, 500.0, 5000.0f);
			}
			Say(FString::Printf(TEXT("%s's call on %s was bad: a pip is out."), *Who(E.Slot), *Who(E.Other)));
			break;
		case Kinetic::ETagEvent::Restamped:
			++Restamped;
			if (LockerRedUntil.IsValidIndex(E.Other))
			{
				LockerRedUntil[E.Other] = E.Time + 3.0;
				const FVector At = LockerCm[E.Other] + FVector(0.0, 0.0, LockerAboveGroundCm);
				Cue(TEXT("StampSlam"), &At, 1000.0, 2000.0f);  // Heard to 30 m (2.5)
			}
			Say(FString::Printf(TEXT("%s re-stamped."), *Who(E.Slot)));
			break;
		case Kinetic::ETagEvent::Stalled:
			if (S && R && S->IsStanding(R))
			{
				S->Stall(R, Kinetic::EStallCause::WorldImpact);
			}
			break;
		case Kinetic::ETagEvent::Respawned:
			PlaceAtPad(E.Slot);
			break;
		case Kinetic::ETagEvent::BeaconGrabbed:
			++Grabbed;
			Cue(TEXT("BeaconKlaxon"), nullptr);  // Both crews hear it
			Say(FString::Printf(TEXT("%s has %s beacon."), *Who(E.Slot),
			                    Settings.bHumanPlays ? *Whose(E.Team).ToLower() : *Whose(E.Team)));
			break;
		case Kinetic::ETagEvent::BeaconDropped:
			if (!BeaconProps[E.Team].bFlying)
			{
				BeaconProps[E.Team].At = ToCm(TheMatch->Beacon(E.Team).At);
				BeaconProps[E.Team].Velocity = FVector::ZeroVector;
				BeaconProps[E.Team].bFlying = true;  // It falls where they went down
				Say(FString::Printf(TEXT("%s beacon is down."), *Whose(E.Team)));
			}
			break;
		case Kinetic::ETagEvent::BeaconReturned:
			BeaconProps[E.Team].bFlying = false;
			BeaconProps[E.Team].At = ToCm(TheMatch->Beacon(E.Team).Stand);
			Say(FString::Printf(TEXT("%s beacon is home."), *Whose(E.Team)));
			break;
		case Kinetic::ETagEvent::Planted:
			Say(FString::Printf(TEXT("%s planted it."), *Who(E.Slot)));
			break;
		case Kinetic::ETagEvent::MatchOver:
			MatchOverAt = E.Time;
			++Finished;
			Say(E.Team == Kinetic::NoPlayer ? FString(TEXT("A draw."))
			    : Settings.bHumanPlays      ? FString(E.Team == Mine ? TEXT("Match: your crew.") : TEXT("Match: their crew."))
			                                : FString::Printf(TEXT("Match: crew %c."), E.Team == 0 ? TEXT('A') : TEXT('B')));
			break;
		default:
			break;
		}
	}
	// The siren five seconds before the mid-match event.
	const FKineticRoundTimes T = RoundTimes();
	if (SirenAt < 0.0 && T.Now >= T.PlannedMidMatchAt - 5.0)
	{
		SirenAt = T.Now;
	}
}

void FKineticTagOut::Beacons()
{
	const double Dt = UKineticSpoolWorld::Dt;
	for (int32 Team = 0; Team < 2; ++Team)
	{
		const Kinetic::FTagBeacon& B = TheMatch->Beacon(Team);
		FBeaconProp& P = BeaconProps[Team];
		double Yaw = 0.0;
		if (B.State == Kinetic::EBeaconState::Carried && Rigger(B.Carrier))
		{
			// On the carrier's back, the flag panel facing behind them.
			const AKineticCharacter* Carrier = Rigger(B.Carrier);
			Yaw = Carrier->GetActorRotation().Yaw + 180.0;
			P.At = Carrier->GetActorLocation() - FRotator(0.0, Carrier->GetActorRotation().Yaw, 0.0).Vector() * 45.0 + FVector(0.0, 0.0, 55.0);
			P.bFlying = false;
			// The closest approach along the walkable route (5.6), twice a second.
			if (TheMatch->Time() >= P.RouteAt)
			{
				P.RouteAt = TheMatch->Time() + 0.5;
				const UNavigationPath* Path = UNavigationSystemV1::FindPathToLocationSynchronously(&World, Carrier->GetActorLocation(), Pads[TeamOf(B.Carrier)]);
				if (Path && Path->IsValid())
				{
					TheMatch->ReportRoute(Team, Path->GetPathLength() / 100.0 - TheMatch->Rules().PadRadiusM);
				}
			}
		}
		else if (B.State == Kinetic::EBeaconState::Dropped)
		{
			if (P.bFlying)
			{
				const FVector Next = P.At + P.Velocity * Dt;
				P.Velocity.Z -= GravityCm * Dt;
				FHitResult Hit;
				FCollisionQueryParams Params(SCENE_QUERY_STAT(KineticBeacon), false, P.Body.Get());
				for (const FSeat& Seat : Seats)
				{
					Params.AddIgnoredActor(Seat.Character.Get());
				}
				if (World.LineTraceSingleByChannel(Hit, P.At, Next, ECC_Visibility, Params))
				{
					P.At = Hit.Location + FVector(0.0, 0.0, 30.0);
					P.bFlying = false;
				}
				else
				{
					P.At = Next;
				}
				TheMatch->SetBeaconAt(Team, ToCore(P.At));  // Into the pit: home at once
			}
		}
		else
		{
			P.At = ToCm(B.Stand);
		}
		if (AStaticMeshActor* Body = P.Body.Get())
		{
			Body->SetActorLocationAndRotation(P.At, FRotator(0.0, Yaw, 0.0));
		}
		if (AStaticMeshActor* Reflector = P.Reflector.Get())
		{
			// The reflector flashes every 3 s while it is carried.
			// It flashes every 3 s while carried, and only in the sun: not under the Gantry (6).
			bool bFlash = B.State == Kinetic::EBeaconState::Carried && FMath::Fmod(TheMatch->Time(), TheMatch->Rules().FlashSeconds) < 0.25;
			if (bFlash)
			{
				FHitResult Shade;
				FCollisionQueryParams Params(SCENE_QUERY_STAT(KineticReflector), false, Reflector);
				Params.AddIgnoredActor(P.Body.Get());
				Params.AddIgnoredActor(Rigger(B.Carrier));
				bFlash = !World.LineTraceSingleByChannel(Shade, P.At + FVector(0.0, 0.0, 60.0), P.At + ToSun * 40000.0, ECC_Visibility, Params);
			}
			Reflector->SetActorLocation(P.At + FVector(0.0, 0.0, 40.0));
			Reflector->SetActorHiddenInGame(B.State == Kinetic::EBeaconState::Carried && !bFlash);
		}
		if (UTextRenderComponent* Flag = P.Flag.Get())
		{
			const bool bShow = B.State == Kinetic::EBeaconState::Carried && Seats.IsValidIndex(B.Carrier) &&
			                   (Seats[B.Carrier].Sight.Plates & (1u << static_cast<int32>(Kinetic::ETagPlate::Back)));
			Flag->SetText(bShow ? FText::AsNumber(TheMatch->Rigger(B.Carrier).Code.Pips[1]) : FText::GetEmpty());
		}
	}
}

void FKineticTagOut::Show()
{
	// The digits each plate shows the watched rigger, the lockers' lights, who may move.
	for (int32 Slot = 0; Slot < Seats.Num(); ++Slot)
	{
		FSeat& Seat = Seats[Slot];
		const Kinetic::FTagRigger& R = TheMatch->Rigger(Slot);
		for (int32 I = 0; I < Kinetic::PlateCount; ++I)
		{
			if (UTextRenderComponent* Text = Seat.Plates[I].Get())
			{
				// The digit while the player can read it; a greasy smudge if it is turned their way
				// but smeared.
				const bool bOther = Slot != ViewSlot();
				const bool bSmudge = bOther && (Seat.Sight.Facing & (1u << I)) && TheMatch->Smeared(Slot, static_cast<Kinetic::ETagPlate>(I));
				Text->SetText(bOther && (Seat.Sight.Plates & (1u << I)) ? FText::AsNumber(R.Code.Pips[I])
				              : bSmudge                                 ? FText::FromString(TEXT("~"))
				                                                        : FText::GetEmpty());
				Text->SetTextRenderColor(bSmudge ? FColor(70, 55, 35) : FColor(255, 232, 90));
			}
			if (UStaticMeshComponent* Flare = Seat.Flares[I].Get())
			{
				Flare->SetVisibility(Slot != ViewSlot() && (Seat.Sight.Flaring & (1u << I)));
			}
		}
		if (UStaticMeshComponent* Glint = Seat.Glint.Get())
		{
			Glint->SetVisibility(R.bStanding && TheMatch->LevelReady(Slot));
		}
		if (AKineticCharacter* Character = Seat.Character.Get())
		{
			Character->SetMovementAllowed(TheMatch->CanMove(Slot));
		}
	}
	const double Now = TheMatch->Time();
	for (int32 I = 0; I < LockerLights.Num(); ++I)
	{
		UMaterialInstanceDynamic* Light = LockerLights[I].Get();
		if (!Light)
		{
			continue;
		}
		bool bBusy = Now < LockerRedUntil[I];
		for (const Kinetic::FTagRigger& R : TheMatch->Riggers())
		{
			bBusy |= R.UseOn == Kinetic::FTagOutMatch::UseLocker + I && R.UseHeld > 0.0;
		}
		const FLinearColor Colour = !TheMatch->LockersOpen() ? FLinearColor(0.05f, 0.05f, 0.05f)
		                            : bBusy                   ? FLinearColor(3.0f, 0.1f, 0.05f)
		                                                      : FLinearColor(0.1f, 3.0f, 0.2f);
		Light->SetVectorParameterValue(TEXT("Color"), Colour);
	}
}
