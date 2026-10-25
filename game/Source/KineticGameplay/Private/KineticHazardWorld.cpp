#include "KineticHazardWorld.h"

#include "Components/StaticMeshComponent.h"
#include "Engine/Engine.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kinetic/Impact.h"
#include "Kinetic/Tables.h"
#include "Kismet/GameplayStatics.h"
#include "KineticCharacter.h"
#include "KineticLadder.h"
#include "KineticProp.h"
#include "KineticRulesSubsystem.h"
#include "KineticSpoolWorld.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Sound/SoundBase.h"

DEFINE_LOG_CATEGORY_STATIC(LogKineticHazard, Log, All);

using Kinetic::FVec3;

namespace
{
FVec3 ToCore(const FVector& Cm) { return {Cm.X / 100.0, Cm.Y / 100.0, Cm.Z / 100.0}; }
FVector FromCore(const FVec3& M) { return FVector(M.X * 100.0, M.Y * 100.0, M.Z * 100.0); }

const Kinetic::FKineticTables* Tables()
{
	const UKineticRulesSubsystem* Rules = GEngine ? GEngine->GetEngineSubsystem<UKineticRulesSubsystem>() : nullptr;
	return Rules ? Rules->Tables() : nullptr;
}

Kinetic::EVolatile CoreKind(EKineticVolatile V) { return static_cast<Kinetic::EVolatile>(static_cast<uint8>(V) - 1); }

constexpr double ManifoldShriekSeconds = 1.0;  // The Yard's tells: valve shriek, then the jet
constexpr double ManifoldJackN = 600.0;         // DT_Volatiles GasManifold TriggerJackN
constexpr double ChargeShedRadiusCm = 400.0;    // The ladder cage beside a charge
}  // namespace

TStatId UKineticHazardWorld::GetStatId() const { RETURN_QUICK_DECLARE_CYCLE_STAT(UKineticHazardWorld, STATGROUP_Tickables); }

bool UKineticHazardWorld::DoesSupportWorldType(const EWorldType::Type WorldType) const
{
	return WorldType == EWorldType::Game || WorldType == EWorldType::PIE;
}

void UKineticHazardWorld::PlayCue(UWorld* World, const TCHAR* Name, const FVector* At)
{
	if (!World)
	{
		return;
	}
	const FString Path = FString::Printf(TEXT("/Game/Audio/SW_%s.SW_%s"), Name, Name);
	USoundBase* Sound = LoadObject<USoundBase>(nullptr, *Path, nullptr, LOAD_NoWarn | LOAD_Quiet);
	if (!Sound)
	{
		return;  // Not imported (make import-audio): silent
	}
	if (At)
	{
		UGameplayStatics::PlaySoundAtLocation(World, Sound, *At);
	}
	else
	{
		UGameplayStatics::PlaySound2D(World, Sound);
	}
}

UStaticMeshComponent* UKineticHazardWorld::Effect(const TCHAR* Mesh, const FVector& At, const FRotator& Rotation, const FVector& Scale,
                                                  const FLinearColor& Colour)
{
	if (!Effects)
	{
		FActorSpawnParameters Params;
		Params.ObjectFlags |= RF_Transient;
		Effects = GetWorld()->SpawnActor<AActor>(Params);
		Effects->SetRootComponent(NewObject<USceneComponent>(Effects));
		Effects->GetRootComponent()->RegisterComponent();
	}
	UStaticMeshComponent* C = NewObject<UStaticMeshComponent>(Effects);
	C->SetStaticMesh(LoadObject<UStaticMesh>(nullptr, Mesh));
	C->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	C->SetCastShadow(false);
	C->SetMobility(EComponentMobility::Movable);
	C->RegisterComponent();
	C->SetWorldLocationAndRotation(At, Rotation);
	C->SetWorldScale3D(Scale);
	if (UMaterialInstanceDynamic* M = C->CreateDynamicMaterialInstance(0))
	{
		M->SetVectorParameterValue(TEXT("Color"), Colour);
	}
	return C;
}

