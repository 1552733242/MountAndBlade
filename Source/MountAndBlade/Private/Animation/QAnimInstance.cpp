// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/QAnimInstance.h"
#include "Character/QCharacter.h"
#include "Character/CharacterEnum.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Curves/CurveVector.h"
#define Message(key,...) GEngine->AddOnScreenDebugMessage(key, 1, FColor::Red,FString::Format(TEXT("{0}"), { FStringFormatArg(##__VA_ARGS__)}));
#define Message2(key,Arg1,Arg2) GEngine->AddOnScreenDebugMessage(key, 1, FColor::Red,FString::Format(TEXT("{0}:{1}"), { FStringFormatArg(Arg1),FStringFormatArg(Arg2)}));



UQAnimInstance::UQAnimInstance()
{
	BindDeclares();
}

void UQAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);
	DeltaTimeX = DeltaSeconds;
	if (DeltaTimeX != 0.0f && Character) {
		UpdateCharacterInfo();
		UpdateAimingValues();
		UpdateLayerValues();
		UpDateFootIK();
		switch (MovementState)
		{
		case ECharacterMovementState::InAir:
			break;
		case ECharacterMovementState::OnGround:UpdateOnGround();
			break;
		}
	}
}

void UQAnimInstance::NativeInitializeAnimation()
{
	if (APawn* Owner = TryGetPawnOwner()) {
		Character = Cast<AQCharacter>(Owner);
	}
}

void UQAnimInstance::UpdateMovementValues()
{
	
	VelocityBlend = InterpVelocityBlend(VelocityBlend, CalculateVelocityBlend(), 12.0f, DeltaTimeX);

	DiagonalScaleAmount = CalculateDiagonalScaleAmount();
	
	RelativeAccelerationAmount = CalculateRelativeAccelerationAmount();

	FLeanAmount NewLeanAmount = { RelativeAccelerationAmount.Y ,RelativeAccelerationAmount.X };

	LeanAmount = InterpLeanAmount(LeanAmount, NewLeanAmount, 4.0f, DeltaTimeX);
	WalkRunBlend = CalculateWalkRunBlend();
	StrideBlend = CalculateStrideBlend();
	StandingPlayRate = CalculateStandingPlayRate();
	CrouchingPlayRate = CalculateCrouchingPlayRate();


}

void UQAnimInstance::UpdateAimingValues()
{
	//平滑过度的瞄准方向
	SmoothedAimingRotation = FMath::RInterpTo(SmoothedAimingRotation, AimRotation, DeltaTimeX, SmoothedAimingRotationInterpSpeed);

	FRotator CharacterRotation = Character->GetActorRotation();
	AimingAngle = { FMath::FindDeltaAngleDegrees(CharacterRotation.Yaw,AimRotation.Yaw),
		FMath::FindDeltaAngleDegrees(CharacterRotation.Pitch,AimRotation.Pitch) };
	SmoothedAimingAngle = { FMath::FindDeltaAngleDegrees(CharacterRotation.Yaw,SmoothedAimingRotation.Yaw),
		FMath::FindDeltaAngleDegrees(CharacterRotation.Pitch,SmoothedAimingRotation.Pitch) };

	if (RotationMode != ECharacterMovementRotationMode::VelocityDirection) {
		AimSweepTime = FMath::GetMappedRangeValueClamped(TRange<double>(-90.0, 90.0), TRange<double>(1.0,0.0), AimingAngle.Y);
		//(源注释)Use the Aiming Yaw Angle divided by the number of spine+pelvis bones to get the amount of spine rotation needed to remain facing the camera direction.
		SpineRotation.Yaw = AimingAngle.X / 4.0;
	}
	else {
		if (HasMovementInput) {
			FRotator InputDir = MovementInput.Rotation();
			float InputCharacterAngle = FMath::FindDeltaAngleDegrees( CharacterRotation.Yaw, InputDir.Yaw);
			InputCharacterAngle = FMath::GetMappedRangeValueClamped(TRange<double>(-180.0, 180.0), TRange<double>(0.0, 1.0), AimingAngle.Y);
			InputYawOffsetTime = FMath::FInterpTo(InputYawOffsetTime, InputCharacterAngle, DeltaTimeX, InputYawOffsetInterpSpeed);
		}
	}
	LeftYawTime = FMath::GetMappedRangeValueClamped(TRange<double>(0, 180.0), TRange<double>(0.5, 0.0), fabs(AimingAngle.X));
	RightYawTime = FMath::GetMappedRangeValueClamped(TRange<double>(0, 180.0), TRange<double>(0.5, 1.0), fabs(AimingAngle.X));
	ForwardYawTime = FMath::GetMappedRangeValueClamped(TRange<double>(-180.0, 180.0), TRange<double>(0.0, 1.0), AimingAngle.X);

}

