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
		if (MovementComponent)
		{
			// Save defaults so we can restore them when the grapple is released
			DefaultBrakingFrictionFactor = MovementComponent->BrakingFrictionFactor;
			DefaultBrakingDeceleration = MovementComponent->BrakingDecelerationFalling;
			DefaultAirControl = MovementComponent->AirControl;
		}
	}
}

void UGrappleComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// Run swing physics every frame while hooked
	if (GrappleState == EGrappleState::Attached)
	{
		UpdateSwing(DeltaTime);
	}
}

void UGrappleComponent::AttachToSurface()
{
	if (!OwnerCharacter) return;

	FVector Start;
	FRotator Rotation;
	OwnerCharacter->GetController()->GetPlayerViewPoint(Start, Rotation);
	FVector End = Start + (Rotation.Vector() * MaxGrappleDistance);

	FHitResult Hit;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(OwnerCharacter);

	bool bHit = GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params);
	if (bHit && Hit.Component.IsValid() && Hit.Component->GetCollisionObjectType() == ECC_WorldStatic)
	{
		GrapplePoint = Hit.Location;
		RopeLength = FVector::Distance(OwnerCharacter->GetActorLocation(), GrapplePoint);
		GrappleState = EGrappleState::Attached;

		// Zero out braking and air control so grapple physics fully control movement
		MovementComponent->SetMovementMode(MOVE_Falling);
		MovementComponent->BrakingFrictionFactor = 0.f;
		MovementComponent->BrakingDecelerationFalling = 0.f;
		MovementComponent->AirControl = 0.f;

		DrawDebugSphere(GetWorld(), GrapplePoint, 20.f, 12, FColor::Green, false, 2.f);
	}
}

void UGrappleComponent::ReleaseGrapple()
{
	GrappleState = EGrappleState::Idle;

	if (!MovementComponent) return;

	// Restore air control so the player can steer normally again
	MovementComponent->AirControl = DefaultAirControl;
	MovementComponent->SetMovementMode(MOVE_Falling);
}

void UGrappleComponent::UpdateSwing(float DeltaTime)
{
	if (!OwnerCharacter || !MovementComponent) return;

	FVector PlayerLocation = OwnerCharacter->GetActorLocation();
	FVector ToAnchor = GrapplePoint - PlayerLocation;
	float CurrentDistance = ToAnchor.Size();
	if (CurrentDistance <= 1.f) return;

	FVector RopeDirection = ToAnchor / CurrentDistance;
	FVector Velocity = MovementComponent->Velocity;

	// 1. Remove any velocity component pulling the player away from the anchor
	//    so the rope never stretches outward
	float RadialVelocityAmount = FVector::DotProduct(Velocity, RopeDirection);
	if (RadialVelocityAmount < 0.f) Velocity -= RopeDirection * RadialVelocityAmount;

	// 2. If the player drifts past the rope length, snap them back to the edge
	if (CurrentDistance > RopeLength)
	{
		FVector ClampedPosition = GrapplePoint - RopeDirection * RopeLength;
		OwnerCharacter->SetActorLocation(ClampedPosition);

		float NewRadial = FVector::DotProduct(Velocity, RopeDirection);
		if (NewRadial < 0.f) Velocity -= RopeDirection * NewRadial;
	}

	MovementComponent->Velocity = Velocity;

	// Draw the rope as a red line each frame for debugging
	DrawDebugLine(GetWorld(), OwnerCharacter->GetActorLocation(), GrapplePoint, FColor::Red, false, 0.f, 0, 2.f);
}

void UGrappleComponent::StartGrapple()
{
	AttachToSurface(); 
}

void UGrappleComponent::StopGrapple()
{
	ReleaseGrapple(); 
}

void UGrappleComponent::Input_StartGrapple()
{ 
	StartGrapple(); 
}
void UGrappleComponent::Input_StopGrapple()
{
	StopGrapple(); 
}

void UGrappleComponent::Input_StartReel() 
{

}

void UGrappleComponent::Input_StopReel() 
{
	
}

void UGrappleComponent::SetForwardInput(float Value)
{
	ForwardInputValue = Value; 
}