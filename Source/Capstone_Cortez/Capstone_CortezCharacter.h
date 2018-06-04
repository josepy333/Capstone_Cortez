// Joseph Cortez
// Capstone Project

// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.
#pragma once
#include "GameFramework/Character.h"
#include "GameFramework/MovementComponent.h"
#include "Interaction.h"
#include "Pickup.h"
#include "Capstone_CortezCharacter.generated.h"




//////////////////////////////////////////////////////////////////////////////
//////////////// CHARACTER ENUMS

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

UENUM(BlueprintType)
namespace CharacterMovementMode
{
	enum Type
	{
		WalkDefault				UMETA(DisplayName="Walk"),
		Fly						UMETA(DisplayName="Fly"),
	};
}

////////////////////////////////////////////////////////////////////////////////

/**Determine if character is in first person mode **/
static inline bool IsFirstPerson(const CharacterCameraMode::Type CameraMode)
{
	return (CameraMode == CharacterCameraMode::FirstPerson);
}

/**Determine if character is in third person mode **/
static inline bool IsThirdPerson(const CharacterCameraMode::Type CameraMode)
{
	return !IsFirstPerson(CameraMode);
}

/** Get name for camera mode **/
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

/**Determine if character is in normal scale mode **/
static inline bool IsNormalScale(const CharacterScaleMode::Type ScaleMode)
{
	return (ScaleMode == CharacterScaleMode::NormalScale);
}

/**Determine if character is in maximum scale mode **/
static inline bool IsMaxScale(const CharacterScaleMode::Type ScaleMode)
{
	return (ScaleMode == CharacterScaleMode::MaxScale);
}

/**Determine if character is in minimum scale mode **/
static inline bool IsMinScale(const CharacterScaleMode::Type ScaleMode)
{
	return (ScaleMode == CharacterScaleMode::MinScale);
}

/** Get name for scale mode **/
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

/**Determine if character is in walking mode **/
static inline bool IsWalking(const CharacterMovementMode::Type MovementMode)
{
	return (MovementMode == CharacterMovementMode::WalkDefault);
}

/**Determine if character is in fly mode **/
static inline bool IsFlying(const CharacterMovementMode::Type MovementMode)
{
	return (MovementMode == CharacterMovementMode::Fly);
}

/** Get name for movmement mode **/
static inline FString GetNameForMovementMode(const CharacterMovementMode::Type MovementMode)
{
	switch (MovementMode)
	{
	case CharacterMovementMode::WalkDefault:
		return TEXT("Walk");
		break;
	case CharacterMovementMode::Fly:
		return TEXT("Fly");
		break;
	default:
		return TEXT("Unknown Movement Mode");
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

	/** Player controller */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class APlayerController* PlayerController;

	

public:
	ACapstone_CortezCharacter();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:

	// Called every frame
	virtual void Tick(float DeltaSeconds) override;

	// Minor Incremental value
	float minorIncrement;

	// Major Incremental value
	float superMinorIncrement;

	// Double incremental value
	float doubleIncrement;

	// Triple incremental value
	float tripleIncrement;

	// Quadrupal Incremental value
	float quadIncrement;

	/** Base turn rate, in deg/sec. Other scaling may affect final turn rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseTurnRate;

	/** Base look up/down rate, in deg/sec. Other scaling may affect final rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseLookUpRate;

	/** Base Actor Size */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Character)
	float NormalSize;

	/** Normal Actor Scale */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Character)
	FVector NormalScale;

	/** Max Actor Scale */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Character)
	FVector MaxScale;

	/** Min Actor Scale */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Character)
	FVector MinScale;

	/** Current camera mode */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Camera)
	TEnumAsByte<CharacterCameraMode::Type> CameraModeEnum;

	/** Current scale mode */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Character)
	TEnumAsByte<CharacterScaleMode::Type> CharacterScaleModeEnum;

	/** Current movment mode */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Character)
	TEnumAsByte<CharacterMovementMode::Type> MovementModeEnum;

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
	* Stop the character from shrinking on the next update.
	* Call this from an input event (such as a button 'up' event) to cease applying
	* shrink. If this is not called, then shrink will be applied
	* until ShrinkMaxSize is reached.
	*/
	UFUNCTION(BlueprintCallable, Category = Character)
		virtual void StopShrinking();

	/**
	* Stop the character from shrinking on the next update.
	* Call this from an input event (such as a button 'up' event) to cease applying
	* shrink. If this is not called, then shrink will be applied
	* until ShrinkMaxSize is reached.
	*/
	UFUNCTION(BlueprintCallable, Category = Character)
		virtual void StopIncrementalShrinking();


protected:

	void ResetGrowState();

protected:

	void ResetShrinkState();

public:


	/** Reset input state after having checked input. */
	virtual void ClearGrowInput();


	/** Reset input state after having checked input. */
	virtual void ClearShrinkInput();

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

	/** Called for up and down movement */
	void JetPack(float Value);

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

public:
	/** Is Camera in first person mode */
	bool IsFirstPersonMode();

	/** Is Camera in third person mode */
	bool IsThirdPersonMode();

	bool IsAlreadyMinScaleMode = false;
	bool IsAlreadyMaxScaleMode = false;
	bool IsAlreadyNormalScaleMode = true;

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

protected:
	// Movement Mode

	/** Cycle to the next Movement mode */
	void CycleMovement();

	/** Sets the Movement mode to the specified value */
	void SetMovementMode(CharacterMovementMode::Type newMovementMode);

	/** Is Movement walking */ 
	bool IsWalkMode();

	/** Is Movement flying */
	bool IsFlyMode();

public:
	/** Sets properties based on Movement mode value */
	UFUNCTION(BlueprintCallable, Category = "Custom")
	void UpdateForMovementMode();

	/** Reinitiallize movement **/
	UFUNCTION(BlueprintCallable, Category = "Custom")
		void ReinitializeMovement();

private:
	/** Toggle the inventory **/
	void ToggleInventory();

	/**Interacts with interaction item **/
	void Interact();

	/** Check if there is an interaction item within near distance in front of player **/
	void CheckForInteractionItem();

	/** The character's reach **/
	float CharacterReach;

	/** The interaction item the character is looking at **/
	AInteraction* CurrentInteraction;

	/** Player inventory stored in an array **/
	UPROPERTY(EditAnywhere)
		TArray<APickup*> Inventory;

public:
	// Inventory HUD

	/** The Text to pop up on screen **/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HUD")
	FString ScreenText;

	/** On true, adds item to inventory **/
	UFUNCTION(BlueprintPure, Category = "Inventory Functions")
	bool AddItemToInventory(APickup* Item);

	/** Get thumbnail for inventory slot **/
	UFUNCTION(BlueprintPure, Category = "Inventory Functions")
	UTexture2D* GetThumbnailAtInventorySlot(int32 Slot);

	/** Get item name for inventory slot **/
	UFUNCTION(BlueprintPure, Category = "Inventory Functions")
	FString GetItemNameAtInventorySlot(int32 Slot);

	/** Use the item at a given inventory slot **/
	UFUNCTION(BlueprintCallable, Category = "Inventory Functions")
	void UseItemAtInventorySlot(int32 Slot);

public:
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }
};