void UQAnimInstance::UpdateLayerValues()
{
	Enable_AimOffset = FMath::Lerp(1.0, 0.0, GetCurveValue(TEXT("Mask_AimOffset")));
	
	BasePose_N = GetCurveValue(TEXT("BasePose_N"));
	BasePose_CLF = GetCurveValue(TEXT("BasePose_CLF"));

	Spine_Add = GetCurveValue(TEXT("Layering_Spine_Add"));
	Head_Add = GetCurveValue(TEXT("Layering_Head_Add"));
	Arm_L_Add = GetCurveValue(TEXT("Layering_Arm_L_Add"));
	Arm_R_Add = GetCurveValue(TEXT("Layering_Arm_R_Add"));

	Hand_R = GetCurveValue(TEXT("Layering_Hand_R"));
	Hand_L = GetCurveValue(TEXT("Layering_Hand_L"));

	Enable_HandIK_L = FMath::Lerp(0.0, GetCurveValue(TEXT("Enable_HandIK_L")), GetCurveValue(TEXT("Layering_Arm_L")));
	Enable_HandIK_R = FMath::Lerp(0.0, GetCurveValue(TEXT("Enable_HandIK_R")), GetCurveValue(TEXT("Layering_Arm_R")));

	Arm_L_LS = GetCurveValue(TEXT("Layering_Arm_L_LS"));
	Arm_L_MS = 1 - FMath::Floor(Arm_L_LS);
	Arm_R_LS = GetCurveValue(TEXT("Layering_Arm_R_LS"));
	Arm_R_MS = 1 - FMath::Floor(Arm_R_LS);
}

void UQAnimInstance::UpdateRotationValues()
{
	MovementDirection =  CalculateMovementDirection();
	float CharacterAndVelocityAngle = FMath::FindDeltaAngleDegrees(Character->GetControlRotation().Yaw, Velocity.Rotation().Yaw);
	//Message2(0,"CharacterAndVelocityAngle", CharacterAndVelocityAngle);
	FYaw = YawOffsetFB->GetVectorValue(CharacterAndVelocityAngle).X;
	BYaw = YawOffsetFB->GetVectorValue(CharacterAndVelocityAngle).Y;
	LYaw = YawOffsetLR->GetVectorValue(CharacterAndVelocityAngle).X;
	RYaw = YawOffsetLR->GetVectorValue(CharacterAndVelocityAngle).Y;
}

FVelocityBlend UQAnimInstance::InterpVelocityBlend(const FVelocityBlend& Current, const FVelocityBlend& Target, float InterpSpeed, float DeltTime)
{
	FVelocityBlend InterpedVelocityBlend;
	InterpedVelocityBlend.VelocityBlendF = FMath::FInterpTo(Current.VelocityBlendF, Target.VelocityBlendF, DeltTime, InterpSpeed);
	InterpedVelocityBlend.VelocityBlendB = FMath::FInterpTo(Current.VelocityBlendB, Target.VelocityBlendB, DeltTime, InterpSpeed);
	InterpedVelocityBlend.VelocityBlendL = FMath::FInterpTo(Current.VelocityBlendL, Target.VelocityBlendL, DeltTime, InterpSpeed);
	InterpedVelocityBlend.VelocityBlendR = FMath::FInterpTo(Current.VelocityBlendR, Target.VelocityBlendR, DeltTime, InterpSpeed);
	return InterpedVelocityBlend;
}

