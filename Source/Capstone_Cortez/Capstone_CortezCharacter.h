// Joseph Cortez
// Capstone Project

// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.
#pragma once
#include "GameFramework/Character.h"
#include "Capstone_CortezCharacter.generated.h"


UENUM(BlueprintType)
namespace CharacterCameraMode
{
	enum Type
	{
		ThirdPersonDefault		UMETA(DisplayName="Third Person"),
		FirstPerson				UMETA(DisplayName="First Person"),

		Max						UMETA(Hidden),
	};
}

static inline bool IsFirstPerson(const CharacterCameraMode::Type CameraMode)
{
	return (CameraMode == CharacterCameraMode::FirstPerson);
}

static inline bool IsThirdPerson(const CharacterCameraMode::Type CameraMode)
{
	return !IsFirstPerson(CameraMode);
}

static inline FString GetNameForCameraMode(const CharacterCameraMode::Type CameraMode)
{
	switch (CameraMode)
	{
	case CharacterCameraMode::ThirdPersonDefault:
		return TEXT("Third Person");
		break;
	case CharacterCameraMode::FirstPerson:
		return TEXT("First Person");
		break;
	default:
		return TEXT("Unknown Camera Mode");
	}
}

UCLASS(config=Game)
class ACapstone_CortezCharacter : public ACharacter
{
	GENERATED_BODY()

	/** Camera boom positioning the camera behind the character */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FollowCamera;
	

public:
	ACapstone_CortezCharacter();

	/** Base turn rate, in deg/sec. Other scaling may affect final turn rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseTurnRate;

	/** Base look up/down rate, in deg/sec. Other scaling may affect final rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseLookUpRate;



	/** Current camera mode */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Camera)
	TEnumAsByte<CharacterCameraMode::Type> CameraModeEnum;

	/** Controls the follow camera turn angle.Only affects Third Person Follow mode. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = SmoothFollowCamera)
	float CameraFollowTurnAngleExponent;

	/** Controls the follow camera turn speed. Only affects Third Person Follow mode */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = SmoothFollowCamera)
		float CameraFollowTurnRate;

	/** Controls the speed that the camera resets in Third Person Follow mode */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = SmoothFollowCamera)
		float CameraResetSpeed;
	
	/** Autoreset after idle */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = SmoothFollowCameraReset)
	bool AutoResetSmoothFollowCameraWhenIdle;

	/** The delay to use if using Auto Reset */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = SmoothFollowCameraReset)
	float AutoResetDelaySeconds;

	/** The speed to use for Auto Resets */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = SmoothFollowCameraReset)
	float AutoResetSpeed;

protected:
	/** Keeps track of whether the camera is currently being reset */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = CameraInternal)
	bool IsResetting;

	/** Keeps track of when the last movement was */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = CameraInternal)
	float LastMovementTime;

	/** Keeps track of whether a reset is automatic or manual */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = CameraInternal)
	bool IsAutoReset;

protected:
	/** Resets HMD orientation in VR. */
	void OnResetVR();

	/** Called for forwards/backward input */
	void MoveForward(float Value);

	/** Called for side to side input */
	void MoveRight(float Value);

	/** 
	 * Called via input to turn at a given rate. 
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void TurnAtRate(float Rate);

	/**
	 * Called via input to turn look up/down at a given rate. 
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void LookUpAtRate(float Rate);

	/** Handler for when a touch input begins. */
	void TouchStarted(ETouchIndex::Type FingerIndex, FVector Location);

	/** Handler for when a touch input stops. */
	void TouchStopped(ETouchIndex::Type FingerIndex, FVector Location);

protected:
	// APawn interface
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	// End of APawn interface

	//virtual void Tick(float DeltaSeconds);

protected:
	// Camera Mode

	/** Cycle to the next camera mode */
	void CycleCamera();

	/** Sets the camera mode to the specified value */
	void SetCameraMode(CharacterCameraMode::Type newCameraMode);

	/** Sets properties based on camera mode value */
	void UpdateForCameraMode();

	/** Is Camera in first person mode */
	bool IsFirstPersonMode();

	/** Is Camera in third person mode */
	bool IsThirdPersonMode();

public:
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }
};

