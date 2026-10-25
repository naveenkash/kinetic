#include "KineticCharacter.h"

#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/Engine.h"
#include "EngineUtils.h"
#include "EnhancedInputComponent.h"
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
	UCharacterMovementComponent* Movement = GetCharacterMovement();
	Movement->MaxWalkSpeed = Cm(bSprinting ? Row.SprintSpeed : Row.RunSpeed);
	Movement->MaxWalkSpeedCrouched = Cm(Row.CrouchSpeed);
	Movement->AirControl = static_cast<float>(Row.AirControl);
	Movement->Mass = static_cast<float>(Row.MassKg);
	// Take-off speed for the class's jump height: v = sqrt(2 g h).
	const double Gravity = FMath::Abs(Movement->GetGravityZ());
	Movement->JumpZVelocity = static_cast<float>(FMath::Sqrt(2.0 * Gravity * Kinetic::Units::MetersToCm(Row.JumpHeight)));
	// A ledge the class can mantle is never a step it walks up: steps stay at the engine's 45 cm.
	return true;
}

void AKineticCharacter::BeginPlay()
{
	Super::BeginPlay();
	ApplyHarness();
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
		GetCharacterMovement()->MaxWalkSpeed = Cm(bSprinting ? SprintSpeedM : RunSpeedM);
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
		Movement->MaxWalkSpeed = Cm(bSprinting ? SprintSpeedM : RunSpeedM);
	}
}

void AKineticCharacter::FirePressed() { OnFire(); }

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
	                             Cm(LadderSpeedM) * FMath::Clamp(ForwardInput, -1.0f, 1.0f));
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
	Input->BindAction(ModeActions[0], ETriggerEvent::Started, this, &AKineticCharacter::OnMode, ELineModeInput::Haul);
	Input->BindAction(ModeActions[1], ETriggerEvent::Started, this, &AKineticCharacter::OnMode, ELineModeInput::Jack);
	Input->BindAction(ModeActions[2], ETriggerEvent::Started, this, &AKineticCharacter::OnMode, ELineModeInput::Hang);
	Input->BindAction(ReleaseAction, ETriggerEvent::Started, this, &AKineticCharacter::OnRelease);
	Input->BindAction(DrawAction, ETriggerEvent::Started, this, &AKineticCharacter::OnDraw);
	Input->BindAction(DrawAction, ETriggerEvent::Completed, this, &AKineticCharacter::OnDraw);
	// Any of them is input for the idle rules.
	for (UInputAction* Any : {MoveAction.Get(), LookAction.Get(), JumpAction.Get(), SprintAction.Get(), CrouchAction.Get(),
	                          FireAction.Get(), SelfDogAction.Get(), ModeActions[0].Get(), ModeActions[1].Get(),
	                          ModeActions[2].Get(), ReleaseAction.Get(), DrawAction.Get()})
	{
		Input->BindAction(Any, ETriggerEvent::Triggered, this, &AKineticCharacter::NoteInput);
	}
}

void AKineticCharacter::NoteInput() { ++Inputs; }

void AKineticCharacter::OnFire()
{
	if (bStunned)
	{
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
	if (UKineticSpoolWorld* Spool = GetWorld()->GetSubsystem<UKineticSpoolWorld>())
	{
		Spool->SelfDogNext(this);
	}
}

void AKineticCharacter::OnMode(ELineModeInput Mode)
{
	if (UKineticSpoolWorld* Spool = GetWorld()->GetSubsystem<UKineticSpoolWorld>())
	{
		Spool->SelectMode(this, Mode == ELineModeInput::Haul   ? Kinetic::ELineMode::Haul
		                        : Mode == ELineModeInput::Jack ? Kinetic::ELineMode::Jack
		                                                       : Kinetic::ELineMode::Hang);
	}
}

void AKineticCharacter::OnRelease()
{
	if (UKineticSpoolWorld* Spool = GetWorld()->GetSubsystem<UKineticSpoolWorld>())
	{
		Spool->Release(this);
	}
}

void AKineticCharacter::OnDraw(const FInputActionValue& Value)
{
	bDrawn = Value.Get<bool>();
	if (UKineticSpoolWorld* Spool = GetWorld()->GetSubsystem<UKineticSpoolWorld>())
	{
		Spool->SetCutterDrawn(this, bDrawn);
	}
}

void AKineticCharacter::OnMove(const FInputActionValue& Value) { Move(Value.Get<FVector2D>()); }

void AKineticCharacter::OnLook(const FInputActionValue& Value)
{
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
