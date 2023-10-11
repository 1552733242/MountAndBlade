// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/QCharacter.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Curves/CurveVector.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"

#define Message(key,...) GEngine->AddOnScreenDebugMessage(key, 1, FColor::Red,FString::Format(TEXT("{0}"), { FStringFormatArg(##__VA_ARGS__)}));




AQCharacter::AQCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
	BindDeclares();

	if (GetMovementComponent())
	{
		GetMovementComponent()->GetNavAgentPropertiesRef().bCanCrouch = true;
	}
}

void AQCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	GetMesh()->AddTickPrerequisiteActor(this);
	MainAnimInstacne = GetMesh()->GetAnimInstance();
	SetMovementModel();
	OnGaitChanged(DesiredGit);
	OnRotationModeChanged(DesiredRotationMode);
	OnViewModeChanged(ViewMode);
	OnOverlayStateChanged(OverlayState);
	if (DesiredStance == ECharacterMovementStance::Crouching)Crouch();
	TargetRotation = GetActorRotation();
	LastVelocityRotation = TargetRotation;
	LastMovementInputRotation = TargetRotation;
}

void AQCharacter::BindDeclares()
{
	SetOverlayState.AddUObject(this, &AQCharacter::OnOverlayStateChanged);
	SetViewMode.AddUObject(this, &AQCharacter::OnViewModeChanged);
	SetGait.AddUObject(this, &AQCharacter::OnGaitChanged);
	SetRotation.AddUObject(this, &AQCharacter::OnRotationModeChanged);
	SetMovementAction.AddUObject(this, &AQCharacter::OnMovementActionChanged);
	SetMovementState.AddUObject(this, &AQCharacter::OnMovementStateChanged);

	BreakFallEvent.AddUObject(this, &AQCharacter::OnBreakFall);
}

void AQCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	SetEssentialValues();
	CacheValues();
	switch (MovementState){
	case ECharacterMovementState::OnGround:
		UpdateCharacterMovement();
		break;
	}
	
}

void AQCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	
	if (APlayerController* PlayerController = CastChecked<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* EnhancedInputLocalPlayerSubsystem =
			ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			EnhancedInputLocalPlayerSubsystem->AddMappingContext(IMBase, 0);
		}
	}
	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
	{
		//Locomotion Bind
		EnhancedInputComponent->BindAction(IAMove, ETriggerEvent::Triggered, this, &AQCharacter::OnMove);
		EnhancedInputComponent->BindAction(IALook, ETriggerEvent::Triggered, this, &AQCharacter::OnLook);
		EnhancedInputComponent->BindAction(IAZoom, ETriggerEvent::Triggered, this, &AQCharacter::OnZoom);
		EnhancedInputComponent->BindAction(IASprint, ETriggerEvent::Started, this, &AQCharacter::OnSpring);
		EnhancedInputComponent->BindAction(IASprint, ETriggerEvent::Completed, this, &AQCharacter::OnStopSpring);

		EnhancedInputComponent->BindAction(IAJump, ETriggerEvent::Started, this, &AQCharacter::Jump);
		EnhancedInputComponent->BindAction(IAJump, ETriggerEvent::Completed, this, &AQCharacter::StopJumping);
		EnhancedInputComponent->BindAction(IACrouch, ETriggerEvent::Started, this, &AQCharacter::OnCrouch);
	}

}

void AQCharacter::OnMovementModeChanged(EMovementMode PrevMovementMode, uint8 PreviousCustomMode)
{
	Super::OnMovementModeChanged(PrevMovementMode, PreviousCustomMode);
	switch (GetCharacterMovement()->MovementMode)
	{
	case EMovementMode::MOVE_Walking:SetMovementState.Broadcast(ECharacterMovementState::OnGround);
		return;
	case EMovementMode::MOVE_NavWalking:SetMovementState.Broadcast(ECharacterMovementState::OnGround);
		return;
	case EMovementMode::MOVE_Falling:SetMovementState.Broadcast(ECharacterMovementState::InAir);
		return;
	}

}

void AQCharacter::OnStartCrouch(float HeightAdjust, float ScaledHeightAdjust)
{
	Super::OnStartCrouch(HeightAdjust, ScaledHeightAdjust);
	OnStanceChanged(ECharacterMovementStance::Crouching);
}

void AQCharacter::OnEndCrouch(float HeightAdjust, float ScaledHeightAdjust)
{
	Super::OnEndCrouch(HeightAdjust, ScaledHeightAdjust);
	OnStanceChanged(ECharacterMovementStance::Stance);
}

