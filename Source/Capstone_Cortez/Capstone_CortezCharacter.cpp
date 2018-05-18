// Joseph Cortez
// Capstone project

// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#include "Capstone_Cortez.h"
#include "Kismet/HeadMountedDisplayFunctionLibrary.h"
#include "Capstone_CortezCharacter.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/SkeletalMeshSocket.h"


//////////////////////////////////////////////////////////////////////////
// ACapstone_CortezCharacter

ACapstone_CortezCharacter::ACapstone_CortezCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;

	// set our turn rates for input
	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;

	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;


	FName headSocket = "headSocket";
	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f); // ...at this rotation rate
	GetCharacterMovement()->JumpZVelocity = 600.f;
	GetCharacterMovement()->AirControl = 0.2f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 300.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow cameraf
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	CameraFollowTurnAngleExponent = .5f;
	CameraFollowTurnRate = .6f;
	CameraResetSpeed = 2.f;
	
	IsResetting = false;
	AutoResetSmoothFollowCameraWhenIdle = true;
	AutoResetDelaySeconds = 5.f;

	IsAutoReset = false;
	AutoResetSpeed = .15f;

	GrowthFactor = 0.5f;
	GrowMaxSize = 10.0f;
	ShrinkMinSize = 0.25f;

	GrowMaxHoldTime = 10;
	ShrinkMaxHoldTime = 10;
	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named MyCharacter (to avoid direct content references in C++)
}

// Called when the game starts or when spawned
void ACapstone_CortezCharacter::BeginPlay()
{
	Super::BeginPlay();

}

// Called every frame
void ACapstone_CortezCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	FVector currentScale = GetActorScale3D();						// Get the current scale of the character
	currentScale = currentScale.GetClampedToSize(ShrinkMinSize, GrowMaxSize);		// Add constraints to scale

	float currentBoomLength = CameraBoom->TargetArmLength;			// Get the current camera boom length
	
	// Ability to have seamless growth
	if (bPressedGrow && IsThirdPersonMode() && currentScale.Size() < GrowMaxSize - 0.01f)
	{
		GrowthFactor += DeltaTime / 2.0f;
		GrowthFactor = FMath::Clamp<float>(GrowthFactor, 0.0f, 2.0f);
		SetActorScale3D(currentScale + (GrowthFactor*.01f));
		CameraBoom->TargetArmLength = (currentBoomLength + (GrowthFactor*2.0f));
	}
	else if(bPressedGrow && IsFirstPersonMode())
	{
		GrowthFactor += DeltaTime / 2.0f;
		GrowthFactor = FMath::Clamp<float>(GrowthFactor, 0.0f, 2.0f);
		SetActorScale3D(currentScale + (GrowthFactor*.01f));
	}

	// Ability to have seamless shrink
	if (bPressedShrink && IsThirdPersonMode() && currentScale.Size() > ShrinkMinSize + 0.01f)
	{
		ShrinkFactor += DeltaTime / 2.0f;
		ShrinkFactor = FMath::Clamp<float>(ShrinkFactor, 0.0f, 2.0f);
		SetActorScale3D(currentScale - (ShrinkFactor*.01f));
		CameraBoom->TargetArmLength = (currentBoomLength - (ShrinkFactor*2.0f));
	}
	else if (bPressedShrink && IsFirstPersonMode())
	{
		ShrinkFactor += DeltaTime / 2.0f;
		ShrinkFactor = FMath::Clamp<float>(ShrinkFactor, 0.0f, 2.0f);
		SetActorScale3D(currentScale - (ShrinkFactor*.01f));
	}
}

//////////////////////////////////////////////////////////////////////////
// Input

void ACapstone_CortezCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// Set up gameplay key bindings
	check(PlayerInputComponent);
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);
	
	// Grow and Shrink
	PlayerInputComponent->BindAction("Grow", IE_Pressed, this, &ACapstone_CortezCharacter::Grow);
	PlayerInputComponent->BindAction("Grow", IE_Released, this, &ACapstone_CortezCharacter::StopGrowing);
	PlayerInputComponent->BindAction("Shrink", IE_Pressed, this, &ACapstone_CortezCharacter::Shrink);
	PlayerInputComponent->BindAction("Shrink", IE_Released, this, &ACapstone_CortezCharacter::StopShrinking);

	PlayerInputComponent->BindAxis("MoveForward", this, &ACapstone_CortezCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &ACapstone_CortezCharacter::MoveRight);

	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick
	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("TurnRate", this, &ACapstone_CortezCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("LookUpRate", this, &ACapstone_CortezCharacter::LookUpAtRate);

	// handle touch devices
	PlayerInputComponent->BindTouch(IE_Pressed, this, &ACapstone_CortezCharacter::TouchStarted);
	PlayerInputComponent->BindTouch(IE_Released, this, &ACapstone_CortezCharacter::TouchStopped);

	// VR headset functionality
	PlayerInputComponent->BindAction("ResetVR", IE_Pressed, this, &ACapstone_CortezCharacter::OnResetVR);

	// Camera inputs
	PlayerInputComponent->BindAction("ToggleCameraMode", IE_Pressed, this, &ACapstone_CortezCharacter::CycleCamera);
}


