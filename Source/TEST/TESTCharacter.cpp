// Copyright Epic Games, Inc. All Rights Reserved.

#include "TESTCharacter.h"
#include "Animation/AnimInstance.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "EnhancedInputComponent.h"
#include "InputActionValue.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "TEST.h"
#include <Net/UnrealNetwork.h>
#include "Kismet/KismetMathLibrary.h"

ATESTCharacter::ATESTCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(55.f, 96.0f);
	
	// Create the first person mesh that will be viewed only by this character's owner
	FirstPersonMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("First Person Mesh"));

	FirstPersonMesh->SetupAttachment(GetMesh());
	FirstPersonMesh->SetOnlyOwnerSee(true);
	FirstPersonMesh->FirstPersonPrimitiveType = EFirstPersonPrimitiveType::FirstPerson;
	FirstPersonMesh->SetCollisionProfileName(FName("NoCollision"));

	// Create the Camera Component	
	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("First Person Camera"));
	FirstPersonCameraComponent->SetupAttachment(FirstPersonMesh, FName("head"));
	FirstPersonCameraComponent->SetRelativeLocationAndRotation(FVector(-2.8f, 5.89f, 0.0f), FRotator(0.0f, 90.0f, -90.0f));
	FirstPersonCameraComponent->bUsePawnControlRotation = true;
	FirstPersonCameraComponent->bEnableFirstPersonFieldOfView = true;
	FirstPersonCameraComponent->bEnableFirstPersonScale = true;
	FirstPersonCameraComponent->FirstPersonFieldOfView = 70.0f;
	FirstPersonCameraComponent->FirstPersonScale = 0.6f;

	// configure the character comps
	GetMesh()->SetOwnerNoSee(true);
	GetMesh()->FirstPersonPrimitiveType = EFirstPersonPrimitiveType::WorldSpaceRepresentation;

	GetCapsuleComponent()->SetCapsuleSize(34.0f, 96.0f);

	// Configure character movement
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;
	GetCharacterMovement()->AirControl = 0.5f;
}

void ATESTCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{	
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		// Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ATESTCharacter::DoJumpStart);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ATESTCharacter::DoJumpEnd);

		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ATESTCharacter::MoveInput);

		// Looking/Aiming
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ATESTCharacter::LookInput);
		EnhancedInputComponent->BindAction(MouseLookAction, ETriggerEvent::Triggered, this, &ATESTCharacter::LookInput);

		//Grappling
		EnhancedInputComponent->BindAction(GrappleAction, ETriggerEvent::Triggered, this, &ATESTCharacter::GrapplePressed);
		EnhancedInputComponent->BindAction(GrappleAction, ETriggerEvent::Triggered, this, &ATESTCharacter::GrappleReleased);
	}
	else
	{
		UE_LOG(LogTEST, Error, TEXT("'%s' Failed to find an Enhanced Input Component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}

void ATESTCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps)
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ATESTCharacter, bIsGrappling);
	DOREPLIFETIME(ATESTCharacter, CurrentGrapplePoint);
	DOREPLIFETIME(ATESTCharacter, CurrentGrappleDistance);
}


void ATESTCharacter::MoveInput(const FInputActionValue& Value)
{
	// get the Vector2D move axis
	FVector2D MovementVector = Value.Get<FVector2D>();

	// pass the axis values to the move input
	DoMove(MovementVector.X, MovementVector.Y);

}

void ATESTCharacter::LookInput(const FInputActionValue& Value)
{
	// get the Vector2D look axis
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	// pass the axis values to the aim input
	DoAim(LookAxisVector.X, LookAxisVector.Y);

}

void ATESTCharacter::DoAim(float Yaw, float Pitch)
{
	if (GetController())
	{
		// pass the rotation inputs
		AddControllerYawInput(Yaw);
		AddControllerPitchInput(Pitch);
	}
}

