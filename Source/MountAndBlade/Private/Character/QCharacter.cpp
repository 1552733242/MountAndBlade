// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/QCharacter.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Curves/CurveVector.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/KismetMathLibrary.h"
#include "Animation/QAnimInstance.h"
#define Message(key,...) GEngine->AddOnScreenDebugMessage(key, 1, FColor::Red,FString::Format(TEXT("{0}"), { FStringFormatArg(##__VA_ARGS__)}));
#define Message2(key,Arg1,Arg2) GEngine->AddOnScreenDebugMessage(key, 1, FColor::Red,FString::Format(TEXT("{0}:{1}"), { FStringFormatArg(Arg1),FStringFormatArg(Arg2)}));




AQCharacter::AQCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
	BindDeclares();
	bUseControllerRotationRoll = false;
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	if (GetMovementComponent())
	{
		GetMovementComponent()->GetNavAgentPropertiesRef().bCanCrouch = true;
	}
}

void AQCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	GetMesh()->AddTickPrerequisiteActor(this);
	MainAnimInstacne = Cast<UQAnimInstance>(GetMesh()->GetAnimInstance());
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
	RollEvent.AddUObject(this, &AQCharacter::OnRollEvent);
}

void AQCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	SetEssentialValues();
	CacheValues();
	if (MovementState == ECharacterMovementState::OnGround) {
		UpdateCharacterMovement();
		UpdateGroudedRotation();
	}
	else if (MovementState == ECharacterMovementState::InAir) {
		UpdateInAirRotation();
	}

}

void AQCharacter::SetMovementModel()
{
	if (MovementDataHandle.IsNull())return;
	if (FCharacterMovementSettingState* Data = MovementDataHandle.GetRow<FCharacterMovementSettingState>(nullptr)) {
		MovementConfigData = *Data;
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
		MainAnimInstacne->JumpEvent.Broadcast();
	}
}

void AQCharacter::GetCurrentStates(FCharacterMovementCurrentStates& Info)
{
	Info.MovementState = MovementState;
	Info.PrevMovementState = PreviousMovementState;
	Info.MovementAction = MovementAction;
	Info.RotationMode = MovementRotationMode;
	Info.ActualGait = MovementGait;
	Info.ActualStance = MovementStance;
	Info.ViewMode = ViewMode;
	Info.OverlayState = OverlayState;
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
	if (MovementAction == ECharacterMovementAction::None) {
		if (CanUpdateMovingRotation()) {
			if (MovementRotationMode == ECharacterMovementRotationMode::VelocityDirection) {
				FRotator Target = { 0,LastVelocityRotation.Yaw,0 };
				SmoothCharacterRotation(Target, 800.f, CalcuateGroundRotationRate());
			}
			else if (MovementRotationMode == ECharacterMovementRotationMode::LookingDirection) {
				if (MovementGait == ECharacterMovementGait::Sprint) {
					FRotator Target = { 0,LastVelocityRotation.Yaw,0 };
					SmoothCharacterRotation(Target, 500.f, CalcuateGroundRotationRate());
				}
				else {
					FRotator Target = { 0,GetControlRotation().Yaw + GetAnimCurveValue(TEXT("YawOffset")),0 };
					SmoothCharacterRotation(Target, 500.f, CalcuateGroundRotationRate());
				}

			}
			else if (MovementRotationMode == ECharacterMovementRotationMode::Aiming) {
				FRotator Target = { 0,GetControlRotation().Yaw ,0 };
				SmoothCharacterRotation(Target, 1000.f, 20.f);
			}
		}
		else {
			//Not Moving
			if (ViewMode == ECharacterViewMode::ThirdPerson) {
				if (MovementRotationMode == ECharacterMovementRotationMode::Aiming) {
					LimitRotation(-100.f, 100.f, 20.f);
				}
				float RotationAmount = GetAnimCurveValue(TEXT("RotationAmount"));
				if (fabs(RotationAmount) > 0.001f) {
					RotationAmount = RotationAmount * UGameplayStatics::GetWorldDeltaSeconds(GetWorld()) * 30.f;
					AddActorWorldRotation(FRotator(0, RotationAmount, 0));
					TargetRotation = GetActorRotation();
				}
			}
			else {
				LimitRotation(-100.f, 100.f, 20.f);
			}
		}
	}
	else if (MovementAction == ECharacterMovementAction::Rolling) {
		if (HasMovementInput) {
			FRotator Target = { 0,LastMovementInputRotation.Yaw,0 };
			SmoothCharacterRotation(Target, 0.f, 2.f);
		}
	}

}

void AQCharacter::UpdateInAirRotation()
{
	if (MovementRotationMode == ECharacterMovementRotationMode::Aiming) {
		FRotator Target = FRotator(0, GetControlRotation().Yaw, 0);
		SmoothCharacterRotation(Target, 0.0f, 15.0f);
		InAirRotation = GetActorRotation();
	}
	else {
		FRotator Target = FRotator(0, InAirRotation.Yaw, 0);
		SmoothCharacterRotation(Target, 0.0f, 5.0f);
	}
}

bool AQCharacter::CanUpdateMovingRotation()
{
	if ((IsMoving && HasMovementInput)||Speed>150.f) {
		if (!HasAnyRootMotion()) {
			return true;
		}
	}
	return false;
}