void UQAnimInstance::UpdateOnGround()
{
	bool NewShouldMove = ShouldMoveCheck();
	if (!ShouldMove && NewShouldMove) {
		ElapsedDelayTime = 0;
		RotateL = false;
		RotateR = false;
	}
	ShouldMove = NewShouldMove;
	//Moving
	if (ShouldMove) {
		UpdateMovementValues();
		UpdateRotationValues();
	}
	//No Moving
	else {
		
		
		if (CanRotateInPlace()) {
			RotateInPlaceCheck();
		}
		else {
			RotateL = false;
			RotateR = false;
		}
		if (CanTurnInPlace()) {
			TurnInPlaceCheck();
		}
		else {
			ElapsedDelayTime = 0;
		}

	}

}

void UQAnimInstance::UpdateCharacterInfo()
{
	FCharacterMovementCurrentStates CurrentStatesInfo;
	Character->GetCurrentStates(CurrentStatesInfo);
	MovementState = CurrentStatesInfo.MovementState;
	PreviousMovementState = CurrentStatesInfo.PrevMovementState;
	RotationMode = CurrentStatesInfo.RotationMode;
	MovementGait = CurrentStatesInfo.ActualGait;
	MovementStance = CurrentStatesInfo.ActualStance;
	ViewMode = CurrentStatesInfo.ViewMode;
	OverlayState = CurrentStatesInfo.OverlayState;
	FCharacterMovementEssentialValues EssentialValuesInfo;
	Character->GetEssentialValues(EssentialValuesInfo);
	Velocity = EssentialValuesInfo.Velocity;
	Acceleration = EssentialValuesInfo.Acceleration;
	MovementInput = EssentialValuesInfo.MovementInput;
	IsMoving = EssentialValuesInfo.IsMoving;
	HasMovementInput = EssentialValuesInfo.HasMovementInput;
	Speed = EssentialValuesInfo.Speed;
	MovementInputAmount = EssentialValuesInfo.MovementInputAmount;
	AimRotation = EssentialValuesInfo.AimingRotation;
	AimYawRate = EssentialValuesInfo.AimYawRate;
}

void UQAnimInstance::UpDateFootIK()
{
	
	SetFootLocking(TEXT("Enable_FootIK_L"), TEXT("FootLock_L"), TEXT("ik_foot_l"),
		FootLock_L_Alplha, FootLock_L_Location, FootLock_L_Rotation);//设置左脚Location Rotation
	SetFootLocking(TEXT("Enable_FootIK_R"), TEXT("FootLock_R"), TEXT("ik_foot_r"),
		FootLock_R_Alplha, FootLock_R_Location, FootLock_R_Rotation);
	
	if (MovementState == ECharacterMovementState::InAir) {
		SetPelvisIKOffset(FVector(0, 0, 0), FVector(0, 0, 0));

	}
	else {
		FVector FootOffset_L_Target, FootOffset_R_Target;
		SetFootOffsets(TEXT("Enable_FootIK_L"), TEXT("ik_foot_l"), TEXT("root"),
			FootOffset_L_Target, FootOffset_L_Location, FootOffset_L_Rotation);
		SetFootOffsets(TEXT("Enable_FootIK_R"), TEXT("ik_foot_r"), TEXT("root"),
			FootOffset_R_Target, FootOffset_R_Location, FootOffset_R_Rotation);
		SetPelvisIKOffset(FootOffset_L_Target, FootOffset_R_Target);
		ResetIKOffsets();
	}
}
void UQAnimInstance::TurnInPlaceCheck()
{
	if (fabs(AimingAngle.X) > TurnCheckMinAngle && AimYawRate < AimYawRateLimit) {
		ElapsedDelayTime += DeltaTimeX;
	}
	else {
		ElapsedDelayTime = 0.0f;
		return;
	}
	float ElapsedDelayTimeLimit = FMath::GetMappedRangeValueClamped(TRange<double>(TurnCheckMinAngle, 180.0),
		TRange<double>(MinAngleDelay, MaxAngleDelay), fabs(AimingAngle.X));

	if (ElapsedDelayTime > ElapsedDelayTimeLimit) {
		//Message(1, AimRotation.Yaw);
		TurninPlace(FRotator(0.0, AimRotation.Yaw, 0.0), 1.0, 0.0, false);
	}

}
void UQAnimInstance::TurninPlace(const FRotator& TargetRotation, float PlayRateScale, float StartTime, bool OverrideCurrent)
{
	FRotator CharacterRotation = Character->GetActorRotation();
	float TurnAngle = FMath::FindDeltaAngleDegrees(CharacterRotation.Yaw, TargetRotation.Yaw);
	FTurnInPlaceAsset TargetTurnAsset;
	if (fabs(TurnAngle)< Turn180Threshold){
		if (TurnAngle < 0) {
			switch (MovementStance)
			{
			case ECharacterMovementStance::Stance:TargetTurnAsset = N_TurnIP_L90; break;
			case ECharacterMovementStance::Crouching:TargetTurnAsset = CLF_TurnIP_L90; break;
			}
		}
		else {
			switch (MovementStance)
			{
			case ECharacterMovementStance::Stance:TargetTurnAsset = N_TurnIP_R90; break;
			case ECharacterMovementStance::Crouching:TargetTurnAsset = CLF_TurnIP_R90; break;
			}
		}
	}
	else {
		if (TurnAngle < 0) {
			switch (MovementStance)
			{
			case ECharacterMovementStance::Stance:TargetTurnAsset = N_TurnIP_L180; break;
			case ECharacterMovementStance::Crouching:TargetTurnAsset = CLF_TurnIP_L180; break;
			}
		}
		else {
			switch (MovementStance)
			{
			case ECharacterMovementStance::Stance:TargetTurnAsset = N_TurnIP_R180; break;
			case ECharacterMovementStance::Crouching:TargetTurnAsset = CLF_TurnIP_R180; break;
			}
		}
	}
	if (!IsPlayingSlotAnimation(TargetTurnAsset.Animation, TargetTurnAsset.SlotName) || OverrideCurrent) {

		PlaySlotAnimationAsDynamicMontage(TargetTurnAsset.Animation, TargetTurnAsset.SlotName,
			0.2, 0.2, PlayRateScale * TargetTurnAsset.PlayRate, 1, 0.0f, StartTime);
		if (TargetTurnAsset.ScaleTurnAngle) {
			RotationScale = TurnAngle / TargetTurnAsset.AnimatedAngle * TargetTurnAsset.PlayRate * PlayRateScale;
			//根据角度判定播放速度
		}
		else {
			
			RotationScale = TargetTurnAsset.PlayRate * PlayRateScale;
			//播放速度
		}
	}
}

