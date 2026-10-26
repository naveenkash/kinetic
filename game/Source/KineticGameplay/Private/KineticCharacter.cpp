#include "KineticCharacter.h"

#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/Engine.h"
#include "EngineUtils.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "EnhancedInputComponent.h"
#include "GroomAsset.h"
#include "GroomComponent.h"
#include "HAL/IConsoleManager.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "InputAction.h"
#include "InputMappingContext.h"
#include "InputModifiers.h"
#include "Kinetic/Units.h"
#include "KineticKinematic.h"
#include "KineticLadder.h"
#include "KineticYardDirector.h"
#include "KineticMovementComponent.h"
#include "KineticRulesSubsystem.h"
#include "KineticSpoolWorld.h"
#include "Animation/AnimSequence.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/SkeletalMesh.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"

DEFINE_LOG_CATEGORY_STATIC(LogKineticCharacter, Log, All);

namespace
{
// docs/tech/player-physics.md section 1: a 1.9 m capsule of 0.4 m radius; the eye 1.7 m up.
constexpr float CapsuleRadiusCm = 40.0f;
constexpr float CapsuleHalfHeightCm = 95.0f;
constexpr float EyeAboveCentreCm = 75.0f;

float Cm(double Meters) { return static_cast<float>(Kinetic::Units::MetersToCm(Meters)); }
}  // namespace