void AQCharacter::Landed(const FHitResult& Hit)
{
	Super::Landed(Hit);
	if (BreakFall) {
		BreakFallEvent.Broadcast();
	}
	else {
		if (HasMovementInput) {
			GetCharacterMovement()->BrakingFrictionFactor = 0.5f;
		}
		else {
			GetCharacterMovement()->BrakingFrictionFactor = 3.f;
		}
	}
	UKismetSystemLibrary::RetriggerableDelay(GetWorld(), 0.5f, FLatentActionInfo());

}

void AQCharacter::OnJumped_Implementation()
{
	Super::OnJumped_Implementation();
	if (Speed > 100.f) {
		InAirRotation = LastVelocityRotation;
	}
	else {
		InAirRotation = GetActorRotation();
	}
	if (MainAnimInstacne) {

	}
}

void AQCharacter::GetCurrentStates(FCharacterMovementCurrentStates& Info)
{
	Info.MovementState = MovementState;
	Info.PrevMovementState = PreviousMovementState;
	Info.RotationMode = MovementRotationMode;
	Info.ActualGait = MovementGait;
	Info.ActualStance = MovementStance;
}

void AQCharacter::GetEssentialValues(FCharacterMovementEssentialValues& Info)
{
	Info.Velocity = GetVelocity();
	Info.Acceleration = Acceleration;
	Info.MovementInput = GetCharacterMovement()->GetCurrentAcceleration();
	Info.IsMoving = IsMoving;
	Info.HasMovementInput = HasMovementInput;
	Info.Speed = Speed;
	Info.MovementInputAmount = MovementInputAmount;
	Info.AimingRotation = GetControlRotation();
	Info.AimYawRate = AimYawRate;
}

void AQCharacter::Get3PPivotTarget(FTransform& Transform)
{
	Transform = GetActorTransform();
}

void AQCharacter::GetCameraParameters(FCameraParameters& CameraParameters)
{
	CameraParameters =  FCameraParameters(ThirdPersonFOV, FirstPersonFOV, RightShoulder);
}

void AQCharacter::UpdateCharacterMovement()
{
	ECharacterMovementGait AllowedGait = GetAllowedGait();
	ECharacterMovementGait ActualGait = GetActualGait(AllowedGait);
	if (ActualGait != MovementGait)SetGait.Broadcast(ActualGait);
	UpdateDynamicMovementSettings(AllowedGait);
}

void AQCharacter::UpdateGroudedRotation()
{
}

void AQCharacter::SetEssentialValues()
{
	Acceleration = CalculateAcceleration();
	FVector TempVelocity = GetVelocity();
	Speed = FVector(TempVelocity.X, TempVelocity.Y, 0.0f).Length();
	IsMoving = Speed > 1.0f;
	if (IsMoving) {
		LastVelocityRotation = TempVelocity.Rotation();
	}
	UCharacterMovementComponent* MovementComponent = GetCharacterMovement();
	FVector CurrentAccleration = MovementComponent->GetCurrentAcceleration();
	float CurrentAcclerationLength = CurrentAccleration.Length();
	float MaxAccleration = MovementComponent->GetMaxAcceleration();
	MovementInputAmount = CurrentAcclerationLength / MaxAccleration;
	HasMovementInput = MovementInputAmount > 0.0f;
	if (HasMovementInput) {
		LastMovementInputRotation = CurrentAccleration.Rotation();
	}
	float Angle = GetControlRotation().Yaw - PreviousAimYaw;
	AimYawRate = fabs(Angle / UGameplayStatics::GetWorldDeltaSeconds(GetWorld()));
}

void AQCharacter::CacheValues()
{
	PreviousVelocity =  GetVelocity();
	PreviousAimYaw = GetControlRotation().Yaw;
}

FVector AQCharacter::CalculateAcceleration()
{
	FVector VelocityAmount = GetVelocity() - PreviousVelocity;
	VelocityAmount /= UGameplayStatics::GetWorldDeltaSeconds(GetWorld());
	return VelocityAmount;
}

