// Joseph Cortez
// Capstone Project

// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.
#pragma once
#include "GameFramework/Character.h"
#include "GameFramework/MovementComponent.h"
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

UENUM(BlueprintType)
namespace CharacterScaleMode
{
	enum Type
	{
		MinScale				UMETA(DisplayName="Min Scale"),
		NormalScale				UMETA(DisplayName = "Normal Scale"),
		MaxScale				UMETA(DisplayName="Max Scale"),
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

static inline bool IsNormalScale(const CharacterScaleMode::Type ScaleMode)
{
	return (ScaleMode == CharacterScaleMode::NormalScale);
}

static inline bool IsMaxScale(const CharacterScaleMode::Type ScaleMode)
{
	return (ScaleMode == CharacterScaleMode::MaxScale);
}

static inline bool IsMinScale(const CharacterScaleMode::Type ScaleMode)
{
	return (ScaleMode == CharacterScaleMode::MinScale);
}

static inline FString GetNameForScaleMode(const CharacterScaleMode::Type ScaleMode)
{
	switch (ScaleMode)
	{
	case CharacterScaleMode::MinScale:
		return TEXT("Min Scale");
		break;
	case CharacterScaleMode::NormalScale:
		return TEXT("Normal Scale");
		break;
	case CharacterScaleMode::MaxScale:
		return TEXT("Max Scale");
		break;
	default:
		return TEXT("Unknown Scale Mode");
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

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:

	// Called every frame
	virtual void Tick(float DeltaSeconds) override;

	/** Base turn rate, in deg/sec. Other scaling may affect final turn rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseTurnRate;

	/** Base look up/down rate, in deg/sec. Other scaling may affect final rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseLookUpRate;

	/** Base Actor Scale */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Character)
	float NormalSize;

	/** Current camera mode */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Camera)
	TEnumAsByte<CharacterCameraMode::Type> CameraModeEnum;

	/** Current scale mode */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Character)
		TEnumAsByte<CharacterScaleMode::Type> CharacterScaleModeEnum;

	/** Controls the follow camera turn angle.Only affects Third Person Follow mode. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = SmoothFollowCamera)
	float CameraFollowTurnAngleExponent;

	/** Controls the follow camera turn speed. Only affects Third Person Follow mode */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = SmoothFollowCamera)
	float CameraFollowTurnRate;

	/** Controls the speed that the camera resets in Third Person Follow mode */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = SmoothFollowCamera)
	float CameraResetSpeed;

	/** Controls the 3rd person camera boom length */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Camera)
	float CurrentBoomLength3P;
	
	/** Autoreset after idle */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = SmoothFollowCameraReset)
	bool AutoResetSmoothFollowCameraWhenIdle;

	/** The delay to use if using Auto Reset */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = SmoothFollowCameraReset)
	float AutoResetDelaySeconds;

	/** The speed to use for Auto Resets */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = SmoothFollowCameraReset)
	float AutoResetSpeed;

	/** When true, player wants to grow */
	UPROPERTY(BlueprintReadOnly, Category = Character)
	uint32 bPressedGrow:1;

	/** When true, player wants to shrink */
	UPROPERTY(BlueprintReadOnly, Category = Character)
		uint32 bPressedShrink:1;

	/** The factor for growth*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Character)
		float GrowthFactor;
	/**The factor for shrinking*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Character)
		float ShrinkFactor;

	/**
	* Grow key Held Time.
	* This is the time that the player has held the grow key, in seconds.
	*/
	UPROPERTY(Transient, BlueprintReadOnly, VisibleInstanceOnly, Category = Character)
		float GrowKeyHoldTime;

	/**
	* Shrink key Held Time.
	* This is the time that the player has held the shrink key, in seconds.
	*/
	UPROPERTY(Transient, BlueprintReadOnly, VisibleInstanceOnly, Category = Character)
		float ShrinkKeyHoldTime;

	/**
	* The max time the grow key can be held.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Character, Meta = (ClampMin = 0.0, UIMin = 0.0))
		float GrowMaxHoldTime;

	/**
	* The max time the shrink key can be held.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Character, Meta = (ClampMin = 0.0, UIMin = 0.0))
		float ShrinkMaxHoldTime;
	
	/**
	* The max grow size.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Character)
		float GrowMaxSize;

	/**
	* The min shrink size.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Character)
		float ShrinkMinSize;

	// Tracks whether or not the character was already growing last frame.
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Transient, Category = Character)
		uint32 bWasGrowing : 1;

	// Tracks whether or not the character was already shrinking last frame.
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Transient, Category = Character)
		uint32 bWasShrinking : 1;

	/**
	* Make the character grow on the next update.
	*/
	UFUNCTION(BlueprintCallable, Category = Character)
		virtual void Grow();

	/**
	* Stop the character from growing on the next update.
	* Call this from an input event (such as a button 'up' event) to cease applying
	* grow. If this is not called, then grow will be applied
	* until GrowMaxSize is reached.
	*/
	UFUNCTION(BlueprintCallable, Category = Character)
		virtual void StopGrowing();

	/**
	* Make the character grow on the next update.
	*
	UFUNCTION(BlueprintCallable, Category = Character)
		virtual void IncrementalGrow();/

	/**
	* Stop the character from growing on the next update.
	* Call this from an input event (such as a button 'up' event) to cease applying
	* grow. If this is not called, then grow will be applied
	* until GrowMaxSize is reached.
	*/
	UFUNCTION(BlueprintCallable, Category = Character)
		virtual void StopIncrementalGrowing();

	/**
	* Make the character shrink on the next update.
	*/
	UFUNCTION(BlueprintCallable, Category = Character)
		virtual void Shrink();

	/**
	* Perform grow. Called by Character when a grow has been detected because Character->bPressedGrow was true. Checks CanGrow().
	* Note that you should usually trigger a grow through Character::Grow() instead.
	* @return	True if the grow was triggered successfully.
	*/
	virtual bool DoGrow();

	/**
	* Perform shrink. Called by Character when a shrink has been detected because Character->bPressedShrink was true. Checks CanShrink().
	* Note that you should usually trigger a shrink through Character::Shrink() instead.
	* @return	True if the shrink was triggered successfully.
	*/
	virtual bool DoShrink();

	/**
	* Stop the character from shrinking on the next update.
	* Call this from an input event (such as a button 'up' event) to cease applying
	* shrink. If this is not called, then shrink will be applied
	* until ShrinkMaxSize is reached.
	*/
	UFUNCTION(BlueprintCallable, Category = Character)
		virtual void StopShrinking();

	/**
	* Make the character shrink on the next update.
	*
	UFUNCTION(BlueprintCallable, Category = Character)
		virtual void IncrementalShrink();/

	/**
	* Stop the character from shrinking on the next update.
	* Call this from an input event (such as a button 'up' event) to cease applying
	* shrink. If this is not called, then shrink will be applied
	* until ShrinkMaxSize is reached.
	*/
	UFUNCTION(BlueprintCallable, Category = Character)
		virtual void StopIncrementalShrinking();

	/**
	* Check if the character can grow in the current state.
	*
	* The default implementation may be overridden or extended by implementing the custom CanJump event in Blueprints.
	*
	* @Return Whether the character can jump in the current state.
	*/
	UFUNCTION(BlueprintCallable, Category = Character)
		bool CanGrow() const;

