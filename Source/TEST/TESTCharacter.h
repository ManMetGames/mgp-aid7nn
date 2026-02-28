// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"
#include "TESTCharacter.generated.h"

class UInputComponent;
class USkeletalMeshComponent;
class UCameraComponent;
class UInputAction;
struct FInputActionValue;

DECLARE_LOG_CATEGORY_EXTERN(LogTemplateCharacter, Log, All);

/**
 *  A basic first person character
 */
UCLASS(abstract)
class ATESTCharacter : public ACharacter
{
	GENERATED_BODY()

	/** Pawn mesh: first person view (arms; seen only by self) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	USkeletalMeshComponent* FirstPersonMesh;

	/** First person camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FirstPersonCameraComponent;

protected:

	/** Jump Input Action */
	UPROPERTY(EditAnywhere, Category ="Input")
	UInputAction* JumpAction;

	/** Move Input Action */
	UPROPERTY(EditAnywhere, Category ="Input")
	UInputAction* MoveAction;

	/** Look Input Action */
	UPROPERTY(EditAnywhere, Category ="Input")
	class UInputAction* LookAction;

	/** Mouse Look Input Action */
	UPROPERTY(EditAnywhere, Category ="Input")
	class UInputAction* MouseLookAction;
	
public:
	ATESTCharacter();

protected:

	/** Called from Input Actions for movement input */
	void MoveInput(const FInputActionValue& Value);

	/** Called from Input Actions for looking input */
	void LookInput(const FInputActionValue& Value);

	/** Handles aim inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoAim(float Yaw, float Pitch);

	/** Handles move inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoMove(float Right, float Forward);

	/** Handles jump start inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoJumpStart();

	/** Handles jump end inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoJumpEnd();

protected:

	/** Set up input action bindings */
	virtual void SetupPlayerInputComponent(UInputComponent* InputComponent) override;
	

public:

	/** Returns the first person mesh **/
	USkeletalMeshComponent* GetFirstPersonMesh() const { return FirstPersonMesh; }

	/** Returns first person camera component **/
	UCameraComponent* GetFirstPersonCameraComponent() const { return FirstPersonCameraComponent; }

public:

	UPROPERTY(BlueprintReadOnly, Replicated)   //replicated, if called on server and changed, that change will be sent to all clients
		bool bIsGrappling = false;

	UPROPERTY(BlueprintReadOnly, Replicated)
	FVector CurrentGrapplePoint = FVector(0, 0, 0);  //position in world in which player is grappling

	UPROPERTY(BlueprintReadOnly, Replicated)
	float CurrentGrappleDistance = 0.0f;   //max distance the grapple can go at current moment (pulls further if extends max grapple distance)

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Catagory = "Grapple")   //allows for quick adjustments
		float MaxGrappleDistance = 4000.0f;   //max start grapple distance

	virtual void GetLifetimeReplicatedProps
	(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void TrySetGrappleLocal();    //do trace and try set grapple point

	void SetGrapplePointLocal(FVector GrapplePoint);  //set the grapple point locally

	UFUNCTION(Server, Reliable)  //called and set grapplepoint on server
	void SetGrapplePointServer(FVector GreapplePoint);

	bool IsGrapplePointValid(FVector GrapplePoint);   //check if point is valid (prevention against cheaters) true/false

	void RegisterNewGrapplePoint(FVector GrapplePoint);   

	void ApplyGrapple(); //applies forces to player when they grapple

	void StopGrappleLocal();

	UFUNCTION(Server, Reliable)
	void StopGrappleRemote();

};