ECharacterMovementGait AQCharacter::GetAllowedGait()
{
	auto GetFreeStandingAllowGait = [this]() {
		switch (DesiredGit) {
		case ECharacterMovementGait::Walk: return ECharacterMovementGait::Walk;
		case ECharacterMovementGait::Run: return ECharacterMovementGait::Run;
		case ECharacterMovementGait::Sprint:return  CanSprint() ? ECharacterMovementGait::Sprint : ECharacterMovementGait::Run;
		}
		return  ECharacterMovementGait();
		}; // 获取不被锁定的站立允许的步态
	auto GetCrouhOrAimingAllowGait = [this]() {
		switch (DesiredGit) {
		case ECharacterMovementGait::Walk: return ECharacterMovementGait::Walk;
		case ECharacterMovementGait::Run: return ECharacterMovementGait::Run;
		case ECharacterMovementGait::Sprint: return ECharacterMovementGait::Run;
		}
		return  ECharacterMovementGait();
		};//获取下蹲 瞄准 允许的步态
	auto GetStandingAllowGait = [&]() {
		switch (MovementRotationMode)
		{
		case ECharacterMovementRotationMode::VelocityDirection:	return GetFreeStandingAllowGait();
		case ECharacterMovementRotationMode::LookingDirection: return GetFreeStandingAllowGait();
		case ECharacterMovementRotationMode::Aiming:		return GetCrouhOrAimingAllowGait();
		}
		return  ECharacterMovementGait();
		};//获取站立允许的步态	
	switch (MovementStance)
	{
	case ECharacterMovementStance::Stance:		return GetStandingAllowGait();
	case ECharacterMovementStance::Crouching:	return GetCrouhOrAimingAllowGait();
	}
	return  ECharacterMovementGait();
}

ECharacterMovementGait AQCharacter::GetActualGait(ECharacterMovementGait AllowedGait)
{
	if (Speed >= CurrentMovementSetting.RunSpeed + 10.f) {

		switch (AllowedGait)
		{
		case ECharacterMovementGait::Walk:return ECharacterMovementGait::Run;
		case ECharacterMovementGait::Run:return ECharacterMovementGait::Run;
		case ECharacterMovementGait::Sprint:return ECharacterMovementGait::Sprint;
		}

	}
	else if (Speed >= CurrentMovementSetting.WalkSpeed + 10.f) {
		return ECharacterMovementGait::Run;
	}
	else {
		return ECharacterMovementGait::Walk;
	}
	return ECharacterMovementGait();
}

FCharacterMovementSetting AQCharacter::GetTargetMovementSettings()
{
	auto SwitchStance = [this](const FCharacterMovementSettingStance& Data) {
			switch (MovementStance)
			{
			case ECharacterMovementStance::Stance:return Data.Standing;
			case ECharacterMovementStance::Crouching:return Data.Crouching;
			}
			return FCharacterMovementSetting();
		};
	switch (MovementRotationMode)
	{
	case ECharacterMovementRotationMode::VelocityDirection:return SwitchStance(MovementConfigData.VelocityDirection);
	case ECharacterMovementRotationMode::LookingDirection:return SwitchStance(MovementConfigData.LookingDirection);
	case ECharacterMovementRotationMode::Aiming:return SwitchStance(MovementConfigData.Aiming);
	}
	return FCharacterMovementSetting();
}

float AQCharacter::GetMappedSpeed()
{
	float Walk = FMath::GetMappedRangeValueClamped<float>(FVector2f(0.0f, CurrentMovementSetting.WalkSpeed ),
		FVector2f(0.0f, 1.0f),Speed);
	float Run = FMath::GetMappedRangeValueClamped<float>(FVector2f(CurrentMovementSetting.WalkSpeed,CurrentMovementSetting.RunSpeed),
		FVector2f(1.0f, 2.0f), Speed);
	float Sprint = FMath::GetMappedRangeValueClamped<float>(FVector2f(CurrentMovementSetting.RunSpeed,CurrentMovementSetting.SprintSpeed ),
		FVector2f(2.0f, 3.0f ), Speed);
	if (Speed > CurrentMovementSetting.RunSpeed)return Sprint;
	if (Speed > Walk)return Run;
	return Walk;
}