float UQAnimInstance::CalculateWalkRunBlend()
{
	switch (MovementGait)
	{
	case ECharacterMovementGait::Walk:		return 0.0f;
	case ECharacterMovementGait::Run:		return 1.0f;
	case ECharacterMovementGait::Sprint:	return 1.0f;
	}	
	return 0.0f;
}

float UQAnimInstance::CalculateStrideBlend()
{
	float WalkBlend = StrideBlendNWalk->GetFloatValue(Speed);
	float RunBlend  = StrideBlendNRun->GetFloatValue(Speed);
	float StandBlend = FMath::Lerp(WalkBlend, RunBlend, GetAnimCurveClamped(TEXT("Weight_Gait"), -1.0f, 0.0, 1.0));
	float CWalkBlend = StrideBlendCWalk->GetFloatValue(Speed);
	float Blend = FMath::Lerp(StandBlend, CWalkBlend, GetCurveValue(TEXT("BasePose_CLF")));
	return Blend;
}

float UQAnimInstance::CalculateStandingPlayRate()
{
	float NoSprintPlayRate = FMath::Lerp(Speed / AnimatedWalkSpeed, Speed / AnimatedRunSpeed, GetAnimCurveClamped(TEXT("Weight_Gait"), -1, 0.0f, 1.0f));
	float SprintPlayRate = Speed / AnimatedSprintSpeed;
	float PlayRate = FMath::Lerp(NoSprintPlayRate, SprintPlayRate, GetAnimCurveClamped(TEXT("Weight_Gait"), -2, 0.0f, 1.0f));
	PlayRate /= StrideBlend;
	PlayRate /= GetOwningComponent()->GetComponentScale().Z;
	return FMath::Clamp(PlayRate, 0.0f, 3.0f);
}

float UQAnimInstance::CalculateCrouchingPlayRate()
{
	float Temp = Speed / AnimatedCrouchSpeed;
	Temp = Temp / StrideBlend;
	Temp = Temp / GetOwningComponent()->GetComponentScale().Z;
	return	FMath::Clamp(Temp, 0.0f, 2.0f);
}