void ACapstone_CortezCharacter::OnResetVR()
{
	UHeadMountedDisplayFunctionLibrary::ResetOrientationAndPosition();
}

void ACapstone_CortezCharacter::TouchStarted(ETouchIndex::Type FingerIndex, FVector Location)
{
		Jump();
		Grow();
		Shrink();
}

void ACapstone_CortezCharacter::TouchStopped(ETouchIndex::Type FingerIndex, FVector Location)
{
		StopJumping();
		StopGrowing();
		StopShrinking();
}

void ACapstone_CortezCharacter::TurnAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void ACapstone_CortezCharacter::LookUpAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

void ACapstone_CortezCharacter::MoveForward(float Value)
{
	if (Value == 0.f) return;

	if ((Controller != NULL) && (IsThirdPersonMode()))
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		AddMovementInput(Direction, Value);
	}
	else
	{
		AddMovementInput(GetActorForwardVector(), Value);
	}
}

void ACapstone_CortezCharacter::MoveRight(float Value)
{
	if (Value == 0.f) return;

	if ((Controller != NULL) && (IsThirdPersonMode()))
	{
		// find out which way is right
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get right vector
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		AddMovementInput(Direction, Value);
	}
	else
	{
		AddMovementInput(GetActorRightVector(), Value);
	}
}

//////////////////////////////////////////////////////////////////////////
// Grow Functions

bool ACapstone_CortezCharacter::CanGrow() const
{
	return CanGrowInternal();
}

bool ACapstone_CortezCharacter::CanGrowInternal_Implementation() const
{
	// Ensure the character isn't currently crouched.
	bool bCanGrow = true;									//!bIsCrouched;

	/* Ensure that the CharacterMovement state is valid
	bCanGrow &=  CharacterMovement &&
		CharacterMovement->IsJumpAllowed() &&
		!CharacterMovement->bWantsToCrouch &&
		// Can only grow from the ground, or if already falling.
		(CharacterMovement->IsMovingOnGround() || CharacterMovement->IsFalling());*/

	if (bCanGrow)
	{
		// Ensure GrowHoldTime is valid.
		if (GetGrowMaxHoldTime() <= 0.0f)
		{
			bCanGrow = false;
		}
	}

	return bCanGrow;
}

void ACapstone_CortezCharacter::OnGrow_Implementation()
{
}

bool ACapstone_CortezCharacter::IsGrowProvidingForce() const
{
	return (bPressedGrow && GrowKeyHoldTime > 0.0f && GrowKeyHoldTime < GrowMaxHoldTime);
}

void ACapstone_CortezCharacter::Grow()
{
	bPressedGrow = true;
	GrowKeyHoldTime = 0.0f;
	//DoGrow();
	//CheckGrowInput(GrowKeyHoldTime);
}

void ACapstone_CortezCharacter::StopGrowing()
{
	bPressedGrow = false;
	GrowthFactor = 0.5f;
	ResetGrowState();
}

bool ACapstone_CortezCharacter::DoGrow()
{
	if (bPressedGrow && CanGrow())
	{
		FVector currentScale = GetActorScale3D();
		if ((int)GrowKeyHoldTime % 2 == 0)
		{
			SetActorScale3D(currentScale * 2);
		}
			
		return true;
	}

	return false;
}

void ACapstone_CortezCharacter::CheckGrowInput(float DeltaTime)
{
	if (Controller != NULL)
	{
		if (bPressedGrow)
		{
			// Increment our timer first so calls to IsJumpProvidingForce() will return true
			GrowKeyHoldTime += DeltaTime;


			const bool bDidGrow = CanGrow() && DoGrow();
			if (!bWasGrowing && bDidGrow)
			{
				DoGrow();
			}

			bWasGrowing = bDidGrow;
		}

		else if (bWasGrowing)
		{
			ResetGrowState();
		}
	}
}

void ACapstone_CortezCharacter::ResetGrowState()
{
	bWasGrowing = false;
	GrowKeyHoldTime = 0.0f;
}

void ACapstone_CortezCharacter::ClearGrowInput()
{
	// Don't disable bPressedGrow right away if it's still held
	if (bPressedGrow && (GrowKeyHoldTime >= GetGrowMaxHoldTime()))
	{
		bPressedGrow = false;
	}
}

float ACapstone_CortezCharacter::GetGrowMaxHoldTime() const
{
	return GrowMaxHoldTime;
}

//////////////////////////////////////////////////////////////////////////
// Shrink Functions

