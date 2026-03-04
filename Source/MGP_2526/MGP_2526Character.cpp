// Copyright Epic Games, Inc. All Rights Reserved.

#include "MGP_2526Character.h"
#include "Animation/AnimInstance.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "EnhancedInputComponent.h"
#include "InputActionValue.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "MGP_2526.h"
#include <Net/UnrealNetwork.h>
#include "Kismet/KismetMathLibrary.h"
#include "Blueprint/UserWidget.h"

AMGP_2526Character::AMGP_2526Character()
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

void AMGP_2526Character::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{	
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		// Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &AMGP_2526Character::DoJumpStart);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &AMGP_2526Character::DoJumpEnd);

		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AMGP_2526Character::MoveInput);

		// Looking/Aiming
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AMGP_2526Character::LookInput);
		EnhancedInputComponent->BindAction(MouseLookAction, ETriggerEvent::Triggered, this, &AMGP_2526Character::LookInput);

		//Grappling
		EnhancedInputComponent->BindAction(GrappleAction, ETriggerEvent::Started, this, &AMGP_2526Character::GrapplePressed);
		EnhancedInputComponent->BindAction(GrappleAction, ETriggerEvent::Completed, this, &AMGP_2526Character::GrappleReleased);
	}
	else
	{
		UE_LOG(LogMGP_2526, Error, TEXT("'%s' Failed to find an Enhanced Input Component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}

void AMGP_2526Character::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AMGP_2526Character, bIsGrappling);
	DOREPLIFETIME(AMGP_2526Character, CurrentGrapplePoint);
	DOREPLIFETIME(AMGP_2526Character, CurrentGrappleDistance);
}


void AMGP_2526Character::MoveInput(const FInputActionValue& Value)
{
	// get the Vector2D move axis
	FVector2D MovementVector = Value.Get<FVector2D>();

	// pass the axis values to the move input
	DoMove(MovementVector.X, MovementVector.Y);

}

void AMGP_2526Character::LookInput(const FInputActionValue& Value)
{
	// get the Vector2D look axis
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	// pass the axis values to the aim input
	DoAim(LookAxisVector.X, LookAxisVector.Y);

}

void AMGP_2526Character::DoAim(float Yaw, float Pitch)
{
	if (GetController())
	{
		// pass the rotation inputs
		AddControllerYawInput(Yaw);
		AddControllerPitchInput(Pitch);
	}
}

void AMGP_2526Character::DoMove(float Right, float Forward)
{
	if (GetController())
	{
		// pass the move inputs
		AddMovementInput(GetActorRightVector(), Right);
		AddMovementInput(GetActorForwardVector(), Forward);
	}
}

void AMGP_2526Character::DoJumpStart()
{
	// pass Jump to the character
	Jump();
}

void AMGP_2526Character::DoJumpEnd()
{
	// pass StopJumping to the character
	StopJumping();
}


void AMGP_2526Character::BeginPlay()
{
	Super::BeginPlay();

	if (IsLocallyControlled() && CrosshairWidgetClass)
	{
		UUserWidget* Crosshair = CreateWidget<UUserWidget>(GetWorld(), CrosshairWidgetClass);
		if (Crosshair)
		{
			Crosshair->AddToViewport();
		}
	}
}



//  *** GRAPPLE MECHANIC IMPLEMENTATION ***
void AMGP_2526Character::TrySetGrappleLocal()
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
void AMGP_2526Character::RegisterNewGrapplePoint(FVector GrapplePoint)
{
	if (IsGrapplePointValid(GrapplePoint))
	{
		CurrentGrapplePoint = GrapplePoint;
		CurrentGrappleDistance = (GrapplePoint - GetActorLocation()).Length();  //
		bIsGrappling = true;
	}

}

void AMGP_2526Character::SetGrapplePointLocal(FVector GrapplePoint)
{
	RegisterNewGrapplePoint(GrapplePoint);
	SetGrapplePointServer(GrapplePoint);
}

void AMGP_2526Character::SetGrapplePointServer_Implementation(FVector GrapplePoint)
{
	RegisterNewGrapplePoint(GrapplePoint);
}

bool AMGP_2526Character::IsGrapplePointValid(FVector GrapplePoint)
{
	//distance to grapple point greater than max distance  (CALLED WHEN STARTING GRAPPLE)
	FVector GrappleDiff = GrapplePoint - GetActorLocation();
	if (GrappleDiff.Length() > MaxGrappleDistance)
	{
		return false;
	}
	return true;
}

void AMGP_2526Character::ApplyGrapple()
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

void AMGP_2526Character::StopGrappleLocal()
{
	bIsGrappling = false;
	StopGrappleRemote();
}

void AMGP_2526Character::GrapplePressed(const FInputActionValue& Input)
{
	TrySetGrappleLocal();
}

void AMGP_2526Character::GrappleReleased(const FInputActionValue& Input)
{
	StopGrappleLocal();
}

void AMGP_2526Character::StopGrappleRemote_Implementation()
{
	bIsGrappling = false;
}


void AMGP_2526Character::Tick(float DeltaSeconds)
{
	//everytime we grapple, it applies the forces to keep us in the air/falling
	Super::Tick(DeltaSeconds);
	ApplyGrapple();
}
