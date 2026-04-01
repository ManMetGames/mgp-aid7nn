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
	FVector GrapplePoint;
	float RopeLength = 0.f;

	UPROPERTY(EditAnywhere, Category = "Grapple")
	float MaxGrappleDistance = 3000.f;

	
	float DefaultBrakingFrictionFactor;
	float DefaultBrakingDeceleration;
	float DefaultAirControl;

	void StartGrapple();
	void StopGrapple();
	void AttachToSurface();
	void ReleaseGrapple();
	// Called every tick while attached - handles all swing physics
	void UpdateSwing(float DeltaTime);

public:
	void Input_StartGrapple();
	void Input_StopGrapple();
	void Input_StartReel();
	void Input_StopReel();

	float ForwardInputValue = 0.f;
	void SetForwardInput(float Value);
};