float UQAnimInstance::CalculateDiagonalScaleAmount()
{
	float Time = fabs(VelocityBlend.VelocityBlendF + VelocityBlend.VelocityBlendB);//F 和 B同时只会有一个有值
	
	return DiagonalScaleAmountCurve->GetFloatValue(Time);
}

bool UQAnimInstance::ShouldMoveCheck()
{
	if ((HasMovementInput && IsMoving) || Speed > 150.f)return true;
	return false;
}

bool UQAnimInstance::CanTurnInPlace()
{
	if (RotationMode == ECharacterMovementRotationMode::LookingDirection &&
		ViewMode == ECharacterViewMode::ThirdPerson && 
		GetCurveValue(TEXT("Enable_Transition"))) {
		return true;
	}
	return false;
}

bool UQAnimInstance::CanRotateInPlace()
{
	return RotationMode == ECharacterMovementRotationMode::Aiming || ViewMode == ECharacterViewMode::FirstPerson;
}



bool UQAnimInstance::AngleInRange(float Angle, float MinAngle, float MaxAngle, float Buffer, bool IncreaseBuffer)
{

	if (IncreaseBuffer) {
		return Angle >= MinAngle - Buffer && Angle <= MaxAngle + Buffer;
	}
	else {
		return Angle >= MinAngle + Buffer && Angle <= MaxAngle - Buffer;
	}

}

bool UQAnimInstance::CanDynamicTransition()
{
	return false;
}


float UQAnimInstance::GetAnimCurveClamped(FName name, float Bias, float ClampMin, float ClampMax)
{
	float CurveValue = GetCurveValue(TEXT("Weight_Gait"));
	return FMath::Clamp(CurveValue + Bias, ClampMin, ClampMax);
}

FLeanAmount UQAnimInstance::InterpLeanAmount(const FLeanAmount& Current, FLeanAmount& Target, float InterpSpeed, float DeltaTime)
{
	return { FMath::FInterpTo(Current.LR, Target.LR, DeltaTime, InterpSpeed),FMath::FInterpTo(Current.BF, Target.BF, DeltaTime, InterpSpeed) };
}

void UQAnimInstance::RotateInPlaceCheck()
{
	RotateL = AimingAngle.X < RotateMinThreshold;
	RotateR = AimingAngle.X > RotateMaxThreshold;
	if (RotateL || RotateR) {
		RotateRate = FMath::GetMappedRangeValueClamped(TRange<float>(AimYawRateMinRange,AimYawRateMaxRange), 
			TRange<float>(MinPlayRate,MaxPlayRate), AimYawRate);
	}
}

EMovementDirection UQAnimInstance::CalculateMovementDirection()
{
	auto CalculateMovementDirectionWhenWalkOrRunning = [this]() {
		auto LookingAiming = [this]() {
			float Angle = FMath::FindDeltaAngleDegrees( AimRotation.Yaw, Velocity.Rotation().Yaw);
			return CalculateQuadrant(MovementDirection, 70.f, -70.f, 110.f, -110.f, 5.f, Angle);
			};
		switch (RotationMode)
		{
		case ECharacterMovementRotationMode::VelocityDirection:	return EMovementDirection::Forward;
		case ECharacterMovementRotationMode::LookingDirection:return LookingAiming();
		case ECharacterMovementRotationMode::Aiming:return LookingAiming();
		}
		return EMovementDirection();
		};
	switch (MovementGait)
	{
	case ECharacterMovementGait::Walk:return CalculateMovementDirectionWhenWalkOrRunning();
	case ECharacterMovementGait::Run:return CalculateMovementDirectionWhenWalkOrRunning();
	case ECharacterMovementGait::Sprint:return EMovementDirection::Forward;
	}
	return EMovementDirection();
}

EMovementDirection UQAnimInstance::CalculateQuadrant(EMovementDirection Current, float FRThreshold, float FLThreshold, float BRThreshold, float BLThreshold, float Buffer, float Angle)
{
	bool Temp = true;//原有逻辑有毛病，都按ture处理
	if (AngleInRange(Angle, FLThreshold, FRThreshold, Buffer, Temp)) {
		return EMovementDirection::Forward;
	}
	else if (AngleInRange(Angle, FRThreshold, BRThreshold, Buffer, Temp)) {
		return EMovementDirection::Right;

	}
	else if (AngleInRange(Angle, BLThreshold, FLThreshold, Buffer, Temp)) {
		return EMovementDirection::Left;

	}
	else {
		return EMovementDirection::Backward;
	}
}