void UKineticHazardWorld::Arm(AKineticProp* Volatile)
{
	if (!Volatile || Volatile->IsSpent() || Volatile->Volatile == EKineticVolatile::None ||
	    Armed.ContainsByPredicate([Volatile](const FArmed& A) { return A.Prop.Get() == Volatile; }))
	{
		return;
	}
	const int32 Id = static_cast<int32>(Volatile->GetUniqueID());
	const bool bManifold = Volatile->Volatile == EKineticVolatile::GasManifold;
	if (!bManifold)
	{
		Gate.Cue(Kinetic::ETellSystem::VolatileArmed, Id, Clock);  // The hiss: 0.3 s before it may go
	}
	else
	{
		Gate.Cue(Kinetic::ETellSystem::RatedLoadMoving, Id, Clock);  // The shriek: the jet after 1.0 s
	}
	const FVector At = Volatile->GetActorLocation();
	PlayCue(GetWorld(), bManifold ? TEXT("ValveShriek") : TEXT("Hiss"), &At);
	Armed.Add({Volatile});
	UE_LOG(LogKineticHazard, Log, TEXT("%s armed"), *Volatile->GetName());
}

void UKineticHazardWorld::UpdateArmed()
{
	for (int32 I = Armed.Num(); I-- > 0;)
	{
		AKineticProp* Prop = Armed[I].Prop.Get();
		if (!Prop || Prop->IsSpent())
		{
			Armed.RemoveAt(I);
			continue;
		}
		const int32 Id = static_cast<int32>(Prop->GetUniqueID());
		if (Prop->Volatile == EKineticVolatile::GasManifold)
		{
			if (Gate.MayFire(Kinetic::ETellSystem::RatedLoadMoving, Id, Clock))
			{
				Gate.Clear(Kinetic::ETellSystem::RatedLoadMoving, Id);
				Armed.RemoveAt(I);
				Vent(Prop);
			}
			continue;
		}
		if (Gate.MayFire(Kinetic::ETellSystem::VolatileArmed, Id, Clock))
		{
			Gate.Clear(Kinetic::ETellSystem::VolatileArmed, Id);
			Armed.RemoveAt(I);
			Blast(Prop, Prop->GetActorLocation());
		}
	}
}

void UKineticHazardWorld::Blast(AKineticProp* Source, const FVector& At)
{
	const Kinetic::FKineticTables* T = Tables();
	if (!T || !Source || Source->Volatile == EKineticVolatile::None || Source->Volatile == EKineticVolatile::GasManifold)
	{
		return;
	}
	const Kinetic::EVolatile Kind = CoreKind(Source->Volatile);
	const Kinetic::FVolatileTuning& V = T->Volatile(Kind);
	Source->Spend();
	++Blasts;
	UE_LOG(LogKineticHazard, Log, TEXT("%s blast at %s"), *Source->GetName(), *At.ToCompactString());
	PlayCue(GetWorld(), TEXT("Blast"), &At);
	// The flash: the blast's radius, for a quarter second.
	Flashes.Add({Effect(TEXT("/Engine/BasicShapes/Sphere.Sphere"), At, FRotator::ZeroRotator, FVector(V.BlastRadius * 2.0),
	                    FLinearColor(1.0f, 0.55f, 0.1f)),
	             Clock + 0.25});
	UKineticSpoolWorld* Spool = GetWorld()->GetSubsystem<UKineticSpoolWorld>();
	// Players: judged on distance (05 6.4), thrown with the impulse.
	for (TActorIterator<AKineticCharacter> It(GetWorld()); It; ++It)
	{
		AKineticCharacter* Player = *It;
		if (Spool && !Spool->IsStanding(Player))
		{
			continue;
		}
		const FVector Away = Player->GetActorLocation() - At;
		const Kinetic::FBlastResult R = Kinetic::ClassifyBlast(Kind, Away.Size() / 100.0, false, *T);
		if (R.Outcome == Kinetic::EOutcome::None && R.ImpulseNs <= 0.0)
		{
			continue;
		}
		const double Mass = T->Harness(KineticHarness::ToCore(Player->Harness)).MassKg;
		const FVector Throw = (Away.GetSafeNormal() + FVector(0.0, 0.0, 0.5)).GetSafeNormal() * (R.ImpulseNs / Mass * 100.0);
		Player->LaunchCharacter(Throw, true, true);
		if (Spool)
		{
			Spool->Hit(Player, R.Outcome, Kinetic::EStallCause::Blast);
		}
	}
	// Props: the impulse falls to nothing at the radius; volatiles in it arm.
	const double RadiusCm = V.BlastRadius * 100.0;
	for (TActorIterator<AKineticProp> It(GetWorld()); It; ++It)
	{
		AKineticProp* Prop = *It;
		const FVector Away = Prop->GetActorLocation() - At;
		const double D = Away.Size();
		if (Prop == Source || Prop->IsSpent() || D > RadiusCm)
		{
			continue;
		}
		if (Prop->Volatile != EKineticVolatile::None)
		{
			Arm(Prop);
		}
		if (!Prop->bFixedScenery && Prop->Placement != EKineticPlacement::Rated)
		{
			Prop->BreakFree();
			const double Impulse = V.BlastImpulseNs * (1.0 - D / RadiusCm);  // N·s
			Prop->Mesh->AddImpulse((Away.GetSafeNormal() + FVector(0.0, 0.0, 0.4)).GetSafeNormal() * Impulse * 100.0);
		}
	}
	if (Source->Volatile == EKineticVolatile::FuelDrum)
	{
		Ignite(At);
	}
	if (Source->Volatile == EKineticVolatile::DemoCharge)
	{
		ShedLadderNear(At);
	}
}