void ATESTCharacter::DoMove(float Right, float Forward)
{
	if (GetController())
	{
		// pass the move inputs
		AddMovementInput(GetActorRightVector(), Right);
		AddMovementInput(GetActorForwardVector(), Forward);
	}
}

void ATESTCharacter::DoJumpStart()
{
	// pass Jump to the character
	Jump();
}

void ATESTCharacter::DoJumpEnd()
{
	// pass StopJumping to the character
	StopJumping();
}

void ATESTCharacter::TrySetGrappleLocal()
{
	UCameraComponent* Cam = GetFirstPersonCameraComponent();
	FVector Start = Cam->GetComponentLocation();   //camera location
	float TraceMaxDistance = 10000.0f;
	FVector End = Start + Cam->GetForwardVector() * TraceMaxDistance;

	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);   //added "this" so player doesnt hit self in trace

	FHitResult OutHit;
	if (GetWorld()->LineTraceSingleByChannel(OutHit, Start, End, ECC_Visibility, Params))
	{
		SetGrapplePointLocal(OutHit.ImpactPoint);   //pass impact point from OutHit to function   (as GrapplePoint)
	} 

}

void ATESTCharacter::RegisterNewGrapplePoint(FVector GrapplePoint)
{
	if (IsGrapplePointValid(GrapplePoint))
	{
		CurrentGrapplePoint = GrapplePoint;
		CurrentGrappleDistance = (GrapplePoint - GetActorLocation()).Length();  //
		bIsGrappling = true;
	}

}

void ATESTCharacter::SetGrapplePointLocal(FVector GrapplePoint)
{
	RegisterNewGrapplePoint(GrapplePoint);
	SetGrapplePointServer(GrapplePoint);
}

void ATESTCharacter::SetGrapplePointServer(FVector GrapplePoint)
{
	RegisterNewGrapplePoint(GrapplePoint);
}

bool ATESTCharacter::IsGrapplePointValid(FVector GrapplePoint)
{
	//distance to grapple point greater than max distance  (CALLED WHEN STARTING GRAPPLE)
	FVector GrappleDiff = GrapplePoint - GetActorLocation();  
	if (GrappleDiff.Length() > MaxGrappleDistance) 
	{
		return false;
	}
	return true;
}

void ATESTCharacter::ApplyGrapple()
{
	if (!bIsGrappling)   //check if grappling
	{
		return;
	}
	FVector GrappleDiff = CurrentGrapplePoint - GetActorLocation();  //difference between actor location and grapple point

	//if difference is less than current grapple distance, return, because there is slack on the rope.
	if (GrappleDiff.Length() < CurrentGrappleDistance)
	{
		return;
	}

	FVector Vel = GetVelocity();
	FVector NormalTowardsGrapplePoint = GrappleDiff.GetSafeNormal();
	FVector VelNormal = Vel.GetSafeNormal();

	float AngleBetweenVelocityAndGrapple = UKismetMathLibrary::DegAcos(VelNormal.Dot(NormalTowardsGrapplePoint));

	//moving towards the grapple point. do nothing
	if (AngleBetweenVelocityAndGrapple < 90)
	{
		return;
	}

	FVector NewVelo = UKismetMathLibrary::ProjectVectorOnToPlane(Vel, NormalTowardsGrapplePoint);
	LaunchCharacter(NewVelo, true, true);  //override x,y,z values
}

void ATESTCharacter::StopGrappleLocal()
{
	bIsGrappling = false;
	StopGrappleRemote();
}

void ATESTCharacter::GrapplePressed(const FInputActionValue& Input)
{
	TrySetGrappleLocal();
}

void ATESTCharacter::GrappleReleased(const FInputActionValue& Input)
{
	StopGrappleLocal();
}

void ATESTCharacter::StopGrappleRemote()
{
	bIsGrappling = false;
}


void ATESTCharacter::Tick(float DeltaSeconds)
{
	//everytime we grapple, it applies the forces to keep us in the air/falling
	Super::Tick(DeltaSeconds);
	ApplyGrapple();
}
