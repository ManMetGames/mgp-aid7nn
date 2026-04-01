#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GrappleComponent.generated.h"

class ACharacter;
class UCharacterMovementComponent;

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
	ACharacter* OwnerCharacter;
	UCharacterMovementComponent* MovementComponent;

	EGrappleState GrappleState = EGrappleState::Idle;

	// World position of the grapple hook point
	FVector GrapplePoint;
	// Current rope length, set when the grapple first attaches
	float RopeLength = 0.f;

	// How far the grapple hook can reach
	UPROPERTY(EditAnywhere, Category = "Grapple")
	float MaxGrappleDistance = 3000.f;

	// Fires a line trace and hooks onto whatever it hits
	void AttachToSurface();

public:
	void Input_StartGrapple();
	void Input_StopGrapple();
	void Input_StartReel();
	void Input_StopReel();

	float ForwardInputValue = 0.f;
	void SetForwardInput(float Value);
};