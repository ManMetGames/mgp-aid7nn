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