AKineticCharacter::AKineticCharacter(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer.SetDefaultSubobjectClass<UKineticMovementComponent>(ACharacter::CharacterMovementComponentName))
{
	PrimaryActorTick.bCanEverTick = true;
	GetCapsuleComponent()->InitCapsuleSize(CapsuleRadiusCm, CapsuleHalfHeightCm);
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(GetCapsuleComponent());
	Camera->SetRelativeLocation(FVector(0.0, 0.0, EyeAboveCentreCm));
	Camera->bUsePawnControlRotation = true;
	bUseControllerRotationYaw = true;
	// The grey-box body: the engine's 1 m cylinder and cube, coloured like the lines.
	static ConstructorHelpers::FObjectFinder<UStaticMesh> Cylinder(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> Cube(TEXT("/Engine/BasicShapes/Cube.Cube"));
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> Shape(TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
	const auto Part = [&](const TCHAR* Name, UStaticMesh* Mesh, const FVector& At, const FVector& Scale) {
		UStaticMeshComponent* C = CreateDefaultSubobject<UStaticMeshComponent>(Name);
		C->SetupAttachment(GetCapsuleComponent());
		C->SetStaticMesh(Mesh);
		C->SetMaterial(0, Shape.Object);
		C->SetRelativeLocation(At);
		C->SetRelativeScale3D(Scale);
		C->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		C->SetCanEverAffectNavigation(false);
		C->SetOwnerNoSee(true);  // First person: the owner sees the world, not their own body
		C->CastShadow = true;
		return C;
	};
	Body = Part(TEXT("Body"), Cylinder.Object, FVector::ZeroVector, FVector(0.8, 0.8, 1.9));
	// The figure: the feet at the capsule's foot, facing the actor's forward (art/characters).
	GetMesh()->SetRelativeLocation(FVector(0.0, 0.0, -CapsuleHalfHeightCm));
	GetMesh()->SetOwnerNoSee(true);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetMesh()->SetCanEverAffectNavigation(false);
	Visor = Part(TEXT("Visor"), Cube.Object, FVector(36.0, 0.0, EyeAboveCentreCm - 5.0), FVector(0.12, 0.5, 0.14));
	UCharacterMovementComponent* Movement = GetCharacterMovement();
	Movement->NavAgentProps.bCanCrouch = true;
	Movement->bRunPhysicsWithNoController = true;  // Tests and bots move it without a player
	NormalFriction = Movement->GroundFriction;
	NormalBraking = Movement->BrakingDecelerationWalking;
	// Props striking a standing player report their impulse (the Impact Monitor's Object hits).
	GetCapsuleComponent()->SetNotifyRigidBodyCollision(true);
}

bool AKineticCharacter::ApplyHarness()
{
	const UKineticRulesSubsystem* Rules = GEngine ? GEngine->GetEngineSubsystem<UKineticRulesSubsystem>() : nullptr;
	const Kinetic::FKineticTables* Tables = Rules ? Rules->Tables() : nullptr;
	if (!Tables)
	{
		UE_LOG(LogKineticCharacter, Error, TEXT("%s: the rules tables did not load; movement left at defaults"),
		       *GetName());
		return false;
	}
	const Kinetic::FHarnessTuning& Row = Tables->Harness(KineticHarness::ToCore(Harness));
	RunSpeedM = static_cast<float>(Row.RunSpeed);
	SprintSpeedM = static_cast<float>(Row.SprintSpeed);
	MantleHeightM = static_cast<float>(Row.MantleHeight);
	LadderSpeedM = static_cast<float>(Row.LadderSpeed);
	CrouchSpeedM = static_cast<float>(Row.CrouchSpeed);
	UCharacterMovementComponent* Movement = GetCharacterMovement();
	Movement->MaxWalkSpeed = GroundSpeedCm();
	Movement->MaxWalkSpeedCrouched = Cm(Row.CrouchSpeed);
	Movement->AirControl = static_cast<float>(Row.AirControl);
	Movement->Mass = static_cast<float>(Row.MassKg);
	// Take-off speed for the class's jump height: v = sqrt(2 g h).
	const double Gravity = FMath::Abs(Movement->GetGravityZ());
	Movement->JumpZVelocity = static_cast<float>(FMath::Sqrt(2.0 * Gravity * Kinetic::Units::MetersToCm(Row.JumpHeight)));
	// A ledge the class can mantle is never a step it walks up: steps stay at the engine's 45 cm.
	LoadFigure();  // The class's figure and takes
	return true;
}

void AKineticCharacter::BeginPlay()
{
	Super::BeginPlay();
	ApplyHarness();
}

namespace
{
TAutoConsoleVariable<int32> CVarMetaHumans(TEXT("kinetic.MetaHumans"), 1,
                                           TEXT("1: play each class as its MetaHuman when it is built, retargeted and fitted"));
}

bool AKineticCharacter::LoadMetaHuman(const FString& Class)
{
	if (CVarMetaHumans.GetValueOnGameThread() == 0)
	{
		return false;  // Off until the retarget is verified (game/README.md)
	}
	// Built by build_metahumans.py, its takes by retarget_metahumans.py, its gear by
	// art/characters (fitted to the body, on its skeleton). All of it, or the harness figure.
	const auto Load = [](const FString& Path) {
		const FString Name = FPaths::GetBaseFilename(Path);
		return LoadObject<USkeletalMesh>(nullptr, *FString::Printf(TEXT("%s.%s"), *Path, *Name), nullptr, LOAD_NoWarn | LOAD_Quiet);
	};
	const FString Built = FString::Printf(TEXT("/Game/MetaHumans/MH_%s/MH_%s"), *Class, *Class);
	const FString Folder = FString::Printf(TEXT("/Game/Characters/MetaHumans/%s"), *Class);
	USkeletalMesh* BodyMesh = Load(FString::Printf(TEXT("%s/Body/SKM_MH_%s_BodyMesh"), *Built, *Class));
	USkeletalMesh* FaceMesh = Load(FString::Printf(TEXT("%s/Face/SKM_MH_%s_FaceMesh"), *Built, *Class));
	USkeletalMesh* GearMesh = Load(FString::Printf(TEXT("%s/SK_MHGear_%s"), *Folder, *Class));
	if (!BodyMesh || !FaceMesh || !GearMesh)
	{
		return false;
	}
	static const TCHAR* Names[] = {TEXT("Idle"), TEXT("Run"), TEXT("Climb"), TEXT("Fall"), TEXT("Brace"), TEXT("Stagger")};
	TObjectPtr<UAnimSequence> MetaTakes[static_cast<int32>(EFigureTake::Count)];
	for (int32 I = 0; I < static_cast<int32>(EFigureTake::Count); ++I)
	{
		MetaTakes[I] = LoadObject<UAnimSequence>(nullptr, *FString::Printf(TEXT("%s/AS_%s_%s_MH.AS_%s_%s_MH"), *Folder, *Class, Names[I], *Class, Names[I]),
		                                         nullptr, LOAD_NoWarn | LOAD_Quiet);
		if (!MetaTakes[I])
		{
			return false;
		}
	}
	for (int32 I = 0; I < static_cast<int32>(EFigureTake::Count); ++I)
	{
		Takes[I] = MetaTakes[I];
	}
	GetMesh()->SetSkeletalMesh(BodyMesh);
	GetMesh()->SetRelativeRotation(FRotator(0.0, -90.0, 0.0));  // A MetaHuman faces +Y; the character faces +X
	// The body is all under the coverall, gloves and boots: not drawn, but still posed every
	// frame, since the face and the gear follow its bones.
	GetMesh()->SetVisibility(true);
	GetMesh()->SetRenderInMainPass(false);
	GetMesh()->SetRenderInDepthPass(false);
	GetMesh()->SetCastShadow(false);
	GetMesh()->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::AlwaysTickPoseAndRefreshBones;
	// Always the full skeleton: the body's lower LODs drop the twist and finger bones, which the
	// gear (one LOD, all bones) would then take in their A-pose, bending the arms and splaying the hands.
	GetMesh()->SetForcedLOD(1);
	const auto Follower = [this](TObjectPtr<USkeletalMeshComponent>& Part, const TCHAR* Name, USkeletalMesh* Mesh) {
		if (!Part)
		{
			Part = NewObject<USkeletalMeshComponent>(this, Name);
			Part->SetupAttachment(GetMesh());
			Part->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			Part->SetCanEverAffectNavigation(false);
			Part->SetOwnerNoSee(true);
			Part->RegisterComponent();
		}
		Part->SetSkeletalMesh(Mesh);
		Part->SetLeaderPoseComponent(GetMesh());
	};
	Follower(Face, TEXT("Face"), FaceMesh);
	Follower(Gear, TEXT("Gear"), GearMesh);
	LoadGrooms(Built);
	return true;
}

void AKineticCharacter::LoadGrooms(const FString& Built)
{
	for (UGroomComponent* Old : Grooms)
	{
		if (Old)
		{
			Old->DestroyComponent();
		}
	}
	Grooms.Reset();
	// Every groom the MetaHuman's build made (its Grooms folder), on the face's head bone. Rigid,
	// not bound to the face's skin: bound, it was drawn metres away (the face follows the body,
	// turned to face +X), and the grooms are tied back, so they need not flex.
	const IAssetRegistry& Registry = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry")).Get();
	TArray<FAssetData> Found;
	Registry.GetAssetsByPath(FName(*(Built + TEXT("/Grooms"))), Found, false);
	// The grooms are authored in the MetaHuman's frame: undo the head's rest transform there.
	const FReferenceSkeleton& RefSkeleton = Face->GetSkeletalMeshAsset()->GetRefSkeleton();
	FTransform HeadRest = FTransform::Identity;
	for (int32 Bone = RefSkeleton.FindBoneIndex(TEXT("head")); Bone != INDEX_NONE; Bone = RefSkeleton.GetParentIndex(Bone))
	{
		HeadRest = HeadRest * RefSkeleton.GetRefBonePose()[Bone];
	}
	for (const FAssetData& Data : Found)
	{
		if (Data.AssetClassPath != UGroomAsset::StaticClass()->GetClassPathName())
		{
			continue;
		}
		UGroomAsset* Asset = Cast<UGroomAsset>(Data.GetAsset());
		if (!Asset)
		{
			continue;
		}
		UGroomComponent* Groom = NewObject<UGroomComponent>(this, Data.AssetName);
		Groom->SetupAttachment(Face, TEXT("head"));
		Groom->SetRelativeTransform(HeadRest.Inverse());
		Groom->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		Groom->SetOwnerNoSee(true);
		Groom->SetGroomAsset(Asset, nullptr);
		Groom->RegisterComponent();
		// Neither its strands (LODs 0, 1) nor its cards (2 to 4) draw on this Mac's Metal renderer:
		// play its best mesh LOD.
		Groom->SetForcedLOD(FMath::Min(5, Groom->GetNumLODs() - 1));
		Grooms.Add(Groom);
	}
	UE_LOG(LogKineticCharacter, Log, TEXT("%s: %d grooms from %s/Grooms (%d assets there)"), *GetName(), Grooms.Num(), *Built, Found.Num());
}

void AKineticCharacter::LoadFigure()
{
	const FString Class = KineticHarness::Name(Harness);
	bMetaHuman = LoadMetaHuman(Class);
	if (bMetaHuman)
	{
		bFigure = true;
		GetMesh()->SetAnimationMode(EAnimationMode::AnimationSingleNode);
		Body->SetVisibility(false);
		Visor->SetVisibility(false);
		Take = EFigureTake::Count;
		SetCrew(Crew);
		return;
	}
	const FString Folder = FString::Printf(TEXT("/Game/Characters/%s"), *Class);
	USkeletalMesh* Figure = LoadObject<USkeletalMesh>(nullptr, *FString::Printf(TEXT("%s/SK_Harness_%s.SK_Harness_%s"), *Folder, *Class, *Class),
	                                                  nullptr, LOAD_NoWarn | LOAD_Quiet);
	static const TCHAR* Names[] = {TEXT("Idle"), TEXT("Run"), TEXT("Climb"), TEXT("Fall"), TEXT("Brace"), TEXT("Stagger")};
	bFigure = Figure != nullptr;
	for (int32 I = 0; I < static_cast<int32>(EFigureTake::Count); ++I)
	{
		Takes[I] = bFigure ? LoadObject<UAnimSequence>(nullptr, *FString::Printf(TEXT("%s/AS_%s_%s.AS_%s_%s"), *Folder, *Class, Names[I], *Class, Names[I]),
		                                               nullptr, LOAD_NoWarn | LOAD_Quiet)
		                   : nullptr;
		bFigure &= Takes[I] != nullptr;
	}
	GetMesh()->SetSkeletalMesh(bFigure ? Figure : nullptr);
	GetMesh()->SetRelativeRotation(FRotator::ZeroRotator);  // The harness figure faces +X itself
	GetMesh()->SetVisibility(true);
	GetMesh()->SetForcedLOD(0);
	GetMesh()->SetRenderInMainPass(true);
	GetMesh()->SetRenderInDepthPass(true);
	GetMesh()->SetCastShadow(true);
	for (USkeletalMeshComponent* Part : {Face.Get(), Gear.Get()})
	{
		if (Part)
		{
			Part->SetSkeletalMesh(nullptr);
		}
	}
	for (UGroomComponent* Old : Grooms)
	{
		if (Old)
		{
			Old->DestroyComponent();
		}
	}
	Grooms.Reset();
	GetMesh()->SetAnimationMode(EAnimationMode::AnimationSingleNode);
	Body->SetVisibility(!bFigure);
	Visor->SetVisibility(!bFigure);
	Take = EFigureTake::Count;
	SetCrew(Crew);
}

void AKineticCharacter::UpdateFigure()
{
	if (!bFigure)
	{
		return;
	}
	// The take for what the character is doing now.
	const UCharacterMovementComponent* Movement = GetCharacterMovement();
	const UWorld* World = GetWorld();
	const double Now = World ? World->GetTimeSeconds() : 0.0;
	if (bStunned && StaggerUntil < Now)
	{
		StaggerUntil = Now + 0.6;  // One stagger take per hit
	}
	const double Speed = FVector(Movement->Velocity.X, Movement->Velocity.Y, 0.0).Size();
	EFigureTake Want = EFigureTake::Idle;
	if (IsClimbing())
	{
		Want = EFigureTake::Climb;
	}
	else if (Now < StaggerUntil)
	{
		Want = EFigureTake::Stagger;
	}
	else if (Movement->IsFalling())
	{
		Want = EFigureTake::Fall;
	}
	else if (bHeldStill || bIsCrouched)
	{
		Want = EFigureTake::Brace;
	}
	else if (Speed > 60.0)
	{
		Want = EFigureTake::Run;
	}
	if (Want != Take)
	{
		Take = Want;
		GetMesh()->PlayAnimation(Takes[static_cast<int32>(Take)], Take != EFigureTake::Stagger);
	}
	// The run's stride is authored at the class's run speed; the climb at its ladder speed.
	const float Rate = Take == EFigureTake::Run ? static_cast<float>(Speed / FMath::Max(1.0f, Cm(RunSpeedM)))
	                   : Take == EFigureTake::Climb ? FMath::Abs(static_cast<float>(Movement->Velocity.Z) / FMath::Max(1.0f, Cm(LadderSpeedM)))
	                                                : 1.0f;
	GetMesh()->SetPlayRate(FMath::Clamp(Rate, 0.0f, 2.0f));
}

FLinearColor AKineticCharacter::TeamTint(int32 InCrew)
{
	// Amber #E69F00 and Sky #56B4E9, colour-blind safe (05 11a).
	return InCrew == 1 ? FLinearColor::FromSRGBColor(FColor(0x56, 0xB4, 0xE9)) : FLinearColor::FromSRGBColor(FColor(0xE6, 0x9F, 0x00));
}

void AKineticCharacter::SetCrew(int32 InCrew)
{
	Crew = InCrew;
	if (UMaterialInstanceDynamic* M = Body->CreateDynamicMaterialInstance(0))
	{
		M->SetVectorParameterValue(TEXT("Color"), TeamTint(Crew));
	}
	// The figure's team-tint slot (plate rim bands, the mode ring, the shoulder lamp): on the
	// harness figure, or on a MetaHuman's gear.
	USkeletalMeshComponent* Tinted = bMetaHuman ? Gear.Get() : GetMesh();
	for (int32 I = 0; bFigure && Tinted && I < Tinted->GetNumMaterials(); ++I)
	{
		if (UMaterialInstanceDynamic* M = Tinted->CreateDynamicMaterialInstance(I))
		{
			M->SetVectorParameterValue(TEXT("TeamTint"), TeamTint(Crew));
		}
	}
	if (UMaterialInstanceDynamic* M = Visor->CreateDynamicMaterialInstance(0))
	{
		M->SetVectorParameterValue(TEXT("Color"), FLinearColor(0.05f, 0.05f, 0.05f));
	}
}

void AKineticCharacter::SetMovementAllowed(bool bAllowed)
{
	if (bAllowed == bMovementAllowed)
	{
		return;
	}
	bMovementAllowed = bAllowed;
	if (!bAllowed)
	{
		ForwardInput = 0.0f;
		ConsumeMovementInputVector();
		SetSprinting(false);
	}
}

void AKineticCharacter::SetSprinting(bool bSprint)
{
	bSprinting = bSprint;
	if (!bUnderLine)
	{
		GetCharacterMovement()->MaxWalkSpeed = GroundSpeedCm();
	}
}

void AKineticCharacter::SetUnderLine(bool bLine, bool bBraced)
{
	bHeldStill = bBraced;
	if (bLine == bUnderLine)
	{
		return;
	}
	bUnderLine = bLine;
	UCharacterMovementComponent* Movement = GetCharacterMovement();
	if (bLine)
	{
		// The skid is the line's acceleration and nothing else: no friction, no speed limit.
		Movement->GroundFriction = 0.0f;
		Movement->BrakingDecelerationWalking = 0.0f;
		Movement->MaxWalkSpeed = 5000.0f;
	}
	else
	{
		Movement->GroundFriction = NormalFriction;
		Movement->BrakingDecelerationWalking = NormalBraking;
		Movement->MaxWalkSpeed = GroundSpeedCm();
	}
}

void AKineticCharacter::FirePressed() { OnFire(); }

float AKineticCharacter::GroundSpeedCm() const
{
	if (bLevelSlow)
	{
		return Cm(CrouchSpeedM);
	}
	return Cm(bSprinting ? SprintSpeedM * SprintScale : RunSpeedM);
}

void AKineticCharacter::SetTagSlowdown(float InSprintScale, float InLadderScale, bool bLevel)
{
	SprintScale = InSprintScale;
	LadderScale = InLadderScale;
	bLevelSlow = bLevel;
	if (!bUnderLine)
	{
		GetCharacterMovement()->MaxWalkSpeed = GroundSpeedCm();
	}
	if (DefaultFov < 0.0f)
	{
		DefaultFov = Camera->FieldOfView;
	}
	// 3x magnification: the tangent of the half-angle a third as wide.
	const float Zoomed = 2.0f * FMath::RadiansToDegrees(FMath::Atan(FMath::Tan(FMath::DegreesToRadians(DefaultFov * 0.5f)) / 3.0f));
	Camera->SetFieldOfView(bLevel ? Zoomed : DefaultFov);
}

AKineticCharacter::FTagInput AKineticCharacter::TakeTagInput()
{
	FTagInput Taken = Tag;
	Tag.Pips.Reset();
	Tag.bCall = Tag.bClear = Tag.bSwing = Tag.bThrow = false;
	return Taken;
}

void AKineticCharacter::OnPip(int32 Pip)
{
	if (bTagOut && !bCursorFree)
	{
		Tag.Pips.Add(Pip);
	}
}

void AKineticCharacter::OnCall()
{
	if (bTagOut && !bCursorFree)
	{
		Tag.bCall = true;
	}
}

void AKineticCharacter::OnUseReleased() { Tag.bUse = false; }

void AKineticCharacter::OnThrow()
{
	if (bTagOut && !bCursorFree)
	{
		Tag.bThrow = true;
	}
}

std::vector<Kinetic::FContact> AKineticCharacter::TakeContacts()
{
	std::vector<Kinetic::FContact> Out;
	Out.swap(Contacts);
	return Out;
}

namespace
{
// The surface modifier an actor's tag names (05 7.3): scrap heaps, laundry and nets x2.0, wood
// x1.2, deep water x3.0, Stall-gel x4.0; anything else is hard.
Kinetic::ESurface SurfaceOf(const AActor* Other)
{
	if (!Other)
	{
		return Kinetic::ESurface::Hard;
	}
	if (Other->ActorHasTag(TEXT("Surface.Scrap")))
	{
		return Kinetic::ESurface::Scrap;
	}
	if (Other->ActorHasTag(TEXT("Surface.Wood")))
	{
		return Kinetic::ESurface::Wood;
	}
	if (Other->ActorHasTag(TEXT("Surface.DeepWater")))
	{
		return Kinetic::ESurface::DeepWater;
	}
	if (Other->ActorHasTag(TEXT("Surface.StallGel")))
	{
		return Kinetic::ESurface::StallGel;
	}
	return Kinetic::ESurface::Hard;
}
}  // namespace

void AKineticCharacter::Landed(const FHitResult& Hit)
{
	// Called before the landing zeroes the fall: the speed into the floor is still there.
	RecordContact(Hit, GetCharacterMovement()->Velocity, FVector::ZeroVector);
	// A Breaker who jumps onto a plank bridge breaks it (the Yard, layout zone 1).
	AActor* Floor = Hit.GetActor();
	if (Harness == EKineticHarness::Breaker && Floor && Floor->ActorHasTag(TEXT("Kinetic.PlankBridge")))
	{
		for (TActorIterator<AKineticYardDirector> It(GetWorld()); It; ++It)
		{
			It->Fracture(Floor);
		}
	}
	Super::Landed(Hit);
}

void AKineticCharacter::NotifyHit(UPrimitiveComponent* MyComp, AActor* Other, UPrimitiveComponent* OtherComp, bool bSelfMoved,
                                  FVector HitLocation, FVector HitNormal, FVector NormalImpulse, const FHitResult& Hit)
{
	Super::NotifyHit(MyComp, Other, OtherComp, bSelfMoved, HitLocation, HitNormal, NormalImpulse, Hit);
	// Movement's own hits are recorded as they happen (Landed, the movement component); what
	// arrives here unprompted is a body striking the player, with Chaos's impulse.
	if (!bSelfMoved && OtherComp && OtherComp->IsSimulatingPhysics())
	{
		RecordContact(Hit, GetCharacterMovement()->Velocity, NormalImpulse);
	}
}

void AKineticCharacter::RecordContact(const FHitResult& Hit, const FVector& MyVelocity, const FVector& NormalImpulse)
{
	AActor* Other = Hit.GetActor();
	UPrimitiveComponent* OtherComp = Hit.GetComponent();
	if (!Other || !OtherComp || Other == this)
	{
		return;
	}
	// The way the contact pushes the player, and both velocities at it (m/s).
	const FVector Push = (GetActorLocation() - FVector(Hit.ImpactPoint)).GetSafeNormal();
	const FVector Mine = MyVelocity / 100.0;
	FVector Theirs = FVector::ZeroVector;
	Kinetic::FContact Contact;
	Contact.OtherId = static_cast<int>(Other->GetUniqueID());
	Contact.Surface = SurfaceOf(Other);
	Contact.Normal = {Push.X, Push.Y, Push.Z};
	const UKineticRulesSubsystem* Rules = GEngine->GetEngineSubsystem<UKineticRulesSubsystem>();
	const Kinetic::FKineticTables* T = Rules ? Rules->Tables() : nullptr;
	if (const AKineticCharacter* Them = Cast<AKineticCharacter>(Other))
	{
		Contact.Kind = Kinetic::EContactKind::Player;
		Theirs = Them->GetCharacterMovement()->Velocity / 100.0;
		Contact.OtherMassKg = T ? T->Harness(KineticHarness::ToCore(Them->Harness)).MassKg : 100.0;
	}
	else if (OtherComp->IsSimulatingPhysics())
	{
		Contact.Kind = Kinetic::EContactKind::Object;
		Theirs = OtherComp->GetPhysicsLinearVelocityAtPoint(Hit.ImpactPoint) / 100.0;
		Contact.OtherMassKg = OtherComp->GetMass();
	}
	else if (const AKineticKinematic* Kinematic = Cast<AKineticKinematic>(Other))
	{
		Contact.Kind = Kinetic::EContactKind::Kinematic;  // Judged at its own speed
		Theirs = Kinematic->Velocity() / 100.0;
	}
	else
	{
		Contact.Kind = OtherComp->Mobility == EComponentMobility::Movable ? Kinetic::EContactKind::Kinematic
		                                                                   : Kinetic::EContactKind::Static;
	}
	Contact.RelativeSpeed = FMath::Max(0.0, FVector::DotProduct(Theirs - Mine, Push));
	if (Contact.Kind == Kinetic::EContactKind::Object)
	{
		// The solver's impulse when Chaos reports one; a sweep into a prop reports none, so the
		// exchange of the two bodies' momentum along the normal stands in for it.
		const double Reported = NormalImpulse.Size() / 100.0;
		const double MyMass = T ? T->Harness(KineticHarness::ToCore(Harness)).MassKg : 100.0;
		const double Reduced = MyMass * Contact.OtherMassKg / FMath::Max(1.0, MyMass + Contact.OtherMassKg);
		Contact.ImpulseNs = Reported > 0.0 ? Reported : Reduced * Contact.RelativeSpeed;
	}
	Contacts.push_back(Contact);
}

void AKineticCharacter::Move(const FVector2D& Axis)
{
	if (!bMovementAllowed)
	{
		ForwardInput = 0.0f;
		return;  // The match is holding the crews
	}
	ForwardInput = static_cast<float>(Axis.Y);
	if (IsClimbing() || bUnderLine || bHeldStill || bStunned)
	{
		return;  // On a ladder forward is up (UpdateLadder reads it); under a line or braced, no walking
	}
	const FRotator Yaw(0.0, GetControlRotation().Yaw, 0.0);
	AddMovementInput(FRotationMatrix(Yaw).GetUnitAxis(EAxis::X), static_cast<float>(Axis.Y));
	AddMovementInput(FRotationMatrix(Yaw).GetUnitAxis(EAxis::Y), static_cast<float>(Axis.X));
}

bool AKineticCharacter::TryMantle()
{
	UWorld* World = GetWorld();
	if (!World || IsClimbing())
	{
		return false;
	}
	const FVector Forward = FRotationMatrix(FRotator(0.0, GetActorRotation().Yaw, 0.0)).GetUnitAxis(EAxis::X);
	const float Feet = static_cast<float>(GetActorLocation().Z) - CapsuleHalfHeightCm;
	const float Reach = CapsuleRadiusCm + 40.0f;
	FCollisionQueryParams Params(SCENE_QUERY_STAT(KineticMantle), false, this);
	// Something in front at waist height...
	FHitResult Wall;
	const FVector Waist = GetActorLocation() - FVector(0.0, 0.0, CapsuleHalfHeightCm * 0.5);
	if (!World->LineTraceSingleByChannel(Wall, Waist, Waist + Forward * Reach, ECC_Visibility, Params))
	{
		return false;
	}
	// ...whose top is within the mantle height, found by tracing down just past its face.
	const FVector Over = Wall.ImpactPoint + Forward * (CapsuleRadiusCm + 5.0f);
	FHitResult Top;
	const FVector Down0(Over.X, Over.Y, Feet + Cm(MantleHeightM) + 5.0f);
	if (!World->LineTraceSingleByChannel(Top, Down0, FVector(Over.X, Over.Y, Feet), ECC_Visibility, Params))
	{
		return false;
	}
	const float Rise = static_cast<float>(Top.ImpactPoint.Z) - Feet;
	if (Rise < 45.0f || Rise > Cm(MantleHeightM) || Top.ImpactNormal.Z < 0.7)
	{
		return false;
	}
	const FVector Standing = Top.ImpactPoint + FVector(0.0, 0.0, CapsuleHalfHeightCm + 2.0f);
	const FCollisionShape Capsule = FCollisionShape::MakeCapsule(CapsuleRadiusCm, CapsuleHalfHeightCm);
	if (World->OverlapBlockingTestByChannel(Standing, FQuat::Identity, ECC_Pawn, Capsule, Params))
	{
		return false;  // No room on top
	}
	SetActorLocation(Standing, false, nullptr, ETeleportType::TeleportPhysics);
	GetCharacterMovement()->Velocity = FVector::ZeroVector;
	GetCharacterMovement()->SetMovementMode(MOVE_Walking);
	return true;
}

void AKineticCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	UpdateLadder(DeltaSeconds);
	UpdateFigure();
}

void AKineticCharacter::UpdateLadder(float DeltaSeconds)
{
	UCharacterMovementComponent* Movement = GetCharacterMovement();
	if (!Ladder)
	{
		if (!bTakesLadders)
		{
			return;
		}
		const FVector Facing = FRotationMatrix(FRotator(0.0, GetActorRotation().Yaw, 0.0)).GetUnitAxis(EAxis::X);
		const float Feet = static_cast<float>(GetActorLocation().Z) - CapsuleHalfHeightCm;
		const double Now = GetWorld()->GetTimeSeconds();
		if (Movement->IsMovingOnGround())
		{
			GroundedAt = Now;
		}
		else if (Movement->IsFalling() && !bStunned && Now - GroundedAt < 0.35 && Movement->Velocity.Z <= 0.0)
		{
			// Stepped off beside the top of a ladder, whichever way it faced: onto the rungs, facing
			// the wall, rather than down the drop (a jump is still rising this soon, and is let go).
			for (TActorIterator<AKineticLadder> It(GetWorld()); It; ++It)
			{
				AKineticLadder* Candidate = *It;
				const FVector Hold = Candidate->ClimbAt(FMath::Min(GetActorLocation().Z, Candidate->TopZ() - 120.0 + CapsuleHalfHeightCm));
				if (Feet > Candidate->TopZ() - 150.0f && Feet < Candidate->TopZ() + 10.0f &&
				    FVector::Dist2D(GetActorLocation(), Hold) < 150.0)  // The ledge guard's window (UKineticMovementComponent)
				{
					FCollisionQueryParams Params(SCENE_QUERY_STAT(KineticLadderCatch), false, this);
					if (GetWorld()->OverlapBlockingTestByChannel(Hold, FQuat::Identity, ECC_Pawn,
					                                             FCollisionShape::MakeCapsule(CapsuleRadiusCm - 5.0f, CapsuleHalfHeightCm - 5.0f), Params))
					{
						continue;
					}
					SetActorLocation(Hold, false, nullptr, ETeleportType::TeleportPhysics);
					const FRotator ToWall(0.0, Candidate->IntoWall.Rotation().Yaw, 0.0);
					SetActorRotation(ToWall);
					if (Controller)
					{
						Controller->SetControlRotation(ToWall);
					}
					Ladder = Candidate;
					Movement->Velocity = FVector::ZeroVector;
					Movement->SetMovementMode(MOVE_Flying);
					return;
				}
			}
		}
		if (ForwardInput < -0.1f && Movement->IsMovingOnGround())
		{
			// Backing over the top of a ladder, facing the wall: onto the rungs, going down.
			for (TActorIterator<AKineticLadder> It(GetWorld()); It; ++It)
			{
				AKineticLadder* Candidate = *It;
				if (FVector::DotProduct(Facing, Candidate->IntoWall) > 0.5 && FMath::Abs(Feet - Candidate->TopZ()) < 40.0f &&
				    FVector::Dist2D(GetActorLocation(), Candidate->StepOff()) < 110.0)
				{
					// Room on the rungs below the top (nobody climbing there).
					const FVector Hold = Candidate->ClimbAt(Candidate->TopZ() - 120.0 + CapsuleHalfHeightCm);
					FCollisionQueryParams Params(SCENE_QUERY_STAT(KineticLadderMount), false, this);
					if (GetWorld()->OverlapBlockingTestByChannel(Hold, FQuat::Identity, ECC_Pawn,
					                                             FCollisionShape::MakeCapsule(CapsuleRadiusCm - 5.0f, CapsuleHalfHeightCm - 5.0f), Params))
					{
						return;
					}
					SetActorLocation(Hold, false, nullptr, ETeleportType::TeleportPhysics);
					Ladder = Candidate;
					Movement->Velocity = FVector::ZeroVector;
					Movement->SetMovementMode(MOVE_Flying);
					return;
				}
			}
			return;
		}
		// Grab a ladder we are inside and pushing toward.
		if (ForwardInput <= 0.1f)
		{
			return;
		}
		TArray<AActor*> Overlaps;
		GetOverlappingActors(Overlaps, AKineticLadder::StaticClass());
		for (AActor* Actor : Overlaps)
		{
			AKineticLadder* Candidate = Cast<AKineticLadder>(Actor);
			if (Candidate && FVector::DotProduct(Facing, Candidate->IntoWall) > 0.5 && Feet < Candidate->TopZ() - 10.0f)
			{
				Ladder = Candidate;
				Movement->SetMovementMode(MOVE_Flying);
				break;
			}
		}
		return;
	}
	// Climbing: forward is up, back is down, at the class's ladder speed, held on the ladder's
	// line (a bump does not knock the climber off the rungs).
	const FVector Line = (Ladder->ClimbAt(GetActorLocation().Z) - GetActorLocation()) * 10.0;
	Movement->Velocity = FVector(FMath::Clamp(Line.X, -150.0, 150.0), FMath::Clamp(Line.Y, -150.0, 150.0),
	                             Cm(LadderSpeedM * LadderScale) * FMath::Clamp(ForwardInput, -1.0f, 1.0f));
	const float Feet = static_cast<float>(GetActorLocation().Z) - CapsuleHalfHeightCm;
	if (Feet >= Ladder->TopZ() - 5.0f)
	{
		// Off the top: onto whatever the ladder leads to, beside anyone standing there.
		const FVector StepOff = GetActorLocation() + Ladder->IntoWall * (CapsuleRadiusCm * 2.0f + 10.0f) +
		                        FVector(0.0, 0.0, 10.0);
		if (!TeleportTo(StepOff, GetActorRotation()))
		{
			Movement->Velocity = FVector::ZeroVector;  // No room yet: hold at the top
			return;
		}
		Ladder = nullptr;
		Movement->SetMovementMode(MOVE_Falling);
	}
	else if (FVector::Dist2D(GetActorLocation(), Ladder->ClimbAt(GetActorLocation().Z)) > 80.0 ||
	         (ForwardInput < -0.1f && FloorUnderFeet()))
	{
		Ladder = nullptr;
		Movement->SetMovementMode(MOVE_Falling);
	}
}

void AKineticCharacter::LeaveLadder()
{
	if (Ladder)
	{
		Ladder = nullptr;
		GetCharacterMovement()->SetMovementMode(MOVE_Falling);
	}
}

bool AKineticCharacter::FloorUnderFeet() const
{
	// Climbing down onto the ground: something to stand on just under the capsule.
	const FVector Feet = GetActorLocation() - FVector(0.0, 0.0, CapsuleHalfHeightCm);
	FHitResult Hit;
	FCollisionQueryParams Params(SCENE_QUERY_STAT(KineticLadderFoot), false, this);
	return GetWorld()->LineTraceSingleByChannel(Hit, Feet + FVector(0.0, 0.0, 5.0), Feet - FVector(0.0, 0.0, 10.0),
	                                            ECC_Visibility, Params) &&
	       Hit.ImpactNormal.Z > 0.7;
}

void AKineticCharacter::BuildInput()
{
	if (InputContext)
	{
		return;
	}
	auto Action = [this](const TCHAR* Name, EInputActionValueType Type) {
		UInputAction* Made = NewObject<UInputAction>(this, Name);
		Made->ValueType = Type;
		return Made;
	};
	MoveAction = Action(TEXT("IA_Move"), EInputActionValueType::Axis2D);
	LookAction = Action(TEXT("IA_Look"), EInputActionValueType::Axis2D);
	JumpAction = Action(TEXT("IA_Jump"), EInputActionValueType::Boolean);
	SprintAction = Action(TEXT("IA_Sprint"), EInputActionValueType::Boolean);
	CrouchAction = Action(TEXT("IA_Crouch"), EInputActionValueType::Boolean);
	FireAction = Action(TEXT("IA_Fire"), EInputActionValueType::Boolean);
	SelfDogAction = Action(TEXT("IA_SelfDog"), EInputActionValueType::Boolean);
	ModeActions[0] = Action(TEXT("IA_ModeHaul"), EInputActionValueType::Boolean);
	ModeActions[1] = Action(TEXT("IA_ModeJack"), EInputActionValueType::Boolean);
	ModeActions[2] = Action(TEXT("IA_ModeHang"), EInputActionValueType::Boolean);
	ReleaseAction = Action(TEXT("IA_Release"), EInputActionValueType::Boolean);
	DrawAction = Action(TEXT("IA_Draw"), EInputActionValueType::Boolean);
	CursorAction = Action(TEXT("IA_Cursor"), EInputActionValueType::Boolean);
	CallAction = Action(TEXT("IA_Call"), EInputActionValueType::Boolean);
	Pip4Action = Action(TEXT("IA_Pip4"), EInputActionValueType::Boolean);
	ThrowAction = Action(TEXT("IA_Throw"), EInputActionValueType::Boolean);
	InputContext = NewObject<UInputMappingContext>(this, TEXT("IMC_Kinetic"));

	// W A S D onto one 2D axis: W and S drive Y (swizzled), A and D drive X; S and A negated.
	auto MapMove = [this](FKey Key, bool bSwizzle, bool bNegate) {
		FEnhancedActionKeyMapping& Mapping = InputContext->MapKey(MoveAction, Key);
		if (bSwizzle)
		{
			Mapping.Modifiers.Add(NewObject<UInputModifierSwizzleAxis>(InputContext));
		}
		if (bNegate)
		{
			Mapping.Modifiers.Add(NewObject<UInputModifierNegate>(InputContext));
		}
	};
	MapMove(EKeys::W, true, false);
	MapMove(EKeys::S, true, true);
	MapMove(EKeys::D, false, false);
	MapMove(EKeys::A, false, true);
	FEnhancedActionKeyMapping& Mouse = InputContext->MapKey(LookAction, EKeys::Mouse2D);
	UInputModifierNegate* InvertY = NewObject<UInputModifierNegate>(InputContext);
	InvertY->bX = false;
	InvertY->bZ = false;
	Mouse.Modifiers.Add(InvertY);
	InputContext->MapKey(JumpAction, EKeys::SpaceBar);
	InputContext->MapKey(SprintAction, EKeys::LeftShift);
	InputContext->MapKey(CrouchAction, EKeys::C);
	// The Spool.
	InputContext->MapKey(FireAction, EKeys::LeftMouseButton);
	InputContext->MapKey(SelfDogAction, EKeys::E);
	InputContext->MapKey(ModeActions[0], EKeys::One);
	InputContext->MapKey(ModeActions[1], EKeys::Two);
	InputContext->MapKey(ModeActions[2], EKeys::Three);
	InputContext->MapKey(ReleaseAction, EKeys::R);
	InputContext->MapKey(DrawAction, EKeys::RightMouseButton);
	// Tag-Out: 1 to 3 are the Spool's mode keys in the other modes, the call strip's pips here.
	InputContext->MapKey(CallAction, EKeys::F);
	InputContext->MapKey(Pip4Action, EKeys::Four);
	InputContext->MapKey(ThrowAction, EKeys::G);
	// Escape lets go of the mouse (and takes it back).
	InputContext->MapKey(CursorAction, EKeys::Escape);
}

void AKineticCharacter::NotifyControllerChanged()
{
	Super::NotifyControllerChanged();
	BuildInput();
	if (const APlayerController* Player = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
		        ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(Player->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(InputContext, 0);
		}
	}
}

void AKineticCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	BuildInput();
	UEnhancedInputComponent* Input = Cast<UEnhancedInputComponent>(PlayerInputComponent);
	if (!Input)
	{
		UE_LOG(LogKineticCharacter, Error, TEXT("Enhanced Input is not the input component (Config/DefaultInput.ini)"));
		return;
	}
	Input->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AKineticCharacter::OnMove);
	Input->BindAction(MoveAction, ETriggerEvent::Completed, this, &AKineticCharacter::OnMove);
	Input->BindAction(LookAction, ETriggerEvent::Triggered, this, &AKineticCharacter::OnLook);
	Input->BindAction(JumpAction, ETriggerEvent::Started, this, &AKineticCharacter::OnJump);
	Input->BindAction(JumpAction, ETriggerEvent::Completed, this, &AKineticCharacter::OnJumpReleased);
	Input->BindAction(SprintAction, ETriggerEvent::Started, this, &AKineticCharacter::OnSprint);
	Input->BindAction(SprintAction, ETriggerEvent::Completed, this, &AKineticCharacter::OnSprint);
	Input->BindAction(CrouchAction, ETriggerEvent::Started, this, &AKineticCharacter::OnCrouch);
	Input->BindAction(CrouchAction, ETriggerEvent::Completed, this, &AKineticCharacter::OnCrouch);
	Input->BindAction(FireAction, ETriggerEvent::Started, this, &AKineticCharacter::OnFire);
	Input->BindAction(SelfDogAction, ETriggerEvent::Started, this, &AKineticCharacter::OnSelfDog);
	Input->BindAction(SelfDogAction, ETriggerEvent::Completed, this, &AKineticCharacter::OnUseReleased);
	Input->BindAction(CallAction, ETriggerEvent::Started, this, &AKineticCharacter::OnCall);
	Input->BindAction(Pip4Action, ETriggerEvent::Started, this, &AKineticCharacter::OnPip, 4);
	Input->BindAction(ThrowAction, ETriggerEvent::Started, this, &AKineticCharacter::OnThrow);
	Input->BindAction(ModeActions[0], ETriggerEvent::Started, this, &AKineticCharacter::OnMode, ELineModeInput::Haul);
	Input->BindAction(ModeActions[1], ETriggerEvent::Started, this, &AKineticCharacter::OnMode, ELineModeInput::Jack);
	Input->BindAction(ModeActions[2], ETriggerEvent::Started, this, &AKineticCharacter::OnMode, ELineModeInput::Hang);
	Input->BindAction(ReleaseAction, ETriggerEvent::Started, this, &AKineticCharacter::OnRelease);
	Input->BindAction(DrawAction, ETriggerEvent::Started, this, &AKineticCharacter::OnDraw);
	Input->BindAction(DrawAction, ETriggerEvent::Completed, this, &AKineticCharacter::OnDraw);
	Input->BindAction(CursorAction, ETriggerEvent::Started, this, &AKineticCharacter::OnCursor);
	// Any of them is input for the idle rules.
	for (UInputAction* Any : {MoveAction.Get(), LookAction.Get(), JumpAction.Get(), SprintAction.Get(), CrouchAction.Get(),
	                          FireAction.Get(), SelfDogAction.Get(), ModeActions[0].Get(), ModeActions[1].Get(),
	                          ModeActions[2].Get(), ReleaseAction.Get(), DrawAction.Get(), CallAction.Get(), Pip4Action.Get(),
	                          ThrowAction.Get()})
	{
		Input->BindAction(Any, ETriggerEvent::Triggered, this, &AKineticCharacter::NoteInput);
	}
}

