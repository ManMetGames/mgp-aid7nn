#include "GrappleComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/World.h"

UGrappleComponent::UGrappleComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UGrappleComponent::BeginPlay()
{
	Super::BeginPlay();

	// Cache the owning character and its movement component for later use
	OwnerCharacter = Cast<ACharacter>(GetOwner());
	if (OwnerCharacter)
	{
		MovementComponent = OwnerCharacter->GetCharacterMovement();
	}
}

void UGrappleComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

// Input stubs - logic will be added in later commits
void UGrappleComponent::Input_StartGrapple() {}
void UGrappleComponent::Input_StopGrapple() {}
void UGrappleComponent::Input_StartReel() {}
void UGrappleComponent::Input_StopReel() {}
void UGrappleComponent::SetForwardInput(float Value) { ForwardInputValue = Value; }