void UKineticHazardWorld::Ignite(const FVector& At)
{
	// The fire patch on the floor under the drum.
	FHitResult Floor;
	FVector Centre = At;
	FCollisionQueryParams Params(SCENE_QUERY_STAT(KineticFire), false);
	if (GetWorld()->LineTraceSingleByChannel(Floor, At + FVector(0, 0, 50), At - FVector(0, 0, 500), ECC_WorldStatic, Params))
	{
		Centre = Floor.ImpactPoint;
	}
	FFire Fire;
	Fire.Patch.Center = ToCore(Centre);
	Fire.LitAt = Clock;
	Fire.Flames = Effect(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"), Centre + FVector(0, 0, Fire.Patch.Height * 50.0),
	                     FRotator::ZeroRotator, FVector(Fire.Patch.Radius * 2.0, Fire.Patch.Radius * 2.0, Fire.Patch.Height),
	                     FLinearColor(1.0f, 0.35f, 0.05f));
	Fires.Add(Fire);
}

void UKineticHazardWorld::Vent(AKineticProp* Manifold)
{
	FJet J;
	const FVector Apex = Manifold->GetActorLocation();
	const FVector Axis = Manifold->GetActorForwardVector();
	J.Jet.Apex = ToCore(Apex);
	J.Jet.Axis = {Axis.X, Axis.Y, Axis.Z};
	J.TriggeredAt = Clock;
	J.Vapour = Effect(TEXT("/Engine/BasicShapes/Cone.Cone"), Apex + Axis * (J.Jet.Length * 50.0),
	                  FRotationMatrix::MakeFromZ(-Axis).Rotator(), FVector(3.2, 3.2, J.Jet.Length), FLinearColor(0.85f, 0.88f, 0.9f));
	Jets.Add(J);
	PlayCue(GetWorld(), TEXT("GasJet"), &Apex);
}

void UKineticHazardWorld::Shove(const FVector& Centre, double RadiusM, double ARef, double Seconds)
{
	Shoves.Add({Centre, RadiusM, ARef, Clock + Seconds});
}

void UKineticHazardWorld::ShedLadderNear(const FVector& At)
{
	for (TActorIterator<AKineticLadder> It(GetWorld()); It; ++It)
	{
		if (FVector::Dist(It->Foot(), FVector(At.X, At.Y, It->Foot().Z)) < ChargeShedRadiusCm)
		{
			It->SetUsable(false);  // The cage falls: that route up the leg is gone for the round
		}
	}
}

