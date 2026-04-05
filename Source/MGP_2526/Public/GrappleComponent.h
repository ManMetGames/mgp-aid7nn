#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GrappleComponent.generated.h"

class ACharacter;
class UCharacterMovementComponent;

// Tracks whether the grapple is idle or hooked onto something
UENUM(BlueprintType)
enum class EGrappleState : uint8
{
	Idle,
	Attached
};

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class MGP_2526_API UGrappleComponent : public UActorComponent
{
	GENERATED_BODY()
public:
	UGrappleComponent();

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(
		float DeltaTime,
		ELevelTick TickType,
		FActorComponentTickFunction* ThisTickFunction
	) override;

private:
	// Cached references set up in BeginPlay
	ACharacter* OwnerCharacter;
	UCharacterMovementComponent* MovementComponent;

	// Current state of the grapple
	EGrappleState GrappleState = EGrappleState::Idle;

	// World position of the grapple hook point
	FVector GrapplePoint;
	// Current rope length, set when the grapple first attaches
	float RopeLength = 0.f;

	// How far the grapple hook can reach
	UPROPERTY(EditAnywhere, Category = "Grapple")
	float MaxGrappleDistance = 3000.f;

	// How fast the rope shortens when reeling
	UPROPERTY(EditAnywhere, Category = "Grapple")
	float ReelSpeed = 1200.f;

	// The rope cannot be shortened below this length
	UPROPERTY(EditAnywhere, Category = "Grapple")
	float MinRopeLength = 300.f;

	// Saved so we can restore them when the grapple is released
	float DefaultBrakingFrictionFactor;
	float DefaultBrakingDeceleration;
	float DefaultAirControl;

	// Fires a line trace and hooks onto whatever it hits
	void AttachToSurface();
	// Detaches the grapple and restores movement settings
	void ReleaseGrapple();
	// Called every tick while attached - handles all swing physics
	void UpdateSwing(float DeltaTime);

	void StartGrapple();
	void StopGrapple();

	// Whether the player is currently holding the reel button
	bool bIsReeling = false;

public:
	// Input entry points called from the character
	void Input_StartGrapple();
	void Input_StopGrapple();
	void Input_StartReel();
	void Input_StopReel();

	// Forward input passed in from the character each frame
	float ForwardInputValue = 0.f;
	void SetForwardInput(float Value);
};