FVector UQAnimInstance::CalculateRelativeAccelerationAmount()
{
	FVector CRelativeAccelerationAmount;
	FRotator CharacterRotation = Character->GetActorRotation();
	if (Acceleration.Dot(Velocity) > 0.0f) {//< 90
		float MaxAcceleration = Character->GetCharacterMovement()->GetMaxAcceleration();
		FVector AccelerationAmount = Acceleration.GetClampedToMaxSize(MaxAcceleration) / MaxAcceleration;
		CRelativeAccelerationAmount = CharacterRotation.UnrotateVector(AccelerationAmount);
		return CRelativeAccelerationAmount;
	}
	else {
		float MaxBrakingDeceleration = Character->GetCharacterMovement()->GetMaxBrakingDeceleration();
		FVector AccelerationAmount = Acceleration.GetClampedToMaxSize(MaxBrakingDeceleration) / MaxBrakingDeceleration;
		CRelativeAccelerationAmount = CharacterRotation.UnrotateVector(AccelerationAmount);
		return CRelativeAccelerationAmount;
	}
}

FVelocityBlend UQAnimInstance::CalculateVelocityBlend()
{
	FVector VelocityDir = Velocity.GetSafeNormal();
	FRotator CharacterDir = Character->GetActorRotation();
	FVector LocRelativeVelocityDir = CharacterDir.UnrotateVector(VelocityDir);

	float Sum = fabs(LocRelativeVelocityDir.X) +
		fabs(LocRelativeVelocityDir.Y) +
		fabs(LocRelativeVelocityDir.Z);

	FVector RelativeDirection = LocRelativeVelocityDir / Sum;
	FVelocityBlend CVelocityBlend;
	CVelocityBlend.VelocityBlendF = FMath::Clamp(RelativeDirection.X, 0.0, 1.0);
	CVelocityBlend.VelocityBlendB = FMath::Abs(FMath::Clamp(RelativeDirection.X, -1.0, 0.0));
	CVelocityBlend.VelocityBlendR = FMath::Clamp(RelativeDirection.Y, 0.0, 1.0);
	CVelocityBlend.VelocityBlendL = FMath::Abs(FMath::Clamp(RelativeDirection.Y, -1.0, 0.0));

	return CVelocityBlend;
}

