// Joseph Cortez
// Capstone project

// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#include "Capstone_Cortez.h"
#include "Kismet/HeadMountedDisplayFunctionLibrary.h"
#include "Capstone_CortezCharacter.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Engine/SkeletalMeshSocket.h"
#include "EngineGlobals.h"
#include "Engine.h"
#include "Capstone_CortezGameMode.h"
#include "InventoryHUD.h"



//////////////////////////////////////////////////////////////////////////
// ACapstone_CortezCharacter

ACapstone_CortezCharacter::ACapstone_CortezCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;
	PrimaryActorTick.bTickEvenWhenPaused = true;

	// set our turn rates for input
	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;

	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Default 3rd person boom length
	CurrentBoomLength3P = 300.0f;

	FName headSocket = "headSocket";				// Socket to attach camera boom to

	// Incremental values
	minorIncrement = 0.01f;
	superMinorIncrement = 0.001f;
	doubleIncrement = 2.0f;
	tripleIncrement = 3.0f;
	quadIncrement = 4.0f;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f); // ...at this rotation rate
	GetCharacterMovement()->JumpZVelocity = 500.0f;
	

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = CurrentBoomLength3P; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	// Useed for follow camera
	CameraFollowTurnAngleExponent = .5f;
	CameraFollowTurnRate = .6f;
	CameraResetSpeed = 2.f;
	
	IsResetting = false;
	AutoResetSmoothFollowCameraWhenIdle = true;
	AutoResetDelaySeconds = 5.f;

	IsAutoReset = false;
	AutoResetSpeed = .15f;

	// Character Size variables
	NormalSize = 1.0f;
	GrowthFactor = 0.5f;
	GrowMaxSize = 10.0f;
	ShrinkMinSize = 0.25f;

	// Maximum grow/ shrink hold times
	GrowMaxHoldTime = 10;
	ShrinkMaxHoldTime = 10;

	// Default scale values for scale modes
	NormalScale = FVector(1.0f, 1.0f, 1.0f);
	MaxScale = FVector(5.773502f, 5.773502f, 5.773502f);
	MinScale = FVector(0.144338f, 0.144338f, 0.144338f);

	// Give character the ability to fly
	GetCharacterMovement()->UCharacterMovementComponent::NavAgentProps.bCanFly = true;

	// Set character in normal scale mode to begin
	SetCharacterScaleMode((CharacterScaleMode::Type) CharacterScaleMode::NormalScale);

	// Set character's forward reach
	CharacterReach = 250.0f;

	// Set the character's health
	CharacterHealth = 50.0f;

	// Set the character's max health
	CharacterMaxHealth = 100.0f;

	// Set the character's min health
	CharacterMinHealth = 0.0f;
	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named MyCharacter (to avoid direct content references in C++)
}

// Called when the game starts or when spawned
void ACapstone_CortezCharacter::BeginPlay()
{
	Super::BeginPlay();

	Inventory.SetNum(6);

	CurrentInteraction = nullptr;
}