protected:
	/**
	* Customizable event to check if the character can grow in the current state.
	* Default implementation returns true if the character is on the ground and not crouching,
	* has a valid CharacterMovementComponent and CanEverGrow() returns true.f
	* As well as returning true when on the ground, it also returns true when GetMaxGrowTime is more
	* than zero and IsGrowing returns true.
	*
	*
	* @Return Whether the character can grow in the current state.
	*/

	UFUNCTION(BlueprintNativeEvent, Category = Character, meta = (DisplayName = "CanGrow"))
		bool CanGrowInternal() const;
	virtual bool CanGrowInternal_Implementation() const;

	void ResetGrowState();

protected:
	/**
	* Customizable event to check if the character can Shrink in the current state.
	* Default implementation returns true if the character is on the ground and not crouching,
	* has a valid CharacterMovementComponent and CanEverShrink() returns true.
	* As well as returning true when on the ground, it also returns true when GetMaxShrinkTime is more
	* than zero and IsShrinking returns true.
	*
	*
	* @Return Whether the character can Shrink in the current state.
	*/

	UFUNCTION(BlueprintNativeEvent, Category = Character, meta = (DisplayName = "CanShrink"))
		bool CanShrinkInternal() const;
	virtual bool CanShrinkInternal_Implementation() const;

	void ResetShrinkState();

public:

	/**
	* Check if the character can shrink in the current state.
	*
	* The default implementation may be overridden or extended by implementing the custom CanJump event in Blueprints.
	*
	* @Return Whether the character can jump in the current state.
	*/
	UFUNCTION(BlueprintCallable, Category = Character)
		bool CanShrink() const;

	/** Trigger grow if grow button has been pressed. */
	virtual void CheckGrowInput(float DeltaTime);

	/** Reset input state after having checked input. */
	virtual void ClearGrowInput();

	/** Trigger shrink if shrink button has been pressed. */
	virtual void CheckShrinkInput(float DeltaTime);

	/** Reset input state after having checked input. */
	virtual void ClearShrinkInput();

	/** Event fired when the character has just started growing */
	UFUNCTION(BlueprintNativeEvent, Category = Character)
	void OnGrow();
	virtual void OnGrow_Implementation();

	/**
	* True if grow is actively providing a force, such as when the grow key is held and the time it has been held is less than GrowMaxHoldTime.
	*/
	UFUNCTION(BlueprintCallable, Category = Character)
	virtual bool IsGrowProvidingForce() const;

	/** Event fired when the character has just started shrinking */
	UFUNCTION(BlueprintNativeEvent, Category = Character)
	void OnShrink();
	virtual void OnShrink_Implementation();

	/**
	* True if shrink is actively providing a force, such as when the shrink key is held and the time it has been held is less than ShrinkMaxHoldTime.
	*/
	UFUNCTION(BlueprintCallable, Category = Character)
		virtual bool IsShrinkProvidingForce() const;

	/**
	* Get the maximum grow time for the character.
	*
	* @return Maximum grow time for the character
	*/
	virtual float GetGrowMaxHoldTime() const;

	/**
	* Get the maximum shrink time for the character.
	*
	* @return Maximum shrink time for the character
	*/
	virtual float GetShrinkMaxHoldTime() const;


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

protected:
	// Character Scale Mode

	/** Cyle to next Grow mode */
	void IncrementalGrow();

	/** Cycle to next Shrink mode */
	void IncrementalShrink();

	/** Sets the character scale mode to the specified value */
	void SetCharacterScaleMode(CharacterScaleMode::Type newCharacterScaleMode);

	/** Sets properties based on scale mode value */
	void UpdateForCharacterScaleMode();

	/**Is Character Scale in normal mode */
	bool IsNormalScaleMode();

	/**Is Character Scale in min mode */
	bool IsMinScaleMode();

	/**Is Character Scale in max mode */
	bool IsMaxScaleMode();



public:
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }
};

