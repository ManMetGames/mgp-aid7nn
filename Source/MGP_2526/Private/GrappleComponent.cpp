#include "GrappleComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"

UGrappleComponent::UGrappleComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UGrappleComponent::BeginPlay()
{
	Super::BeginPlay();

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

void UGrappleComponent::AttachToSurface()
{
	if (!OwnerCharacter)
	{
		return;
	}

	// Get the camera position and direction to fire the trace from
	FVector Start;
	FRotator Rotation;
	OwnerCharacter->GetController()->GetPlayerViewPoint(Start, Rotation);
	FVector End = Start + (Rotation.Vector() * MaxGrappleDistance);

	FHitResult Hit;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(OwnerCharacter); // ignore self so we don't hit our own capsule

	bool bHit = GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params);

	// Only attach if we hit a static world surface (not dynamic objects or pawns)
	if (bHit && Hit.Component.IsValid() && Hit.Component->GetCollisionObjectType() == ECC_WorldStatic)
	{
		GrapplePoint = Hit.Location;
		RopeLength = FVector::Distance(OwnerCharacter->GetActorLocation(), GrapplePoint);

		// Draw a debug sphere at the hook point so we can see where it landed
		DrawDebugSphere(GetWorld(), GrapplePoint, 20.f, 12, FColor::Green, false, 2.f);
	}
}

void UGrappleComponent::Input_StartGrapple()
{
	AttachToSurface(); 

}
void UGrappleComponent::Input_StopGrapple()
{
	//
}

void UGrappleComponent::Input_StartReel() 
{
	//
}

void UGrappleComponent::Input_StopReel() 
{
	//
}

void UGrappleComponent::SetForwardInput(float Value) 
{
	ForwardInputValue = Value; 
}