// Called every frame
void ACapstone_CortezCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	FVector currentScale = GetActorScale3D();						// Get the current scale of the character
	currentScale = currentScale.GetClampedToSize(ShrinkMinSize, GrowMaxSize);		// Add constraints to scale
	
	// Ability to have seamless growth
	if (bPressedGrow && IsThirdPersonMode() && currentScale.Size() < GrowMaxSize -superMinorIncrement && IsWalkMode())
	{
		GrowthFactor = 1.0f;

		SetActorScale3D(currentScale + (GrowthFactor*minorIncrement));
		CurrentBoomLength3P = CurrentBoomLength3P + (GrowthFactor*doubleIncrement);			//Needed to adjust 3rd person camera boom
		CameraBoom->TargetArmLength = CurrentBoomLength3P;

		if (currentScale.GetMax() > MaxScale.GetMax() - minorIncrement)
		{
			SetCharacterScaleMode((CharacterScaleMode::Type) CharacterScaleMode::MaxScale);
		}
		
		// Increase movement speed when larger than normal
		if (currentScale.GetMax() > NormalSize + minorIncrement)
		{
			GetCharacterMovement()->UCharacterMovementComponent::MaxWalkSpeed = GetCharacterMovement()->UCharacterMovementComponent::MaxWalkSpeed + (GrowthFactor*quadIncrement);
			GetCharacterMovement()->UCharacterMovementComponent::Mass = GetCharacterMovement()->UCharacterMovementComponent::Mass + (GrowthFactor*doubleIncrement);
			GetCharacterMovement()->UCharacterMovementComponent::JumpZVelocity = GetCharacterMovement()->UCharacterMovementComponent::JumpZVelocity + (GrowthFactor*quadIncrement);
			GetCharacterMovement()->UCharacterMovementComponent::JumpOffJumpZFactor = GetCharacterMovement()->UCharacterMovementComponent::JumpOffJumpZFactor + (GrowthFactor*quadIncrement);
			GetCharacterMovement()->UCharacterMovementComponent::MaxStepHeight = GetCharacterMovement()->UCharacterMovementComponent::MaxStepHeight + (GrowthFactor*doubleIncrement);
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

		// Makes it possible for character to initiate jetpack when half size
		else if (currentScale.GetMax() > (NormalSize / doubleIncrement))
			SetCharacterScaleMode((CharacterScaleMode::Type) CharacterScaleMode::NormalScale);
	}
	else if(bPressedGrow && IsFirstPersonMode() && currentScale.Size() < GrowMaxSize -minorIncrement && IsWalkMode())
	{
		GrowthFactor = 1.0f;
		SetActorScale3D(currentScale + (GrowthFactor*minorIncrement));
		CurrentBoomLength3P = CurrentBoomLength3P + (GrowthFactor*doubleIncrement);			//Needed to adjust 3rd person camera boom

		if (currentScale.GetMax() > MaxScale.GetMax() - minorIncrement)
		{
			SetCharacterScaleMode((CharacterScaleMode::Type) CharacterScaleMode::MaxScale);
		}
		
		// Increase movement speed when larger than normal
		if (currentScale.GetMax() > NormalSize + minorIncrement)
		{
			GetCharacterMovement()->UCharacterMovementComponent::MaxWalkSpeed = GetCharacterMovement()->UCharacterMovementComponent::MaxWalkSpeed + (GrowthFactor*quadIncrement);
			GetCharacterMovement()->UCharacterMovementComponent::Mass = GetCharacterMovement()->UCharacterMovementComponent::Mass + (GrowthFactor *minorIncrement);
			GetCharacterMovement()->UCharacterMovementComponent::JumpZVelocity = GetCharacterMovement()->UCharacterMovementComponent::JumpZVelocity + (GrowthFactor*quadIncrement);
			GetCharacterMovement()->UCharacterMovementComponent::JumpOffJumpZFactor = GetCharacterMovement()->UCharacterMovementComponent::JumpOffJumpZFactor + (GrowthFactor*quadIncrement);
			GetCharacterMovement()->UCharacterMovementComponent::MaxStepHeight = GetCharacterMovement()->UCharacterMovementComponent::MaxStepHeight + (GrowthFactor*doubleIncrement);
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

		// Makes it possible for character to initiate jetpack when half size
		else if (currentScale.GetMax() > (NormalSize / doubleIncrement))
			SetCharacterScaleMode((CharacterScaleMode::Type) CharacterScaleMode::NormalScale);
	}

	// Ability to have seamless shrink
	if (bPressedShrink && IsThirdPersonMode() && currentScale.Size() >= ShrinkMinSize)
	{
		ShrinkFactor = 1.0f;
		SetActorScale3D(currentScale - (ShrinkFactor*minorIncrement));
		CurrentBoomLength3P = CurrentBoomLength3P - (ShrinkFactor*doubleIncrement);			//Needed to adjust 3rd person camera boom
		CameraBoom->TargetArmLength = CurrentBoomLength3P;

		if (currentScale.GetMax() < MinScale.GetMax() + minorIncrement)
		{
			SetCharacterScaleMode((CharacterScaleMode::Type) CharacterScaleMode::MinScale);
		}

		// Decrease movement speed when larger than normal
		if (currentScale.GetMax() > NormalSize +minorIncrement)
		{
			GetCharacterMovement()->UCharacterMovementComponent::MaxWalkSpeed = GetCharacterMovement()->UCharacterMovementComponent::MaxWalkSpeed - (GrowthFactor*quadIncrement);
			GetCharacterMovement()->UCharacterMovementComponent::Mass = GetCharacterMovement()->UCharacterMovementComponent::Mass - (GrowthFactor*doubleIncrement);
			GetCharacterMovement()->UCharacterMovementComponent::JumpZVelocity = GetCharacterMovement()->UCharacterMovementComponent::JumpZVelocity - (GrowthFactor*quadIncrement);
			GetCharacterMovement()->UCharacterMovementComponent::JumpOffJumpZFactor = GetCharacterMovement()->UCharacterMovementComponent::JumpOffJumpZFactor - (GrowthFactor*quadIncrement);
			GetCharacterMovement()->UCharacterMovementComponent::MaxStepHeight = GetCharacterMovement()->UCharacterMovementComponent::MaxStepHeight - (GrowthFactor*doubleIncrement);
			GetCharacterMovement()->UCharacterMovementComponent::GravityScale = GetCharacterMovement()->UCharacterMovementComponent::GravityScale - (GrowthFactor*minorIncrement);
			if (currentScale.GetMax() < MaxScale.GetMax() - doubleIncrement)
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
		// Makes it possible for character to initiate jetpack when half size
		else if (currentScale.GetMax() < (NormalSize / doubleIncrement))
			SetCharacterScaleMode((CharacterScaleMode::Type) CharacterScaleMode::MinScale);
	}
	else if (bPressedShrink && IsFirstPersonMode() && currentScale.Size() >= ShrinkMinSize)
	{
		ShrinkFactor = 1.0f;
		SetActorScale3D(currentScale - (ShrinkFactor*minorIncrement));
		CurrentBoomLength3P = CurrentBoomLength3P - (ShrinkFactor*doubleIncrement);			//Needed to adjust 3rd person camera boom

		if (currentScale.GetMax() < MinScale.GetMax() + minorIncrement)
		{
			SetCharacterScaleMode((CharacterScaleMode::Type) CharacterScaleMode::MinScale);
		}

		// Decrease movement speed when larger than normal
		if (currentScale.GetMax() > NormalSize +minorIncrement)
		{
			GetCharacterMovement()->UCharacterMovementComponent::MaxWalkSpeed = GetCharacterMovement()->UCharacterMovementComponent::MaxWalkSpeed - (GrowthFactor*quadIncrement);
			GetCharacterMovement()->UCharacterMovementComponent::Mass = GetCharacterMovement()->UCharacterMovementComponent::Mass - (GrowthFactor*doubleIncrement);
			GetCharacterMovement()->UCharacterMovementComponent::JumpZVelocity = GetCharacterMovement()->UCharacterMovementComponent::JumpZVelocity - (GrowthFactor*quadIncrement);
			GetCharacterMovement()->UCharacterMovementComponent::JumpOffJumpZFactor = GetCharacterMovement()->UCharacterMovementComponent::JumpOffJumpZFactor - (GrowthFactor*quadIncrement);
			GetCharacterMovement()->UCharacterMovementComponent::MaxStepHeight = GetCharacterMovement()->UCharacterMovementComponent::MaxStepHeight - (GrowthFactor*doubleIncrement);
			GetCharacterMovement()->UCharacterMovementComponent::GravityScale = GetCharacterMovement()->UCharacterMovementComponent::GravityScale - (GrowthFactor*minorIncrement);

			if (currentScale.GetMax() < MaxScale.GetMax() - doubleIncrement)
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
		// Makes it possible for character to initiate jetpack when half size
		else if (currentScale.GetMax() < (NormalSize / doubleIncrement))
			SetCharacterScaleMode((CharacterScaleMode::Type) CharacterScaleMode::MinScale);
	}

	// Checking for interaction item in front of player
	CheckForInteractionItem();

	ACapstone_CortezGameMode* MyGameMode = Cast<ACapstone_CortezGameMode>(UGameplayStatics::GetGameMode(this));

	// If health reaches 0, change state to game over
	if (CharacterHealth == 0)
	{
		MyGameMode->ChangeCurrentPlayState(EPlayState::GameOver);
	}

	GetCharacterHealth();
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
	PlayerInputComponent->BindAction("Interact", IE_Pressed, this, &ACapstone_CortezCharacter::Interact);
	PlayerInputComponent->BindAction("ToggleInventoryScreen", IE_Pressed, this, &ACapstone_CortezCharacter::ToggleInventory);


	PlayerInputComponent->BindAxis("MoveForward", this, &ACapstone_CortezCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &ACapstone_CortezCharacter::MoveRight);
	PlayerInputComponent->BindAxis("JetPackBoost", this, &ACapstone_CortezCharacter::JetPack);

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

/** Movement Function to allow for up and down movment of jetpack **/
void ACapstone_CortezCharacter::JetPack(float Value)
{
	if (Value == 0.f) return;

	if ((Controller != NULL) && (IsThirdPersonMode()))
	{
		// find out which way is up
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get up vector
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Z);
		AddMovementInput(Direction, Value);
	}
	else
	{
		AddMovementInput(GetActorUpVector(), Value);
	}
}

//////////////////////////////////////////////////////////////////////////
// Grow Functions

void ACapstone_CortezCharacter::Grow()
{
	bPressedGrow = true;
	GrowKeyHoldTime = 0.0f;
}

void ACapstone_CortezCharacter::StopGrowing()
{
	bPressedGrow = false;
	GrowthFactor = 0.5f;
	ResetGrowState();
}

void ACapstone_CortezCharacter::StopIncrementalGrowing()
{
	bPressedGrow = false;
	GrowthFactor = 0.5f;
	ResetGrowState();
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

void ACapstone_CortezCharacter::StopIncrementalShrinking()
{
	bPressedShrink = false;
	ShrinkFactor = 0.5f;
	ResetShrinkState();
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
		CameraBoom->TargetArmLength = 0.0f;
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
	if (IsWalkMode() && IsThirdPersonMode())
	{
		int newCharacterScaleMode = (int)CharacterScaleModeEnum + 1;

		if (newCharacterScaleMode == 3) newCharacterScaleMode = CharacterScaleMode::MaxScale;
		SetCharacterScaleMode((CharacterScaleMode::Type) newCharacterScaleMode);
	}
	
}

// Cycle Shrink States
void ACapstone_CortezCharacter::IncrementalShrink()
{
	if (IsWalkMode() && IsThirdPersonMode())
	{
		int newCharacterScaleMode = (int)CharacterScaleModeEnum - 1;

		if (newCharacterScaleMode == -1) newCharacterScaleMode = CharacterScaleMode::MinScale;
		SetCharacterScaleMode((CharacterScaleMode::Type) newCharacterScaleMode);
	}
	
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
	if (IsAlreadyMinScaleMode && !GetCharacterMovement()->IsFalling())
	{
		int newMovementMode = (int)MovementModeEnum + 1;

		if (newMovementMode == 2) newMovementMode = CharacterMovementMode::WalkDefault;
		GetCharacterMovement()->StopMovementImmediately();								// Stops character movement when transitioning to flight
		SetMovementMode((CharacterMovementMode::Type) newMovementMode);
		Controller->SetIgnoreMoveInput(true);											// Doesn't let character move during animation
	}
	
}

// Set Movement mode
void ACapstone_CortezCharacter::SetMovementMode(CharacterMovementMode::Type newMovementMode)
{
	// Set up a latent action timer so movement is not allowed during animation
	FLatentActionInfo LatentActionInfo;
	LatentActionInfo.CallbackTarget = this;
	LatentActionInfo.ExecutionFunction = "ReinitializeMovement";
	LatentActionInfo.UUID = 123;
	LatentActionInfo.Linkage = 0;

	MovementModeEnum = newMovementMode;
	UpdateForMovementMode();
	if (newMovementMode == 1)
		UKismetSystemLibrary::Delay(this, 2, LatentActionInfo);			// Set delay for changing to flight mode
	else
		UKismetSystemLibrary::Delay(this, 1.5, LatentActionInfo);		// Set delay for changing to walking mode

}

// Determines character movement between flight and nonflight
void ACapstone_CortezCharacter::UpdateForMovementMode()
{
	FVector currentScale = GetActorScale3D();						// Get the current scale of the character
	// Changes Movement mode
	switch (MovementModeEnum)
	{
	case CharacterMovementMode::WalkDefault:
		GetCharacterMovement()->UCharacterMovementComponent::SetMovementMode(MOVE_Walking);
		// Need to off Controller Rotation Yaw if in Third Person mode and leave on if in First Person Mode
		if(IsThirdPersonMode())
			bUseControllerRotationYaw = false;
		break;
	case CharacterMovementMode::Fly:
		GetCharacterMovement()->UCharacterMovementComponent::SetMovementMode(MOVE_Flying);
		bUseControllerRotationYaw = true;
		break;
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

/** Sets controller to accept movements **/
void ACapstone_CortezCharacter::ReinitializeMovement()
{
	Controller->ResetIgnoreMoveInput();
}

// Interact with object
void ACapstone_CortezCharacter::Interact()
{
	if (CurrentInteraction != nullptr)
	{
		CurrentInteraction->Interact_Implementation();
	}
}

// Toggle Inventory Screen
void ACapstone_CortezCharacter::ToggleInventory()
{
	ACapstone_CortezGameMode* GameMode = Cast<ACapstone_CortezGameMode>(GetWorld()->GetAuthGameMode());
	APlayerController* MyController = GetWorld()->GetFirstPlayerController();
	
	// Check player's HUD state if inventory is open
	if (GameMode->GetHUDState() == GameMode->HS_InGame)
	{
		GameMode->ChangeHUDState(GameMode->HS_Inventory);
		MyController->SetInputMode(FInputModeGameAndUI());
	}
	else
	{
		GameMode->ChangeHUDState(GameMode->HS_InGame);
		MyController->SetInputMode(FInputModeGameOnly());
	}

}

// Check if there is an interaction item in front of character
void ACapstone_CortezCharacter::CheckForInteractionItem()
{
	// Ray Tracing variables
	FVector StartLineTrace;
	FVector EndLineTrace;
	
	if (IsFirstPersonMode())
	{
		StartLineTrace = FollowCamera->GetComponentLocation();
		EndLineTrace = (FollowCamera->GetForwardVector() * CharacterReach) + StartLineTrace;

	}
	else
	{
		StartLineTrace = CameraBoom->GetComponentLocation();
		EndLineTrace = (CameraBoom->GetForwardVector() * CharacterReach) + StartLineTrace;
	}

	// Declare hitresult for raycaster
	FHitResult HitResult;

	// Initialize query params & ignore character
	FCollisionQueryParams CQP;
	CQP.AddIgnoredActor(this);

	// Cast the line trace
	GetWorld()->LineTraceSingleByChannel(HitResult, StartLineTrace, EndLineTrace, ECC_WorldDynamic, CQP);
	AInteraction* PotentialInteraction = Cast<AInteraction>(HitResult.GetActor());

	if (PotentialInteraction == NULL)
	{
		ScreenText = "";
		CurrentInteraction = nullptr;
		return;
	}
	else
	{
		CurrentInteraction = PotentialInteraction;
		ScreenText = PotentialInteraction->InteractionText;
	}
			
	 
}

/** Adds item to inventory slot **/
bool ACapstone_CortezCharacter::AddItemToInventory(APickup * Item)
{
	if (Item != NULL)
	{
		// Find first empty slot
		const int32 AvailableSlot = Inventory.Find(nullptr);

		if (AvailableSlot != INDEX_NONE)
		{
			Inventory[AvailableSlot] = Item;
			return true;
		}
		else
		{
			GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("Your inventory is full"));
			return false;
		}

	}
	else return false;
	
}

/**Get's the thumbnail texture at the inventory slot **/
UTexture2D* ACapstone_CortezCharacter::GetThumbnailAtInventorySlot(int32 Slot)
{
	if (Inventory[Slot] != NULL)
	{
		return Inventory[Slot]->PickupThumbnail;
	}
	else return nullptr;
}

/** Get's the item name at the inventory slot **/
FString ACapstone_CortezCharacter::GetItemNameAtInventorySlot(int32 Slot)
{
	if (Inventory[Slot] != NULL)
	{
		return Inventory[Slot]->ItemName;
	}
	else return FString("None");
}

/** Use item at the inventory slot **/
void ACapstone_CortezCharacter::UseItemAtInventorySlot(int32 Slot)
{
	if (Inventory[Slot] != NULL)
	{
		Inventory[Slot]->Use_Implementation();

		// Remove item from inventory
		Inventory[Slot] = NULL;
	}
}

/////////Health Functions
////////////////////////////////////////////

void ACapstone_CortezCharacter::UpdateCharacterHealth(float Health)
{
	float newCharacterHealth;

	newCharacterHealth = CharacterHealth - Health;

	//  Check if we can add or decrease health and apply health change
	if (CharacterHealth == 100)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("You are already at full health"));
		bCanUseItem = false;
	}
	else if (newCharacterHealth <= 100.0f && newCharacterHealth >= 0.0f)
	{
		CharacterHealth = newCharacterHealth;
		bCanUseItem = true;
	}
	else if (newCharacterHealth > 100.0f)
	{
		CharacterHealth = CharacterMaxHealth;
		bCanUseItem = true;
	}
	else if (newCharacterHealth < 0.0f)
	{
		CharacterHealth = CharacterMinHealth;
		bCanUseItem = true;
	}
}

float ACapstone_CortezCharacter::GetCharacterHealth()
{
	return CharacterHealth;
}




