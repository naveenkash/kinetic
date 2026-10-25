#include "KineticCharacter.h"

#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Engine/Engine.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "InputAction.h"
#include "InputMappingContext.h"
#include "InputModifiers.h"
#include "Kinetic/Units.h"
#include "KineticLadder.h"
#include "KineticRulesSubsystem.h"

DEFINE_LOG_CATEGORY_STATIC(LogKineticCharacter, Log, All);

namespace
{
// docs/tech/player-physics.md section 1: a 1.9 m capsule of 0.4 m radius; the eye 1.7 m up.
constexpr float CapsuleRadiusCm = 40.0f;
constexpr float CapsuleHalfHeightCm = 95.0f;
constexpr float EyeAboveCentreCm = 75.0f;

float Cm(double Meters) { return static_cast<float>(Kinetic::Units::MetersToCm(Meters)); }
}  // namespace

AKineticCharacter::AKineticCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
	GetCapsuleComponent()->InitCapsuleSize(CapsuleRadiusCm, CapsuleHalfHeightCm);
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(GetCapsuleComponent());
	Camera->SetRelativeLocation(FVector(0.0, 0.0, EyeAboveCentreCm));
	Camera->bUsePawnControlRotation = true;
	bUseControllerRotationYaw = true;
	UCharacterMovementComponent* Movement = GetCharacterMovement();
	Movement->NavAgentProps.bCanCrouch = true;
	Movement->bRunPhysicsWithNoController = true;  // Tests and bots move it without a player
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

void AKineticCharacter::SetSprinting(bool bSprint)
{
	bSprinting = bSprint;
	GetCharacterMovement()->MaxWalkSpeed = Cm(bSprinting ? SprintSpeedM : RunSpeedM);
}

void AKineticCharacter::Move(const FVector2D& Axis)
{
	ForwardInput = static_cast<float>(Axis.Y);
	if (IsClimbing())
	{
		return;  // On a ladder, forward is up: UpdateLadder reads ForwardInput
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
		// Grab a ladder we are inside and pushing toward.
		if (ForwardInput <= 0.1f)
		{
			return;
		}
		TArray<AActor*> Overlaps;
		GetOverlappingActors(Overlaps, AKineticLadder::StaticClass());
		const FVector Facing = FRotationMatrix(FRotator(0.0, GetActorRotation().Yaw, 0.0)).GetUnitAxis(EAxis::X);
		for (AActor* Actor : Overlaps)
		{
			AKineticLadder* Candidate = Cast<AKineticLadder>(Actor);
			if (Candidate && FVector::DotProduct(Facing, Candidate->IntoWall) > 0.5 &&
			    GetActorLocation().Z - CapsuleHalfHeightCm < Candidate->TopZ() - 10.0f)
			{
				Ladder = Candidate;
				Movement->SetMovementMode(MOVE_Flying);
				break;
			}
		}
		return;
	}
	// Climbing: forward is up, back is down, at the class's ladder speed.
	Movement->Velocity = FVector(0.0, 0.0, Cm(LadderSpeedM) * FMath::Clamp(ForwardInput, -1.0f, 1.0f));
	const float Feet = static_cast<float>(GetActorLocation().Z) - CapsuleHalfHeightCm;
	if (Feet >= Ladder->TopZ() - 5.0f)
	{
		// Off the top: onto whatever the ladder leads to.
		const FVector StepOff = GetActorLocation() + Ladder->IntoWall * (CapsuleRadiusCm * 2.0f + 10.0f) +
		                        FVector(0.0, 0.0, 10.0);
		SetActorLocation(StepOff, false, nullptr, ETeleportType::TeleportPhysics);
		Ladder = nullptr;
		Movement->SetMovementMode(MOVE_Falling);
	}
	else if (!IsOverlappingActor(Ladder) || (ForwardInput < -0.1f && Movement->IsMovingOnGround()))
	{
		Ladder = nullptr;
		Movement->SetMovementMode(MOVE_Falling);
	}
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
	if (Value.Get<bool>())
	{
		Crouch();
	}
	else
	{
		UnCrouch();
	}
}
