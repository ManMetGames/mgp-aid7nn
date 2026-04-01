#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"
#include "GrappleComponent.h"
#include "MGP_2526Character.generated.h"

class UInputComponent;
class USkeletalMeshComponent;
class UCameraComponent;
class UInputAction;
struct FInputActionValue;

DECLARE_LOG_CATEGORY_EXTERN(LogTemplateCharacter, Log, All);

UCLASS(abstract)
class AMGP_2526Character : public ACharacter
{
	GENERATED_BODY()

	/** First person arms mesh, visible only to the local player */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	USkeletalMeshComponent* FirstPersonMesh;

	/** First person camera attached to the head socket */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FirstPersonCameraComponent;

	/** Grapple mechanic component */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Grapple", meta = (AllowPrivateAccess = "true"))
	UGrappleComponent* GrappleComponent;

protected:
	/** Jump Input Action */
	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* JumpAction;

	/** Move Input Action */
	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* MoveAction;

	/** Look Input Action */
	UPROPERTY(EditAnywhere, Category = "Input")
	class UInputAction* LookAction;

	/** Mouse Look Input Action */
	UPROPERTY(EditAnywhere, Category = "Input")
	class UInputAction* MouseLookAction;

	/** Grapple Input Action */
	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* GrappleAction;

	/** Reel Input Action */
	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* ReelAction;

public:
	AMGP_2526Character();

	/** Crosshair widget class to spawn for the local player */
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<class UUserWidget> CrosshairWidgetClass;

protected:
	void MoveInput(const FInputActionValue& Value);
	void LookInput(const FInputActionValue& Value);

	UFUNCTION(BlueprintCallable, Category = "Input")
	virtual void DoAim(float Yaw, float Pitch);

	UFUNCTION(BlueprintCallable, Category = "Input")
	virtual void DoMove(float Right, float Forward);

	UFUNCTION(BlueprintCallable, Category = "Input")
	virtual void DoJumpStart();

	UFUNCTION(BlueprintCallable, Category = "Input")
	virtual void DoJumpEnd();

	virtual void BeginPlay() override;

protected:
	virtual void SetupPlayerInputComponent(UInputComponent* InputComponent) override;

public:
	USkeletalMeshComponent* GetFirstPersonMesh() const { return FirstPersonMesh; }
	UCameraComponent* GetFirstPersonCameraComponent() const { return FirstPersonCameraComponent; }
};