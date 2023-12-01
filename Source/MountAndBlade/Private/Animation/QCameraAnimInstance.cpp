// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/QCameraAnimInstance.h"
#include "Character/QCharacter.h"
void UQCameraAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);
	UpdateCharacterInfo();
}

void UQCameraAnimInstance::NativeInitializeAnimation()
{
	
}

void UQCameraAnimInstance::UpdateCharacterInfo()
{
	if (IQCharacterMovementInterface* CharacterInfo = Cast<IQCharacterMovementInterface>(ControlledPawn)) {
		FCharacterMovementCurrentStates Info;
		CharacterInfo->GetCurrentStates(Info);
		MovementState = Info.MovementState;
		MovementAction = Info.MovementAction;
		RotationMode = Info.RotationMode;
		Gait = Info.ActualGait;
		MovementStance = Info.ActualStance;
		ViewMode = Info.ViewMode;
	}
	if (IQCameraInterface* CameraInfo = Cast<IQCameraInterface>(ControlledPawn)) {
		CameraInfo->Get3PPivotTarget(PivotTarget);
		CameraInfo->GetCameraParameters(Parameters);
	}
}