void AKineticCharacter::NoteInput() { ++Inputs; }

void AKineticCharacter::OnFire()
{
	if (bStunned || bCursorFree)
	{
		return;
	}
	if (bTagOut)
	{
		Tag.bSwing = true;  // The Cutter is always in hand
		return;
	}
	if (UKineticSpoolWorld* Spool = GetWorld()->GetSubsystem<UKineticSpoolWorld>())
	{
		if (bDrawn)
		{
			Spool->Swing(this);  // With the Cutter out, the button swings it (05 4.1)
		}
		else
		{
			Spool->FireDog(this);
		}
	}
}

void AKineticCharacter::OnSelfDog()
{
	if (bTagOut)
	{
		Tag.bUse = !bCursorFree;
		return;
	}
	if (UKineticSpoolWorld* Spool = GetWorld()->GetSubsystem<UKineticSpoolWorld>())
	{
		Spool->SelfDogNext(this);
	}
}

void AKineticCharacter::OnMode(ELineModeInput Mode)
{
	if (bTagOut)
	{
		OnPip(Mode == ELineModeInput::Haul ? 1 : Mode == ELineModeInput::Jack ? 2 : 3);
		return;
	}
	if (UKineticSpoolWorld* Spool = GetWorld()->GetSubsystem<UKineticSpoolWorld>())
	{
		Spool->SelectMode(this, Mode == ELineModeInput::Haul   ? Kinetic::ELineMode::Haul
		                        : Mode == ELineModeInput::Jack ? Kinetic::ELineMode::Jack
		                                                       : Kinetic::ELineMode::Hang);
	}
}

