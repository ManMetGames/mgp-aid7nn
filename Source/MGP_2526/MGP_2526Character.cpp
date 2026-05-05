#include "MGP_2526Character.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "EnhancedInputComponent.h"
#include "InputActionValue.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Blueprint/UserWidget.h"
#include "Materials/MaterialParameterCollectionInstance.h"

AMGP_2526Character::AMGP_2526Character()
{
	// Set up collision capsule
	GetCapsuleComponent()->InitCapsuleSize(55.f, 96.f);

	// First person mesh - only visible to the owning player
	FirstPersonMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("First Person Mesh"));
	FirstPersonMesh->SetupAttachment(GetMesh());
	FirstPersonMesh->SetOnlyOwnerSee(true);
	FirstPersonMesh->FirstPersonPrimitiveType = EFirstPersonPrimitiveType::FirstPerson;
	FirstPersonMesh->SetCollisionProfileName(FName("NoCollision"));

	// Camera attached to the head socket of the first person mesh
	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("First Person Camera"));
	FirstPersonCameraComponent->SetupAttachment(FirstPersonMesh, FName("head"));
	FirstPersonCameraComponent->SetRelativeLocationAndRotation(FVector(-2.8f, 5.89f, 0.f), FRotator(0.f, 90.f, -90.f));
	FirstPersonCameraComponent->bUsePawnControlRotation = true;

	// Hide the third person mesh from the owning player
	GetMesh()->SetOwnerNoSee(true);
	GetCapsuleComponent()->SetCapsuleSize(34.f, 96.f);

	// Default movement values
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.f;
	GetCharacterMovement()->AirControl = 0.5f;

	// Create the grapple component
	GrappleComponent = CreateDefaultSubobject<UGrappleComponent>(TEXT("GrappleComponent"));

	// Enable tick so we can update the FOV and distortion every frame
	PrimaryActorTick.bCanEverTick = true;
}

void AMGP_2526Character::BeginPlay()
{
	Super::BeginPlay();

	// Only spawn the crosshair for the locally controlled player
	if (IsLocallyControlled() && CrosshairWidgetClass)
	{
		UUserWidget* Crosshair = CreateWidget<UUserWidget>(GetWorld(), CrosshairWidgetClass);
		if (Crosshair)
		{
			Crosshair->AddToViewport();
		}
	}

	// Set the camera to the default FOV on start
	if (FirstPersonCameraComponent)
	{
		FirstPersonCameraComponent->SetFieldOfView(DefaultFOV);
	}
}

void AMGP_2526Character::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	// Check if the grapple is currently attached
	bool bIsGrappling = GrappleComponent && GrappleComponent->GrappleState == EGrappleState::Attached;

	// Lerp the alpha toward 1 when grappling, back toward 0 when not
	float TargetAlpha = bIsGrappling ? 1.f : 0.f;
	GrappleEffectAlpha = FMath::FInterpTo(GrappleEffectAlpha, TargetAlpha, DeltaSeconds, GrappleFOVInterpSpeed);

	// Apply the FOV lerp to the camera
	if (FirstPersonCameraComponent)
	{
		float NewFOV = FMath::Lerp(DefaultFOV, GrappleFOV, GrappleEffectAlpha);
		FirstPersonCameraComponent->SetFieldOfView(NewFOV);
	}

	// Push the alpha into the material parameter collection to drive the edge distortion
	if (GrappleMPC)
	{
		UMaterialParameterCollectionInstance* MPCInstance = GetWorld()->GetParameterCollectionInstance(GrappleMPC);
		if (MPCInstance)
		{
			// GrappleEffectAlpha drives the distortion intensity in the post process material
			MPCInstance->SetScalarParameterValue(FName("GrappleEffectAlpha"), GrappleEffectAlpha);
		}
	}
}

void AMGP_2526Character::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		// Jump
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &AMGP_2526Character::DoJumpStart);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &AMGP_2526Character::DoJumpEnd);

		// Move and look
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AMGP_2526Character::MoveInput);
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AMGP_2526Character::LookInput);
		EnhancedInputComponent->BindAction(MouseLookAction, ETriggerEvent::Triggered, this, &AMGP_2526Character::LookInput);

		// Grapple inputs bound directly to the component
		EnhancedInputComponent->BindAction(GrappleAction, ETriggerEvent::Started, GrappleComponent, &UGrappleComponent::Input_StartGrapple);
		EnhancedInputComponent->BindAction(GrappleAction, ETriggerEvent::Completed, GrappleComponent, &UGrappleComponent::Input_StopGrapple);
		EnhancedInputComponent->BindAction(ReelAction, ETriggerEvent::Started, GrappleComponent, &UGrappleComponent::Input_StartReel);
		EnhancedInputComponent->BindAction(ReelAction, ETriggerEvent::Completed, GrappleComponent, &UGrappleComponent::Input_StopReel);
	}
}

void AMGP_2526Character::MoveInput(const FInputActionValue& Value)
{
	FVector2D MovementVector = Value.Get<FVector2D>();
	DoMove(MovementVector.X, MovementVector.Y);

	// Pass forward input to the grapple component for swing force
	if (GrappleComponent)
	{
		GrappleComponent->SetForwardInput(MovementVector.Y);
	}
}

void AMGP_2526Character::LookInput(const FInputActionValue& Value)
{
	FVector2D LookAxisVector = Value.Get<FVector2D>();
	DoAim(LookAxisVector.X, LookAxisVector.Y);
}

void AMGP_2526Character::DoAim(float Yaw, float Pitch)
{
	if (GetController())
	{
		AddControllerYawInput(Yaw);
		AddControllerPitchInput(Pitch);
	}
}

void AMGP_2526Character::DoMove(float Right, float Forward)
{
	if (GetController())
	{
		AddMovementInput(GetActorRightVector(), Right);
		AddMovementInput(GetActorForwardVector(), Forward);
	}
}

void AMGP_2526Character::DoJumpStart() { Jump(); }
void AMGP_2526Character::DoJumpEnd() { StopJumping(); }