void UQAnimInstance::SetFootOffsets(const FName& EnableFootIKCurve, const FName& IKFootBone, 
	const FName& RootBone, FVector& CurrentLocationTarget, 
	FVector& CurrentLocationOffset, FRotator& CurrentRotationOffset)
{
	if (GetCurveValue(EnableFootIKCurve) > 0.0f) {
		USkeletalMeshComponent* OwnerMesh = GetOwningComponent();
		
		FVector IKFootFloorLocation; 
		IKFootFloorLocation.X = OwnerMesh->GetSocketLocation(IKFootBone).X;
		IKFootFloorLocation.Y = OwnerMesh->GetSocketLocation(IKFootBone).Y;
		IKFootFloorLocation.Z = OwnerMesh->GetSocketLocation(RootBone).Z;
		FVector FootTracBegin = FVector(IKFootFloorLocation.X, IKFootFloorLocation.Y, IKFootFloorLocation.Z+ IK_TraceDistanceAboveFoot);
		FVector FootTracEnd = FVector(IKFootFloorLocation.X, IKFootFloorLocation.Y, IKFootFloorLocation.Z - IK_TraceDistanceBelowFoot);
		FHitResult HitResult;
		UKismetSystemLibrary::LineTraceSingle(this, FootTracBegin,
			FootTracEnd, UEngineTypes::ConvertToTraceType(ECollisionChannel::ECC_Visibility),
			false, TArray<AActor*, FDefaultAllocator>(), EDrawDebugTrace::Type::None, 
			HitResult,true);
		FRotator TargetRotationOffset;
		if (Character->GetCharacterMovement()->IsWalkable(HitResult)) {
			FVector OldFootFloorLocoation = FootHeight * FVector(0, 0, 1) + IKFootFloorLocation;
			FVector NewFootFloorLocoation = FootHeight * HitResult.ImpactNormal + HitResult.ImpactPoint;

			//DrawDebugPoint(GetWorld(), IKFootFloorLocation + FVector(0, 0, FootHeight) , 20.f, FColor::Cyan);
			//DrawDebugLine(GetWorld(), FootTracBegin, FootTracEnd, FColor::Blue);
			//DrawDebugPoint(GetWorld(), HitResult.ImpactPoint, 10.f, FColor::Red);
			CurrentLocationTarget = NewFootFloorLocoation - OldFootFloorLocoation;
			
			Message2(11, "CurrentLocationTarget", CurrentLocationTarget.ToString());
			Message2(12, "IKFootFloorLocation", IKFootFloorLocation.ToString());
			TargetRotationOffset = FRotator(FMath::RadiansToDegrees(FMath::Atan2(HitResult.ImpactNormal.X, HitResult.ImpactNormal.Z)) * -1.f,
				0.f,FMath::RadiansToDegrees(FMath::Atan2(HitResult.ImpactNormal.Y, HitResult.ImpactNormal.Z)));
		}
		if (CurrentLocationOffset.Z > CurrentLocationTarget.Z) {
			CurrentLocationOffset = FMath::VInterpTo(CurrentLocationOffset, CurrentLocationTarget, DeltaTimeX, 30.0f);
		}
		else {
			CurrentLocationOffset = FMath::VInterpTo(CurrentLocationOffset, CurrentLocationTarget, DeltaTimeX, 15.0f);
		}
		CurrentRotationOffset = FMath::RInterpTo(CurrentRotationOffset, TargetRotationOffset, DeltaTimeX, 30.0f);
	}
	else {
		CurrentLocationOffset = FVector(0, 0, 0);
		CurrentRotationOffset = FRotator(0, 0, 0);
	}
}

void UQAnimInstance::SetPelvisIKOffset(const FVector& FootOffset_L_Target,const FVector& FootOffset_R_Target)
{
	PelvisAlpha = (GetCurveValue(TEXT("Enable_FootIK_L")) + GetCurveValue(TEXT("Enable_FootIK_R"))) / 2.0f;
	FVector PelvisTarget;
	if (PelvisAlpha > 0.0f) {
		if (FootOffset_L_Target.Z < FootOffset_R_Target.Z) {
			PelvisTarget = FootOffset_L_Target;
		}
		else {
			PelvisTarget = FootOffset_R_Target;//选定最考上的脚，逆向运动学修改盆骨位置
		}
		if (PelvisTarget.Z > PelvisOffset.Z) {
			PelvisOffset = FMath::VInterpTo(PelvisOffset, PelvisTarget, DeltaTimeX, 10.0f);
		}
		else {
			PelvisOffset = FMath::VInterpTo(PelvisOffset, PelvisTarget, DeltaTimeX, 15.0f);
		}
	}
	else {
		PelvisOffset = FVector(0, 0, 0);
	}
}

void UQAnimInstance::SetFootLocking(const FName& EnableFootIKCurve, const FName& FootLockCurve,
	const FName& IKFootBone, float& CurrentFootLockAlpha,
	FVector& CurrentFootLockLocation, FRotator& CurrentFootLockRotation)
{
	if (GetCurveValue(EnableFootIKCurve) > 0.0f) {
		float FootLockCurveValue = GetCurveValue(FootLockCurve);
		if (FootLockCurveValue >= 0.99f || FootLockCurveValue < CurrentFootLockAlpha) {
			CurrentFootLockAlpha = FootLockCurveValue;
		}
		if (CurrentFootLockAlpha >= 0.99f) {
			FTransform OwnerMeshTransform = GetOwningComponent()->GetSocketTransform(IKFootBone, ERelativeTransformSpace::RTS_Component);
			CurrentFootLockLocation = OwnerMeshTransform.GetLocation();
			CurrentFootLockRotation = OwnerMeshTransform.GetRotation().Rotator();
		}
		if (CurrentFootLockAlpha > 0.0f) {
			SetFootLockOffsets(CurrentFootLockLocation, CurrentFootLockRotation);
		}
	}
}