void AKineticCharacter::OnRelease()
{
	if (bTagOut)
	{
		Tag.bClear = true;
		return;
	}
	if (UKineticSpoolWorld* Spool = GetWorld()->GetSubsystem<UKineticSpoolWorld>())
	{
		Spool->Release(this);
	}
}

void AKineticCharacter::OnDraw(const FInputActionValue& Value)
{
	if (bTagOut)
	{
		Tag.bLevel = Value.Get<bool>() && !bCursorFree;
		return;
	}
	bDrawn = Value.Get<bool>();
	if (UKineticSpoolWorld* Spool = GetWorld()->GetSubsystem<UKineticSpoolWorld>())
	{
		Spool->SetCutterDrawn(this, bDrawn);
	}
}

void AKineticCharacter::OnMove(const FInputActionValue& Value) { Move(Value.Get<FVector2D>()); }

void AKineticCharacter::OnCursor()
{
	APlayerController* Player = Cast<APlayerController>(Controller);
	if (!Player)
	{
		return;
	}
	bCursorFree = !bCursorFree;
	Player->SetShowMouseCursor(bCursorFree);
	if (bCursorFree)
	{
		FInputModeGameAndUI Mode;
		Mode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		Mode.SetHideCursorDuringCapture(false);
		Player->SetInputMode(Mode);
	}
	else
	{
		Player->SetInputMode(FInputModeGameOnly());
	}
}

