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
class UAnimSequence;
class UCameraComponent;
class UStaticMeshComponent;
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

	// The crew (0 or 1; 0 in the walk-the-arena mode): its team tint (characters/README.md,
	// Amber and Sky) is on the body and the lines.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Kinetic")
	int32 Crew = 0;
	void SetCrew(int32 InCrew);
	static FLinearColor TeamTint(int32 Crew);

	// What other players see of the character (the owner does not): the class's generated
	// figure (art/characters, /Game/Characters/<Class>) on the character's skeletal mesh,
	// playing its takes from the character's state; without those assets, a grey-box column in the
	// crew's colour with a visor on the facing side.
	UPROPERTY(VisibleAnywhere, Category = "Kinetic")
	TObjectPtr<UStaticMeshComponent> Body;
	UPROPERTY(VisibleAnywhere, Category = "Kinetic")
	TObjectPtr<UStaticMeshComponent> Visor;

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
	const AKineticLadder* GetLadder() const { return Ladder; }
	// Lets go of the ladder (a reset or a Stall moves the character off it).
	void LeaveLadder();
	// Whether pressing into a ladder takes hold of it (or backing over its top climbs down). On
	// for players; a bot turns it on only when its route takes the ladder.
	void SetTakesLadders(bool bTakes) { bTakesLadders = bTakes; }

	// The match's hold (game-modes 2: no movement until 2:26 of the Drop). Held, move, jump and
	// the ladder are ignored.
	void SetMovementAllowed(bool bAllowed);
	bool IsMovementAllowed() const { return bMovementAllowed; }

	// Counts the owner's own input (any key or the mouse), for the match's idle rules.
	uint32 InputCount() const { return Inputs; }

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
	// Jump, or mantle a ledge in front, or let go of a ladder: the jump key.
	void JumpPressed() { OnJump(); }

	// What the capsule hit since the Spool last asked: the Impact Monitor's contacts for the
	// step (player-physics 4.1), taken and cleared by UKineticSpoolWorld.
	std::vector<Kinetic::FContact> TakeContacts();

	// Staggered, flipped or Stalled: no input until the Spool says otherwise.
	void SetStunned(bool bInStunned) { bStunned = bInStunned; }
	bool IsStunned() const { return bStunned; }

	// One contact for the Impact Monitor: what was hit, at the velocity that hit it (cm/s), and
	// the impulse Chaos reported (kg cm/s; zero for a sweep).
	void RecordContact(const FHitResult& Hit, const FVector& MyVelocity, const FVector& NormalImpulse);
	// A contact made by something else (a kinematic running into the player).
	void AddContact(const Kinetic::FContact& Contact) { Contacts.push_back(Contact); }

	virtual void Landed(const FHitResult& Hit) override;
	virtual void NotifyHit(UPrimitiveComponent* MyComp, AActor* Other, UPrimitiveComponent* OtherComp, bool bSelfMoved,
	                       FVector HitLocation, FVector HitNormal, FVector NormalImpulse, const FHitResult& Hit) override;

	virtual void Tick(float DeltaSeconds) override;

	// The figure's takes, in the order of Takes (Idle, Run, Climb, Fall, Brace, Stagger).
	enum class EFigureTake : uint8 { Idle, Run, Climb, Fall, Brace, Stagger, Count };
	bool HasFigure() const { return bFigure; }
	EFigureTake FigureTake() const { return Take; }

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
	void LoadFigure();
	void UpdateFigure();
	bool FloorUnderFeet() const;
	void NoteInput();
	void BuildInput();

	float RunSpeedM = 6.2f, SprintSpeedM = 7.5f, MantleHeightM = 2.2f, LadderSpeedM = 2.0f;
	bool bSprinting = false;
	float ForwardInput = 0.0f;
	bool bUnderLine = false;
	bool bHeldStill = false;
	bool bStunned = false;
	bool bMovementAllowed = true;
	bool bTakesLadders = true;
	bool bFigure = false;
	EFigureTake Take = EFigureTake::Count;
	double StaggerUntil = 0.0;
	UPROPERTY()
	TObjectPtr<UAnimSequence> Takes[static_cast<int32>(EFigureTake::Count)];
	uint32 Inputs = 0;
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