void UQAnimInstance::SetFootLockOffsets(FVector& LocalLocation,FRotator& LocalRotation)
{
	
	UCharacterMovementComponent* CharacterMovement = Character->GetCharacterMovement();
	FRotator RotationDifference;
	if (CharacterMovement->IsMovingOnGround()) {
		RotationDifference = Character->GetActorRotation() - CharacterMovement->GetLastUpdateRotation();
		RotationDifference.Normalize();
	}

	
	LocalRotation = LocalRotation - RotationDifference;
	LocalRotation.Normalize();

	
	FVector CurrentVelocity = Velocity * UGameplayStatics::GetWorldDeltaSeconds(GetWorld());
	FVector LocationDifference = GetOwningComponent()->GetComponentRotation().UnrotateVector(CurrentVelocity);

	FVector AmountLocation = LocalLocation - LocationDifference;
	AmountLocation.RotateAngleAxis(RotationDifference.Yaw, FVector(0.0, 0.0, -1.0));
	LocalLocation = AmountLocation;
	
	
}

void UQAnimInstance::ResetIKOffsets()
{
	FootOffset_L_Location = FMath::VInterpTo(FootOffset_L_Location, FVector(0, 0, 0), DeltaTimeX, 15.0f);
	FootOffset_R_Location = FMath::VInterpTo(FootOffset_L_Location, FVector(0, 0, 0), DeltaTimeX, 15.0f);
	FootOffset_L_Rotation = FMath::RInterpTo(FootOffset_L_Rotation, FRotator(0, 0, 0), DeltaTimeX, 15.0f);
	FootOffset_R_Rotation = FMath::RInterpTo(FootOffset_R_Rotation, FRotator(0, 0, 0), DeltaTimeX, 15.0f);
}

void UQAnimInstance::BindDeclares()
{
	PlayTransition.AddUObject(this, &UQAnimInstance::OnPlayTransition);
	PlayDynamicTransition.AddUObject(this, &UQAnimInstance::OnPlayDynamicTransition);
	JumpEvent.AddUObject(this, &UQAnimInstance::OnJump);
}

void UQAnimInstance::OnPlayDynamicTransition(float ReTriggerDelay, FDynamicMontageParams Parameters)
{
	static FTimerHandle Timer;
	static FTimerDelegate TimerDelegate;
	TimerDelegate.BindLambda([&]() {
		PlaySlotAnimationAsDynamicMontage(Parameters.Animation, TEXT("Grounded Slot"),
		Parameters.BlendInTime,
		Parameters.BlendOutTime,
		Parameters.PlayRate, 1, 0.0f,
		Parameters.StartTime);
		});
	GetWorld()->GetTimerManager().SetTimer(Timer, TimerDelegate, ReTriggerDelay, false);
}

void UQAnimInstance::OnPlayTransition(FDynamicMontageParams Parameters)
{
	PlaySlotAnimationAsDynamicMontage(Parameters.Animation, TEXT("Grounded Slot"),
		Parameters.BlendInTime,
		Parameters.BlendOutTime,
		Parameters.PlayRate,1,0.0f,
		Parameters.StartTime);
}

void UQAnimInstance::K2_PlayTransition(FDynamicMontageParams Parameters)
{
	PlayTransition.Broadcast(Parameters);
}


void UQAnimInstance::K2_PlayDynamicTransition(float ReTriggerDelay, FDynamicMontageParams Parameters)
{
	PlayDynamicTransition.Broadcast(ReTriggerDelay, Parameters);
}

void UQAnimInstance::OnJump()
{
	Jumped = true;
	JumpPlayRate = FMath::GetMappedRangeValueClamped(TRange<float>(0.f, 600.f),
		TRange<float>(1.2f, 1.5f), Speed);
	/*const FLatentActionInfo LatentInfo(0, FMath::Rand(), TEXT("OnJumpFinishCallBack"), this);
	UKismetSystemLibrary::Delay(GetWorld(), 0.1, LatentInfo);*/
	static FTimerHandle Timer;
	static FTimerDelegate TimerDelegate;
	TimerDelegate.BindLambda([&]() {
		Jumped = false;
		});
	GetWorld()->GetTimerManager().SetTimer(Timer, TimerDelegate, 0.1, false);
}



