// Joseph Cortez
// Capstone project

// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#include "Capstone_Cortez.h"
#include "Kismet/HeadMountedDisplayFunctionLibrary.h"
#include "Capstone_CortezCharacter.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/SkeletalMeshSocket.h"
#include "EngineGlobals.h"
#include "Engine.h"


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

	// Default 3rd person boom length
	CurrentBoomLength3P = 300.0f;

	FName headSocket = "headSocket";
	minorIncrement = 0.01f;
	majorIncrement = 0.001f;
	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f); // ...at this rotation rate
	GetCharacterMovement()->JumpZVelocity = 500.0f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = CurrentBoomLength3P; // The camera follows at this distance behind the character	
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

	NormalSize = 1.0f;
	GrowthFactor = 0.5f;
	GrowMaxSize = 10.0f;
	ShrinkMinSize = 0.25f;

	GrowMaxHoldTime = 10;
	ShrinkMaxHoldTime = 10;

	NormalScale = FVector(1.0f, 1.0f, 1.0f);
	MaxScale = FVector(5.773502f, 5.773502f, 5.773502f);
	MinScale = FVector(0.144338f, 0.144338f, 0.144338f);

	SetCharacterScaleMode((CharacterScaleMode::Type) CharacterScaleMode::NormalScale);
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
	
	/*******************         VARIABLE TRACKER FOR ACTOR SIZE ****************************************************************/
	if (GEngine)
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, FString::Printf(TEXT("Current Scale %f Current Boom Length %f"), currentScale.GetMax(), CurrentBoomLength3P));

	/*******************************************************************************************************************************/
	
	// Ability to have seamless growth
	if (bPressedGrow && IsThirdPersonMode() && currentScale.Size() < GrowMaxSize -.001)
	{
		GrowthFactor = 1.0f;

		SetActorScale3D(currentScale + (GrowthFactor*minorIncrement));
		CurrentBoomLength3P = CurrentBoomLength3P + (GrowthFactor*2.0f);			//Needed to adjust 3rd person camera boom
		CameraBoom->TargetArmLength = CurrentBoomLength3P;

		if (currentScale.GetMax() > MaxScale.GetMax() - minorIncrement)
		{
			SetCharacterScaleMode((CharacterScaleMode::Type) CharacterScaleMode::MaxScale);
		}
		
		// Increase movement speed when larger than normal
		if (currentScale.GetMax() > NormalSize + minorIncrement)
		{
			GetCharacterMovement()->UCharacterMovementComponent::MaxWalkSpeed = GetCharacterMovement()->UCharacterMovementComponent::MaxWalkSpeed + (GrowthFactor*4.0f);
			GetCharacterMovement()->UCharacterMovementComponent::Mass = GetCharacterMovement()->UCharacterMovementComponent::Mass + (GrowthFactor*2.0f);
			GetCharacterMovement()->UCharacterMovementComponent::JumpZVelocity = GetCharacterMovement()->UCharacterMovementComponent::JumpZVelocity + (GrowthFactor*4.0f);
			GetCharacterMovement()->UCharacterMovementComponent::JumpOffJumpZFactor = GetCharacterMovement()->UCharacterMovementComponent::JumpOffJumpZFactor + (GrowthFactor*4.0f);
			GetCharacterMovement()->UCharacterMovementComponent::MaxStepHeight = GetCharacterMovement()->UCharacterMovementComponent::MaxStepHeight + (GrowthFactor*2.0f);
			GetCharacterMovement()->UCharacterMovementComponent::GravityScale = GetCharacterMovement()->UCharacterMovementComponent::GravityScale + (GrowthFactor*minorIncrement);

		}
		// Reset Movement Stats when within normal size range
		else if (currentScale.GetMax() < NormalSize + minorIncrement && currentScale.GetMax() > NormalSize)
		{
			GetCharacterMovement()->UCharacterMovementComponent::MaxWalkSpeed = 600.0f;
			GetCharacterMovement()->UCharacterMovementComponent::Mass = 1.0f;
			GetCharacterMovement()->UCharacterMovementComponent::JumpZVelocity = 500.0f;
			GetCharacterMovement()->UCharacterMovementComponent::JumpOffJumpZFactor = 0.5f;
			GetCharacterMovement()->UCharacterMovementComponent::MaxStepHeight = 45.0f;
			GetCharacterMovement()->UCharacterMovementComponent::GravityScale = 1.0f;

			SetCharacterScaleMode((CharacterScaleMode::Type) CharacterScaleMode::NormalScale);
		}
	}
	else if(bPressedGrow && IsFirstPersonMode() && currentScale.Size() < GrowMaxSize -minorIncrement)
	{
		GrowthFactor = 1.0f;
		SetActorScale3D(currentScale + (GrowthFactor*minorIncrement));
		CurrentBoomLength3P = CurrentBoomLength3P + (GrowthFactor*2.0f);			//Needed to adjust 3rd person camera boom

		if (currentScale.GetMax() > MaxScale.GetMax() - minorIncrement)
		{
			SetCharacterScaleMode((CharacterScaleMode::Type) CharacterScaleMode::MaxScale);
		}
		
		// Increase movement speed when larger than normal
		if (currentScale.GetMax() > NormalSize + minorIncrement)
		{
			GetCharacterMovement()->UCharacterMovementComponent::MaxWalkSpeed = GetCharacterMovement()->UCharacterMovementComponent::MaxWalkSpeed + (GrowthFactor*4.0f);
			GetCharacterMovement()->UCharacterMovementComponent::Mass = GetCharacterMovement()->UCharacterMovementComponent::Mass + (GrowthFactor *minorIncrement);
			GetCharacterMovement()->UCharacterMovementComponent::JumpZVelocity = GetCharacterMovement()->UCharacterMovementComponent::JumpZVelocity + (GrowthFactor*4.0f);
			GetCharacterMovement()->UCharacterMovementComponent::JumpOffJumpZFactor = GetCharacterMovement()->UCharacterMovementComponent::JumpOffJumpZFactor + (GrowthFactor*4.0f);
			GetCharacterMovement()->UCharacterMovementComponent::MaxStepHeight = GetCharacterMovement()->UCharacterMovementComponent::MaxStepHeight + (GrowthFactor*2.0f);
			GetCharacterMovement()->UCharacterMovementComponent::GravityScale = GetCharacterMovement()->UCharacterMovementComponent::GravityScale + (GrowthFactor*minorIncrement);
		}
		// Reset Movement Stats when within normal size range
		else if (currentScale.GetMax() < NormalSize + minorIncrement && currentScale.GetMax() > NormalSize)
		{
			GetCharacterMovement()->UCharacterMovementComponent::MaxWalkSpeed = 600.0f;
			GetCharacterMovement()->UCharacterMovementComponent::Mass = 1.0f;
			GetCharacterMovement()->UCharacterMovementComponent::JumpZVelocity = 500.0f;
			GetCharacterMovement()->UCharacterMovementComponent::JumpOffJumpZFactor = 0.5f;
			GetCharacterMovement()->UCharacterMovementComponent::MaxStepHeight = 45.0f;
			GetCharacterMovement()->UCharacterMovementComponent::GravityScale = 1.0f;
			SetCharacterScaleMode((CharacterScaleMode::Type) CharacterScaleMode::NormalScale);
		}
	}

	// Ability to have seamless shrink
	if (bPressedShrink && IsThirdPersonMode() && currentScale.Size() >= ShrinkMinSize)
	{
		ShrinkFactor = 1.0f;
		SetActorScale3D(currentScale - (ShrinkFactor*minorIncrement));
		CurrentBoomLength3P = CurrentBoomLength3P - (ShrinkFactor*2.0f);			//Needed to adjust 3rd person camera boom
		CameraBoom->TargetArmLength = CurrentBoomLength3P;

		if (currentScale.GetMax() < MinScale.GetMax() + minorIncrement)
		{
			SetCharacterScaleMode((CharacterScaleMode::Type) CharacterScaleMode::MinScale);
		}

		// Decrease movement speed when larger than normal
		if (currentScale.GetMax() > NormalSize +minorIncrement)
		{
			GetCharacterMovement()->UCharacterMovementComponent::MaxWalkSpeed = GetCharacterMovement()->UCharacterMovementComponent::MaxWalkSpeed - (GrowthFactor*4.0f);
			GetCharacterMovement()->UCharacterMovementComponent::Mass = GetCharacterMovement()->UCharacterMovementComponent::Mass - (GrowthFactor*2.0f);
			GetCharacterMovement()->UCharacterMovementComponent::JumpZVelocity = GetCharacterMovement()->UCharacterMovementComponent::JumpZVelocity - (GrowthFactor*4.0f);
			GetCharacterMovement()->UCharacterMovementComponent::JumpOffJumpZFactor = GetCharacterMovement()->UCharacterMovementComponent::JumpOffJumpZFactor - (GrowthFactor*4.0f);
			GetCharacterMovement()->UCharacterMovementComponent::MaxStepHeight = GetCharacterMovement()->UCharacterMovementComponent::MaxStepHeight - (GrowthFactor*2.0f);
			GetCharacterMovement()->UCharacterMovementComponent::GravityScale = GetCharacterMovement()->UCharacterMovementComponent::GravityScale - (GrowthFactor*minorIncrement);
			if (currentScale.GetMax() < MaxScale.GetMax() - 2.0f)
			{
				SetCharacterScaleMode((CharacterScaleMode::Type) CharacterScaleMode::NormalScale);
			}
		}
		// Reset Movement Stats when within normal size range
		else if (currentScale.GetMax() < NormalSize +minorIncrement && currentScale.GetMax() > NormalSize)
		{
			GetCharacterMovement()->UCharacterMovementComponent::MaxWalkSpeed = 600.0f;
			GetCharacterMovement()->UCharacterMovementComponent::Mass = 1.0f;
			GetCharacterMovement()->UCharacterMovementComponent::JumpZVelocity = 500.0f;
			GetCharacterMovement()->UCharacterMovementComponent::JumpOffJumpZFactor = 0.5f;
			GetCharacterMovement()->UCharacterMovementComponent::MaxStepHeight = 45.0f;
			GetCharacterMovement()->UCharacterMovementComponent::GravityScale = 1.0f;
			SetCharacterScaleMode((CharacterScaleMode::Type) CharacterScaleMode::NormalScale);
		}
	}
	else if (bPressedShrink && IsFirstPersonMode() && currentScale.Size() >= ShrinkMinSize)
	{
		ShrinkFactor = 1.0f;
		SetActorScale3D(currentScale - (ShrinkFactor*minorIncrement));
		CurrentBoomLength3P = CurrentBoomLength3P - (ShrinkFactor*2.0f);			//Needed to adjust 3rd person camera boom

		if (currentScale.GetMax() < MinScale.GetMax() + minorIncrement)
		{
			SetCharacterScaleMode((CharacterScaleMode::Type) CharacterScaleMode::MinScale);
		}

		// Decrease movement speed when larger than normal
		if (currentScale.GetMax() > NormalSize +minorIncrement)
		{
			GetCharacterMovement()->UCharacterMovementComponent::MaxWalkSpeed = GetCharacterMovement()->UCharacterMovementComponent::MaxWalkSpeed - (GrowthFactor*4.0f);
			GetCharacterMovement()->UCharacterMovementComponent::Mass = GetCharacterMovement()->UCharacterMovementComponent::Mass - (GrowthFactor*2.0f);
			GetCharacterMovement()->UCharacterMovementComponent::JumpZVelocity = GetCharacterMovement()->UCharacterMovementComponent::JumpZVelocity - (GrowthFactor*4.0f);
			GetCharacterMovement()->UCharacterMovementComponent::JumpOffJumpZFactor = GetCharacterMovement()->UCharacterMovementComponent::JumpOffJumpZFactor - (GrowthFactor*4.0f);
			GetCharacterMovement()->UCharacterMovementComponent::MaxStepHeight = GetCharacterMovement()->UCharacterMovementComponent::MaxStepHeight - (GrowthFactor*2.0f);
			GetCharacterMovement()->UCharacterMovementComponent::GravityScale = GetCharacterMovement()->UCharacterMovementComponent::GravityScale - (GrowthFactor*minorIncrement);

			if (currentScale.GetMax() < MaxScale.GetMax() - 2.0f)
			{
				SetCharacterScaleMode((CharacterScaleMode::Type) CharacterScaleMode::NormalScale);
			}

		}
		// Reset Movement Stats when within normal size range
		else if (currentScale.GetMax() < NormalSize + minorIncrement && currentScale.GetMax() > NormalSize)
		{
			GetCharacterMovement()->UCharacterMovementComponent::MaxWalkSpeed = 600.0f;
			GetCharacterMovement()->UCharacterMovementComponent::Mass = 1.0f;
			GetCharacterMovement()->UCharacterMovementComponent::JumpZVelocity = 500.0f;
			GetCharacterMovement()->UCharacterMovementComponent::JumpOffJumpZFactor = 0.5f;
			GetCharacterMovement()->UCharacterMovementComponent::MaxStepHeight = 45.0f;
			GetCharacterMovement()->UCharacterMovementComponent::GravityScale = 1.0f;
			SetCharacterScaleMode((CharacterScaleMode::Type) CharacterScaleMode::NormalScale);
		}
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
	PlayerInputComponent->BindAction("IncrementalGrow", IE_Pressed, this, &ACapstone_CortezCharacter::IncrementalGrow);
	PlayerInputComponent->BindAction("IncrementalGrow", IE_Released, this, &ACapstone_CortezCharacter::StopIncrementalGrowing);
	PlayerInputComponent->BindAction("IncrementalShrink", IE_Pressed, this, &ACapstone_CortezCharacter::IncrementalShrink);
	PlayerInputComponent->BindAction("IncrementalShrink", IE_Released, this, &ACapstone_CortezCharacter::StopIncrementalShrinking);


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

	// Camera inputs
	PlayerInputComponent->BindAction("ToggleCameraMode", IE_Pressed, this, &ACapstone_CortezCharacter::CycleCamera);

	// Movement inputs
	PlayerInputComponent->BindAction("ToggleMovementMode", IE_Pressed, this, &ACapstone_CortezCharacter::CycleMovement);
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

/*void ACapstone_CortezCharacter::IncrementalGrow()
{
	bPressedGrow = true;
	GrowKeyHoldTime = 0.0f;
	DoGrow();
	//CheckGrowInput(GrowKeyHoldTime);
}*/

void ACapstone_CortezCharacter::StopIncrementalGrowing()
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
			CurrentBoomLength3P = CurrentBoomLength3P * 2.0f;			//Needed to adjust 3rd person camera boom
			CameraBoom->TargetArmLength = CurrentBoomLength3P;
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

/*void ACapstone_CortezCharacter::IncrementalShrink()
{
	bPressedShrink = true;
	ShrinkKeyHoldTime = 0.0f;
	DoShrink();
}*/

void ACapstone_CortezCharacter::StopIncrementalShrinking()
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
		CameraBoom->TargetArmLength = CurrentBoomLength3P;
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

//////////////////////////////////////////////////////////////////////////
// Character Scale Mode

// Cycle Grow States
void ACapstone_CortezCharacter::IncrementalGrow()
{
	int newCharacterScaleMode = (int)CharacterScaleModeEnum + 1;

	if (newCharacterScaleMode == 3) newCharacterScaleMode = CharacterScaleMode::MaxScale;
	SetCharacterScaleMode((CharacterScaleMode::Type) newCharacterScaleMode);
}

// Cycle Shrink States
void ACapstone_CortezCharacter::IncrementalShrink()
{
	int newCharacterScaleMode = (int)CharacterScaleModeEnum - 1;

	if (newCharacterScaleMode == -1) newCharacterScaleMode = CharacterScaleMode::MinScale;
	SetCharacterScaleMode((CharacterScaleMode::Type) newCharacterScaleMode);
}

// Set the scale mode
void ACapstone_CortezCharacter::SetCharacterScaleMode(CharacterScaleMode::Type newScaleMode)
{
	CharacterScaleModeEnum = newScaleMode;
	UpdateForCharacterScaleMode();

}

// Determines character scale values
void ACapstone_CortezCharacter::UpdateForCharacterScaleMode()
{
	// Offset Z location so character won't go through floor when growing
	FVector GrowLocationOffset = FVector(0.0f, 0.0f, 350.0f);

	// Offset Z locaton so character won't fall when shrinking
	FVector ShrinkLocationOffset = FVector(0.0f, 0.0f, 50.0f);

	// Changes visibility of first and third person meshes
	switch (CharacterScaleModeEnum)
	{
	case CharacterScaleMode::MinScale:
		if (!bPressedGrow && !bPressedShrink)
		{
			SetActorScale3D(MinScale);
			CurrentBoomLength3P = 130.0f;			//Needed to adjust 3rd person camera boom
			CameraBoom->TargetArmLength = CurrentBoomLength3P;
		}
		// Make sure we don't readjust Z location if already min size
		if (!IsAlreadyMinScaleMode && (!bPressedGrow && !bPressedShrink))
			SetActorRelativeLocation(GetActorLocation() - ShrinkLocationOffset);
		IsAlreadyMinScaleMode = true;
		IsAlreadyMaxScaleMode = false;
		IsAlreadyNormalScaleMode = false;
		break;
	case CharacterScaleMode::NormalScale:
		if (!bPressedGrow && !bPressedShrink)
		{
			SetActorScale3D(NormalScale);
			CurrentBoomLength3P = 300.0f;			//Needed to adjust 3rd person camera boom
			CameraBoom->TargetArmLength = CurrentBoomLength3P;
			GetCharacterMovement()->UCharacterMovementComponent::MaxWalkSpeed = 600.0f;
			GetCharacterMovement()->UCharacterMovementComponent::Mass = 1.0f;
			GetCharacterMovement()->UCharacterMovementComponent::JumpZVelocity = 500.0f;
			GetCharacterMovement()->UCharacterMovementComponent::JumpOffJumpZFactor = 0.5f;
			GetCharacterMovement()->UCharacterMovementComponent::MaxStepHeight = 45.0f;
			GetCharacterMovement()->UCharacterMovementComponent::GravityScale = 1.0f;
		}
		if (IsAlreadyMaxScaleMode && (!bPressedGrow && !bPressedShrink))
			SetActorRelativeLocation(GetActorLocation() - GrowLocationOffset);
		else if (IsAlreadyMinScaleMode && (!bPressedGrow && !bPressedShrink))
			SetActorRelativeLocation(GetActorLocation() + ShrinkLocationOffset);
		IsAlreadyNormalScaleMode = true;
		IsAlreadyMinScaleMode = false;
		IsAlreadyMaxScaleMode = false;
		break;
	case CharacterScaleMode::MaxScale:
		if (!bPressedGrow && !bPressedShrink)
		{
			SetActorScale3D(MaxScale);
			CurrentBoomLength3P = 1256.0f;			//Needed to adjust 3rd person camera boom
			CameraBoom->TargetArmLength = CurrentBoomLength3P;
			GetCharacterMovement()->UCharacterMovementComponent::MaxWalkSpeed = 1800.0f;
			GetCharacterMovement()->UCharacterMovementComponent::Mass = 6.0f;
			GetCharacterMovement()->UCharacterMovementComponent::JumpZVelocity = 3000.0f;
			GetCharacterMovement()->UCharacterMovementComponent::JumpOffJumpZFactor = 3.0f;
			GetCharacterMovement()->UCharacterMovementComponent::MaxStepHeight = 270.0f;
			GetCharacterMovement()->UCharacterMovementComponent::GravityScale = 6.0f;
		}
		
		// Make sure we don't readjust Z location if already max size
		if (!IsAlreadyMaxScaleMode && (!bPressedGrow && !bPressedShrink))
			SetActorRelativeLocation(GetActorLocation() + GrowLocationOffset);
		IsAlreadyMaxScaleMode = true;
		IsAlreadyMinScaleMode = false;
		IsAlreadyNormalScaleMode = false;
		break;

	default:
		break;
	}
}

bool ACapstone_CortezCharacter::IsNormalScaleMode()
{
	return IsNormalScale(CharacterScaleModeEnum);
}

bool ACapstone_CortezCharacter::IsMinScaleMode()
{
	return IsMinScale(CharacterScaleModeEnum);
}

bool ACapstone_CortezCharacter::IsMaxScaleMode()
{
	return IsMaxScale(CharacterScaleModeEnum);
}

//////////////////////////////////////////////////////////////////////////
// Movement Mode

// Cycles the character's movement mode
void ACapstone_CortezCharacter::CycleMovement()
{

	int newMovementMode = (int)MovementModeEnum + 1;

	if (newMovementMode == 2) newMovementMode = CharacterMovementMode::WalkDefault;
	SetMovementMode((CharacterMovementMode::Type) newMovementMode);
}

// Set Movement mode
void ACapstone_CortezCharacter::SetMovementMode(CharacterMovementMode::Type newMovementMode)
{
	MovementModeEnum = newMovementMode;
	UpdateForMovementMode();

}

// Determines character movement between flight and nonflight
void ACapstone_CortezCharacter::UpdateForMovementMode()
{

	// Changes Movement mode
	switch (MovementModeEnum)
	{
	case CharacterMovementMode::WalkDefault:
		GetCharacterMovement()->UCharacterMovementComponent::SetMovementMode(MOVE_Walking);
		break;
	case CharacterMovementMode::Fly:
		GetCharacterMovement()->UCharacterMovementComponent::SetMovementMode(MOVE_Flying);
	default:
		break;
	}
}

bool ACapstone_CortezCharacter::IsWalkMode()
{
	return IsWalking(MovementModeEnum);
}

bool ACapstone_CortezCharacter::IsFlyMode()
{
	return IsFlying(MovementModeEnum);
}