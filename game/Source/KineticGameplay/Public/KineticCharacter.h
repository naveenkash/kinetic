// The player in a harness: a first-person character whose movement is its class's row of
// DT_HarnessClass (bible/05 section 2.2: run, sprint, jump height, mantle height, air control,
// crouch speed, mass, ladder speed), read from KineticCore's tables. This is the movement
// capsule of docs/tech/player-physics.md section 1; the flight body and the Spool come next.
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "KineticHarness.h"

#include "KineticCharacter.generated.h"

class AKineticLadder;
class UCameraComponent;
class UInputAction;
class UInputMappingContext;
struct FInputActionValue;

UCLASS()
class KINETICGAMEPLAY_API AKineticCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AKineticCharacter();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Kinetic")
	EKineticHarness Harness = EKineticHarness::Rigger;

	UPROPERTY(VisibleAnywhere, Category = "Kinetic")
	TObjectPtr<UCameraComponent> Camera;

	// Sets the movement from the class's table row. Called at BeginPlay; call again after
	// changing Harness at run time. False (and logged) if the tables did not load.
	bool ApplyHarness();

	// The class's figures, in metres and seconds, as the tables give them.
	float SprintSpeed() const { return SprintSpeedM; }
	float RunSpeed() const { return RunSpeedM; }
	float MantleHeight() const { return MantleHeightM; }
	float LadderSpeed() const { return LadderSpeedM; }

	void SetSprinting(bool bSprint);
	bool IsSprinting() const { return bSprinting; }
	bool IsClimbing() const { return Ladder != nullptr; }

	// Climbs onto a ledge within the class's mantle height in front, if there is one and the
	// character fits on top. Returns whether it did.
	bool TryMantle();

	// Movement for tests and bots: the same as the move and look actions.
	void Move(const FVector2D& Axis);

	virtual void Tick(float DeltaSeconds) override;

protected:
	virtual void BeginPlay() override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	virtual void NotifyControllerChanged() override;

private:
	void OnMove(const FInputActionValue& Value);
	void OnLook(const FInputActionValue& Value);
	void OnJump();
	void OnJumpReleased();
	void OnSprint(const FInputActionValue& Value);
	void OnCrouch(const FInputActionValue& Value);
	void UpdateLadder(float DeltaSeconds);
	void BuildInput();

	float RunSpeedM = 6.2f, SprintSpeedM = 7.5f, MantleHeightM = 2.2f, LadderSpeedM = 2.0f;
	bool bSprinting = false;
	float ForwardInput = 0.0f;

	UPROPERTY()
	TObjectPtr<AKineticLadder> Ladder;

	// The input, made in code (no assets): see BuildInput.
	UPROPERTY()
	TObjectPtr<UInputMappingContext> InputContext;
	UPROPERTY()
	TObjectPtr<UInputAction> MoveAction;
	UPROPERTY()
	TObjectPtr<UInputAction> LookAction;
	UPROPERTY()
	TObjectPtr<UInputAction> JumpAction;
	UPROPERTY()
	TObjectPtr<UInputAction> SprintAction;
	UPROPERTY()
	TObjectPtr<UInputAction> CrouchAction;
};
