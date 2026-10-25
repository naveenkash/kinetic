// The player in a harness: a first-person character whose movement is its class's row of
// DT_HarnessClass (bible/05 section 2.2: run, sprint, jump height, mantle height, air control,
// crouch speed, mass, ladder speed), read from KineticCore's tables, carrying the Spool and the
// Cutter (UKineticSpoolWorld solves the lines). This is the movement capsule of
// docs/tech/player-physics.md section 1; the flight body comes with impacts.
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Kinetic/Impact.h"
#include "KineticHarness.h"

#include <vector>

#include "KineticCharacter.generated.h"

class AKineticLadder;
class UKineticSpoolWorld;

// The Spool's mode keys (1, 2, 3).
enum class ELineModeInput : uint8
{
	Haul,
	Jack,
	Hang
};
class UCameraComponent;
class UInputAction;
class UInputMappingContext;
struct FInputActionValue;

UCLASS()
class KINETICGAMEPLAY_API AKineticCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	explicit AKineticCharacter(const FObjectInitializer& ObjectInitializer);

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

	// A Haul or Jack on a grounded player overrides their movement (05 5.1): set by the Spool
	// each step. While it is, input is ignored and the ground does not brake the skid. Braced,
	// the player holds still.
	void SetUnderLine(bool bLine, bool bBraced);
	bool IsUnderLine() const { return bUnderLine; }

	// The Spool's buttons, for tests and bots.
	void FirePressed();

	// What the capsule hit since the Spool last asked: the Impact Monitor's contacts for the
	// step (player-physics 4.1), taken and cleared by UKineticSpoolWorld.
	std::vector<Kinetic::FContact> TakeContacts();

	// Staggered, flipped or Stalled: no input until the Spool says otherwise.
	void SetStunned(bool bInStunned) { bStunned = bInStunned; }
	bool IsStunned() const { return bStunned; }

	// One contact for the Impact Monitor: what was hit, at the velocity that hit it (cm/s), and
	// the impulse Chaos reported (kg cm/s; zero for a sweep).
	void RecordContact(const FHitResult& Hit, const FVector& MyVelocity, const FVector& NormalImpulse);

	virtual void Landed(const FHitResult& Hit) override;
	virtual void NotifyHit(UPrimitiveComponent* MyComp, AActor* Other, UPrimitiveComponent* OtherComp, bool bSelfMoved,
	                       FVector HitLocation, FVector HitNormal, FVector NormalImpulse, const FHitResult& Hit) override;

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
	void OnFire();
	void OnSelfDog();
	void OnMode(ELineModeInput Mode);
	void OnRelease();
	void OnDraw(const FInputActionValue& Value);
	void UpdateLadder(float DeltaSeconds);
	void BuildInput();

	float RunSpeedM = 6.2f, SprintSpeedM = 7.5f, MantleHeightM = 2.2f, LadderSpeedM = 2.0f;
	bool bSprinting = false;
	float ForwardInput = 0.0f;
	bool bUnderLine = false;
	bool bHeldStill = false;
	bool bStunned = false;
	std::vector<Kinetic::FContact> Contacts;
	bool bDrawn = false;
	float NormalFriction = 8.0f, NormalBraking = 2048.0f;

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
	UPROPERTY()
	TObjectPtr<UInputAction> FireAction;
	UPROPERTY()
	TObjectPtr<UInputAction> SelfDogAction;
	UPROPERTY()
	TObjectPtr<UInputAction> ModeActions[3];
	UPROPERTY()
	TObjectPtr<UInputAction> ReleaseAction;
	UPROPERTY()
	TObjectPtr<UInputAction> DrawAction;
};