void UKineticHazardWorld::WatchVolatiles()
{
	const Kinetic::FKineticTables* T = Tables();
	UKineticSpoolWorld* Spool = GetWorld()->GetSubsystem<UKineticSpoolWorld>();
	for (TActorIterator<AKineticProp> It(GetWorld()); It; ++It)
	{
		AKineticProp* Prop = *It;
		if (Prop->Volatile == EKineticVolatile::None || Prop->IsSpent())
		{
			continue;
		}
		const Kinetic::FVolatileTuning& V = T->Volatile(CoreKind(Prop->Volatile));
		// An impact: the velocity changes by more than the trigger speed in a step.
		const FVector Now = Prop->IsSimulating() ? Prop->Mesh->GetPhysicsLinearVelocity() : FVector::ZeroVector;
		const FVector* Last = LastVelocity.Find(Prop);
		if (Last && V.TriggerSpeed > 0.0 && (Now - *Last).Size() / 100.0 > V.TriggerSpeed)
		{
			Arm(Prop);
		}
		LastVelocity.Add(Prop, Now);
		// Something fixed (a charge) is struck by a body moving over its trigger speed.
		if (Prop->bFixedScenery && V.TriggerSpeed > 0.0)
		{
			const FVector At = Prop->GetActorLocation();
			for (TActorIterator<AKineticCharacter> C(GetWorld()); C; ++C)
			{
				if (FVector::Dist(C->GetActorLocation(), At) < 100.0 && C->GetVelocity().Size() / 100.0 > V.TriggerSpeed)
				{
					Arm(Prop);
				}
			}
			for (TActorIterator<AKineticProp> O(GetWorld()); O; ++O)
			{
				if (*O != Prop && O->IsSimulating() && FVector::Dist(O->GetActorLocation(), At) < 120.0 &&
				    O->GetVelocity().Size() / 100.0 > V.TriggerSpeed)
				{
					Arm(Prop);
				}
			}
		}
		// A manifold: a Jack over 600 N with a Dog in it (05 6.4).
		if (Prop->Volatile == EKineticVolatile::GasManifold && Spool)
		{
			for (const FKineticLine& L : Spool->Lines())
			{
				const bool bOn = (L.A.Seat == EKineticSeat::Prop && L.A.Prop.Get() == Prop) ||
				                 (L.B.Seat == EKineticSeat::Prop && L.B.Prop.Get() == Prop);
				if (bOn && L.IsLive() && L.State.Mode == Kinetic::ELineMode::Jack && L.State.AppliedForce > ManifoldJackN)
				{
					Arm(Prop);
				}
			}
		}
	}
}

bool UKineticHazardWorld::IsBurning(const FVector& At) const
{
	return Fires.ContainsByPredicate([&](const FFire& F) { return F.Patch.Contains(ToCore(At), Clock, F.LitAt); });
}

void UKineticHazardWorld::PushPlayer(AKineticCharacter& Player, const FVec3& ARef)
{
	const Kinetic::FKineticTables* T = Tables();
	const double Mass = T->Harness(KineticHarness::ToCore(Player.Harness)).MassKg;
	const FVec3 A = Kinetic::BodyAcceleration(ARef, Mass, Kinetic::EVolumeTarget::Player, T->Globals());
	if (A.Length() > 0.0)
	{
		Player.GetCharacterMovement()->AddImpulse(FromCore(A * Dt), true);
	}
}