bool ACapstone_CortezCharacter::CanShrink() const
{
	return CanShrinkInternal();
}

bool ACapstone_CortezCharacter::CanShrinkInternal_Implementation() const
{
	// Ensure the character isn't currently crouched.
	bool bCanShrink = true;										//!bIsCrouched;

	/* Ensure that the CharacterMovement state is valid
	bCanShrink &=  CharacterMovement &&
	CharacterMovement->IsJumpAllowed() &&
	!CharacterMovement->bWantsToCrouch &&
	// Can only Shrink from the ground, or if already falling.
	(CharacterMovement->IsMovingOnGround() || CharacterMovement->IsFalling());*/

	if (bCanShrink)
	{
		// Ensure ShrinkHoldTime is valid.
		if (GetShrinkMaxHoldTime() <= 0.0f)
		{
			bCanShrink = false;
		}
	}

	return bCanShrink;
}

void ACapstone_CortezCharacter::OnShrink_Implementation()
{
}

bool ACapstone_CortezCharacter::IsShrinkProvidingForce() const
{
	return (bPressedShrink && ShrinkKeyHoldTime > 0.0f && ShrinkKeyHoldTime < ShrinkMaxHoldTime);
}

void ACapstone_CortezCharacter::Shrink()
{
	bPressedShrink = true;
	ShrinkKeyHoldTime = 0.0f;
	//DoShrink();
}

void ACapstone_CortezCharacter::StopShrinking()
{
	bPressedShrink = false;
	ShrinkFactor = 0.5f;
	ResetShrinkState();
}

bool ACapstone_CortezCharacter::DoShrink()
{
	if (bPressedShrink && CanShrink())
	{
		FVector currentScale = GetActorScale3D();
		if ((int)ShrinkKeyHoldTime % 2 == 0)
			SetActorScale3D(currentScale / 2);
		return true;
	}

	return false;
}

void ACapstone_CortezCharacter::CheckShrinkInput(float DeltaTime)
{
	if (Controller != NULL)
	{
		if (bPressedShrink)
		{
			// Increment our timer first so calls to IsShrinkProvidingForce() will return true
			ShrinkKeyHoldTime += DeltaTime;


			const bool bDidShrink = CanShrink() && DoShrink();
			if (!bWasShrinking && bDidShrink)
			{
				OnShrink();
			}

			bWasShrinking = bDidShrink;
		}

		else if (bWasShrinking)
		{
			ResetShrinkState();
		}
	}
}

void ACapstone_CortezCharacter::ResetShrinkState()
{
	bWasShrinking = false;
	ShrinkKeyHoldTime = 0.0f;
}

void ACapstone_CortezCharacter::ClearShrinkInput()
{
	// Don't disable bPressedShrink right away if it's still held
	if (bPressedShrink && (ShrinkKeyHoldTime >= GetShrinkMaxHoldTime()))
	{
		bPressedShrink = false;
	}
}

float ACapstone_CortezCharacter::GetShrinkMaxHoldTime() const
{
	return ShrinkMaxHoldTime;
}


//////////////////////////////////////////////////////////////////////////
// Camera Mode

// Cycles the camera between 1st and 3rd person
void ACapstone_CortezCharacter::CycleCamera()
{

	int newCameraMode = (int)CameraModeEnum + 1;

	if (newCameraMode == 2) newCameraMode = CharacterCameraMode::ThirdPersonDefault;
	SetCameraMode((CharacterCameraMode::Type) newCameraMode);
}

// Set the camera mode
void ACapstone_CortezCharacter::SetCameraMode(CharacterCameraMode::Type newCameraMode)
{
	CameraModeEnum = newCameraMode;
	UpdateForCameraMode();

}


// Determines camera values for 3rd or 1st person
void ACapstone_CortezCharacter::UpdateForCameraMode()
{
	
	// Changes visibility of first and third person meshes
	switch (CameraModeEnum)
	{
	case CharacterCameraMode::ThirdPersonDefault:
		IsResetting = false;
		CameraBoom->AttachTo(GetMesh(), "headSocket");
		CameraBoom->TargetArmLength = 300.f;
		bUseControllerRotationPitch = false;
		bUseControllerRotationYaw = false;
		bUseControllerRotationRoll = false;
		break;
	case CharacterCameraMode::FirstPerson:
		CameraBoom->AttachTo(GetMesh(), "headSocket");
		CameraBoom->TargetArmLength = 0.f;
		IsResetting = false;
		bUseControllerRotationPitch = false;
		bUseControllerRotationYaw = true;
		bUseControllerRotationRoll = false;
		break;

	default:
		break;
	}
}

bool ACapstone_CortezCharacter::IsFirstPersonMode()
{
	return IsFirstPerson(CameraModeEnum);
}

bool ACapstone_CortezCharacter::IsThirdPersonMode()
{
	return IsThirdPerson(CameraModeEnum);
}