void AKineticCharacter::OnLook(const FInputActionValue& Value)
{
	if (bCursorFree)
	{
		return;  // The mouse is the cursor's until Escape again
	}
	const FVector2D Axis = Value.Get<FVector2D>();
	AddControllerYawInput(static_cast<float>(Axis.X));
	AddControllerPitchInput(static_cast<float>(Axis.Y));
}

void AKineticCharacter::OnJump()
{
	if (!bMovementAllowed || bStunned)
	{
		return;
	}
	if (IsClimbing())
	{
		// Let go, pushed back off the ladder.
		LaunchCharacter(-Ladder->IntoWall * 250.0 + FVector(0.0, 0.0, 200.0), true, true);
		Ladder = nullptr;
		GetCharacterMovement()->SetMovementMode(MOVE_Falling);
		return;
	}
	if (!TryMantle())
	{
		Jump();
	}
}

void AKineticCharacter::OnJumpReleased() { StopJumping(); }

void AKineticCharacter::OnSprint(const FInputActionValue& Value) { SetSprinting(Value.Get<bool>()); }

void AKineticCharacter::OnCrouch(const FInputActionValue& Value)
{
	// C is the brace for the classes that have one (05 5.4: crouch while still), a crouch otherwise.
	if (UKineticSpoolWorld* Spool = GetWorld()->GetSubsystem<UKineticSpoolWorld>())
	{
		Spool->SetBrace(this, Value.Get<bool>());
	}
	if (Value.Get<bool>())
	{
		Crouch();
	}
	else
	{
		UnCrouch();
	}
}