void AQCharacter::RagdollStart()
{
	GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_None);
}

void AQCharacter::RagdollEnd()
{
}

void AQCharacter::RagdollUpdate()
{
}

void AQCharacter::SetActorLocationDuringRagdoll()
{
}

void AQCharacter::GetGetUpAnimation()
{
}

void AQCharacter::SmoothCharacterRotation(const FRotator& Target, float TargetInterpSpeed, float ActorInterpSpeed)
{
	TargetRotation =  FMath::RInterpTo(TargetRotation, Target, UGameplayStatics::GetWorldDeltaSeconds(GetWorld()), TargetInterpSpeed);
	FRotator NowRotaion = FMath::RInterpTo(GetActorRotation(), TargetRotation, UGameplayStatics::GetWorldDeltaSeconds(GetWorld()), ActorInterpSpeed);
	SetActorRotation(NowRotaion);
}

float AQCharacter::CalcuateGroundRotationRate()
{
	float ActorRotatorSpeed = CurrentMovementSetting.RotationRateCurve->GetFloatValue(GetMappedSpeed());
	float AimRotatorSpeed = FMath::GetMappedRangeValueClamped(TRange<float>(0.f, 300.f), TRange<float>(1.f, 3.f), AimYawRate);
	return AimRotatorSpeed * ActorRotatorSpeed;
}

void AQCharacter::LimitRotation(float AimYawMin, float AimYawMax, float InterpSpeed)
{
	

	//瞄准夹角到达一定程度开始旋转
	float AngleControllAndActor = FMath::FindDeltaAngleDegrees(GetActorRotation().Yaw, GetControlRotation().Yaw);
	if (AngleControllAndActor < AimYawMin || AngleControllAndActor > AimYawMax) {
		FRotator Target;
		if (AngleControllAndActor > 0.0f) {
			Target = { 0,GetControlRotation().Yaw + AimYawMin,0 };
		}
		else {
			Target = { 0,GetControlRotation().Yaw + AimYawMax,0 };
		}
		//插值速度为0表示目标角度不再插值，仅仅插值真实旋转速度
		SmoothCharacterRotation(Target, 0.0f, InterpSpeed);
	}
}

bool AQCharacter::SetActorLocationAndRotationAndUpdateTargetRotation(const FVector& NewLocation, const FRotator& NewRotation, 
	bool Sweep, FHitResult& SweepHitResult, bool Teleport)
{
	TargetRotation = NewRotation;
	return SetActorLocationAndRotation(NewLocation, NewRotation, Sweep, (Sweep ? &SweepHitResult : nullptr), TeleportFlagToEnum(Teleport));
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

float AQCharacter::GetAnimCurveValue(const FName& CurveName)
{
	if (MainAnimInstacne) {
		return MainAnimInstacne->GetCurveValue(CurveName);
	}
	return 0.0f;
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
		float Delta = FMath::FindDeltaAngleDegrees(CurrentAcclerationRotation.Yaw, ControlRoattion.Yaw);
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
	MovementRotationMode = NewRotationMode;
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
	OverlayState = NewOverlayState;
}

void AQCharacter::OnStanceChanged(ECharacterMovementStance NewStance)
{
	MovementStance = NewStance;
}

void AQCharacter::OnBreakFall()
{
	if (MainAnimInstacne && RollAnimMontage) {
		MainAnimInstacne->Montage_Play(RollAnimMontage, 1.35, EMontagePlayReturnType::MontageLength);
	}
}

void AQCharacter::OnRollEvent()
{
	if (MainAnimInstacne && RollAnimMontage) {
		MainAnimInstacne->Montage_Play(RollAnimMontage, 1.15, EMontagePlayReturnType::MontageLength);
	}
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

void AQCharacter::Jump()
{
	if (MovementAction == ECharacterMovementAction::None) {
		if (MovementState == ECharacterMovementState::Ragdoll) {

		}
		else if (MovementState == ECharacterMovementState::Mantling) {
			//No Action
		}
		else {
			if (MovementState == ECharacterMovementState::OnGround) {
				if (HasMovementInput) {
					if (MovementStance == ECharacterMovementStance::Stance) {
						Super::Jump();
					}
					else {
						Super::UnCrouch();
					}
				}
				else {
					if (MovementStance == ECharacterMovementStance::Stance) {
						Super::Jump();
					}
					else {
						Super::UnCrouch();
					}
				}
			}
			else if (MovementState == ECharacterMovementState::InAir) {

			}
		}
	}
}

void AQCharacter::StopJumping()
{
	Super::StopJumping();
}

void AQCharacter::Aim()
{
	SetRotation.Broadcast(ECharacterMovementRotationMode::Aiming);
}

void AQCharacter::StopAim()
{
	if (ViewMode == ECharacterViewMode::ThirdPerson) {
		SetRotation.Broadcast(DesiredRotationMode);
	}
	else {
		SetRotation.Broadcast(ECharacterMovementRotationMode::LookingDirection);
	}
}

void AQCharacter::Sprint()
{
	DesiredGit = ECharacterMovementGait::Sprint;
}

void AQCharacter::StopSprint()
{
	DesiredGit = ECharacterMovementGait::Run;
}

void AQCharacter::Roll()
{
	
	RollEvent.Broadcast();
}

void AQCharacter::Ragdoll()
{

}

void AQCharacter::ChangeMovementStance()
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