bool AQCharacter::CanSprint()
{
	auto CanLookingDirectionSprint = [this] {
		UCharacterMovementComponent* MovementComponent = GetCharacterMovement();
		FRotator CurrentAcclerationRotation = MovementComponent->GetCurrentAcceleration().Rotation();
		FRotator ControlRoattion = GetControlRotation();
		float Delta = FMath::FindDeltaAngleDegrees(ControlRoattion.Yaw, CurrentAcclerationRotation.Yaw);
		if (fabs(Delta) < 50.f && MovementInputAmount > 0.9f)return true;
		return false;
	};
	if (!HasMovementInput) return false;
	switch (MovementRotationMode)
	{
	case ECharacterMovementRotationMode::VelocityDirection:return MovementInputAmount > 0.9f ? true : false;
	case ECharacterMovementRotationMode::LookingDirection:return CanLookingDirectionSprint();
	case ECharacterMovementRotationMode::Aiming:return false;
	}
	return false;
}

void AQCharacter::OnMove(const FInputActionValue& Value)
{

	FVector2D  MoveMent = Value.Get<FVector2D>();
	AddMovementInput(GetActorForwardVector(), MoveMent.X);
	AddMovementInput(GetActorRightVector(), MoveMent.Y);
}

void AQCharacter::OnLook(const FInputActionValue& Value)
{
	FVector2D  LookMent = Value.Get<FVector2D>();
	AddControllerYawInput(LookMent.X);
	AddControllerPitchInput(LookMent.Y);
}

void AQCharacter::SetMovementModel()
{
	if (MovementDataHandle.IsNull())return;
	if (FCharacterMovementSettingState* Data = MovementDataHandle.GetRow<FCharacterMovementSettingState>(nullptr)) {
		MovementConfigData = *Data;
	}
}

void AQCharacter::OnZoom(const FInputActionValue& Value)
{
	
}

void AQCharacter::OnCrouch(const FInputActionValue& Value)
{
	if (MovementAction == ECharacterMovementAction::None) {
		if (MovementState == ECharacterMovementState::OnGround) {
			if (MovementStance == ECharacterMovementStance::Stance) {
				Crouch();
			}
			if (MovementStance == ECharacterMovementStance::Crouching) {
				UnCrouch();
			}
		}
	}
}

void AQCharacter::OnSpring(const FInputActionValue& Value)
{
	DesiredGit = ECharacterMovementGait::Sprint;
}

void AQCharacter::OnStopSpring(const FInputActionValue& Value)
{
	DesiredGit = ECharacterMovementGait::Run;
}

void AQCharacter::OnMovementStateChanged(ECharacterMovementState NewState)
{
	ECharacterMovementState PreState = MovementState;
	MovementState = NewState;
	switch (MovementState)
	{
	case ECharacterMovementState::None:
		break;
	case ECharacterMovementState::InAir:
		break;
	case ECharacterMovementState::OnGround:
		break;
	default:
		break;
	}
}

void AQCharacter::OnMovementActionChanged(ECharacterMovementAction NewAction)
{
}

void AQCharacter::OnRotationModeChanged(ECharacterMovementRotationMode NewRotationMode)
{
}

void AQCharacter::OnGaitChanged(ECharacterMovementGait NewGait)
{
	MovementGait = NewGait;
}

void AQCharacter::OnViewModeChanged(ECharacterViewMode NewViewMode)
{
}

void AQCharacter::OnOverlayStateChanged(ECharacterOverlayState NewOverlayState)
{
}

void AQCharacter::OnStanceChanged(ECharacterMovementStance NewStance)
{
	MovementStance = NewStance;
}

void AQCharacter::OnBreakFall()
{
}

void AQCharacter::UpdateDynamicMovementSettings(ECharacterMovementGait AllowedGait)
{
	CurrentMovementSetting = GetTargetMovementSettings();
	UCharacterMovementComponent* MovementComponent = GetCharacterMovement();
	float CurrentConfigSpeed = 0.0f;
	switch (AllowedGait)
	{
	case ECharacterMovementGait::Walk:CurrentConfigSpeed = CurrentMovementSetting.WalkSpeed;
		break;
	case ECharacterMovementGait::Run:CurrentConfigSpeed = CurrentMovementSetting.RunSpeed;
		break;
	case ECharacterMovementGait::Sprint:CurrentConfigSpeed = CurrentMovementSetting.SprintSpeed;
		break;
	}
	MovementComponent->MaxWalkSpeed = CurrentConfigSpeed;
	MovementComponent->MaxWalkSpeedCrouched = CurrentConfigSpeed;

	FVector Data = CurrentMovementSetting.MovementCurve->GetVectorValue(GetMappedSpeed());
	MovementComponent->MaxAcceleration = Data.X;
	MovementComponent->BrakingDecelerationWalking = Data.Y;
	MovementComponent->GroundFriction = Data.Z;//摩擦力

}