void UKineticHazardWorld::PushBodies()
{
	const Kinetic::FKineticTables* T = Tables();
	UKineticSpoolWorld* Spool = GetWorld()->GetSubsystem<UKineticSpoolWorld>();
	const auto ARefAt = [&](const FVector& At, const FVector& VelocityCm) {
		FVec3 Sum;
		const FVec3 P = ToCore(At);
		const FVec3 V = ToCore(VelocityCm);
		for (const FFire& F : Fires)
		{
			Sum = Sum + F.Patch.ARefAt(P, V, Clock, F.LitAt);
		}
		for (const FJet& J : Jets)
		{
			Sum = Sum + J.Jet.ARefAt(P, V, Clock, J.TriggeredAt);
		}
		for (const FShove& S : Shoves)
		{
			const FVector Away = At - S.Centre;
			if (Clock < S.Until && Away.Size() < S.RadiusM * 100.0)
			{
				const FVector Dir = (Away.GetSafeNormal2D() + FVector(0, 0, 0.2)).GetSafeNormal();
				Sum = Sum + FVec3{Dir.X, Dir.Y, Dir.Z} * S.ARef;
			}
		}
		return Sum;
	};
	TSet<TWeakObjectPtr<AKineticCharacter>> Burning;
	for (TActorIterator<AKineticCharacter> It(GetWorld()); It; ++It)
	{
		AKineticCharacter* Player = *It;
		if (Spool && !Spool->IsStanding(Player))
		{
			continue;
		}
		PushPlayer(*Player, ARefAt(Player->GetActorLocation(), Player->GetVelocity()));
		if (IsBurning(Player->GetActorLocation() - FVector(0, 0, 90)))
		{
			Burning.Add(Player);
			if (!InFire.Contains(Player) && Spool)
			{
				Spool->Hit(Player, Kinetic::EOutcome::Stagger, Kinetic::EStallCause::Blast);  // Entering a fire patch staggers
			}
		}
	}
	InFire = MoveTemp(Burning);
	// Loose props and props on a line (VolumeMoves), never placed, rated or fixed ones.
	for (TActorIterator<AKineticProp> It(GetWorld()); It; ++It)
	{
		AKineticProp* Prop = *It;
		if (!Prop->IsSimulating() || Prop->IsSpent())
		{
			continue;
		}
		const FVec3 A = Kinetic::BodyAcceleration(ARefAt(Prop->GetActorLocation(), Prop->GetVelocity()), Prop->MassKg,
		                                          Kinetic::EVolumeTarget::LooseObject, T->Globals());
		if (A.Length() > 0.0)
		{
			Prop->Mesh->AddForce(FromCore(A), NAME_None, true);
		}
	}
}

void UKineticHazardWorld::ResetRound()
{
	for (const FFire& F : Fires)
	{
		if (F.Flames.IsValid())
		{
			F.Flames->DestroyComponent();
		}
	}
	for (const FJet& J : Jets)
	{
		if (J.Vapour.IsValid())
		{
			J.Vapour->DestroyComponent();
		}
	}
	if (Effects)
	{
		Effects->Destroy();
		Effects = nullptr;
	}
	Fires.Reset();
	Jets.Reset();
	Flashes.Reset();
	Shoves.Reset();
	Armed.Reset();
	Gate.Reset();
	LastVelocity.Reset();
	InFire.Reset();
	for (TActorIterator<AKineticLadder> It(GetWorld()); It; ++It)
	{
		It->SetUsable(true);  // The ladder cage restored (events-and-moving-parts 7)
	}
}

void UKineticHazardWorld::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (!GetWorld() || !GetWorld()->HasBegunPlay() || !Tables())
	{
		return;
	}
	WatchVolatiles();
	UpdateArmed();
	PushBodies();
	// Spent effects go.
	for (int32 I = Fires.Num(); I-- > 0;)
	{
		if (Clock - Fires[I].LitAt > Fires[I].Patch.Seconds)
		{
			if (Fires[I].Flames.IsValid())
			{
				Fires[I].Flames->DestroyComponent();
			}
			Fires.RemoveAt(I);
		}
	}
	for (int32 I = Jets.Num(); I-- > 0;)
	{
		if (Clock - Jets[I].TriggeredAt > Jets[I].Jet.Seconds)
		{
			if (Jets[I].Vapour.IsValid())
			{
				Jets[I].Vapour->DestroyComponent();
			}
			Jets.RemoveAt(I);
		}
	}
	Shoves.RemoveAll([this](const FShove& S) { return Clock >= S.Until; });
	for (int32 I = Flashes.Num(); I-- > 0;)
	{
		if (Clock >= Flashes[I].Until)
		{
			if (Flashes[I].Mesh.IsValid())
			{
				Flashes[I].Mesh->DestroyComponent();
			}
			Flashes.RemoveAt(I);
		}
	}
	Clock += Dt;
}
