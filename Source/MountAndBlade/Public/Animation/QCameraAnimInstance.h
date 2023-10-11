// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "Character/CharacterEnum.h"
#include "Camera/QCameraInterface.h"
#include "QCameraAnimInstance.generated.h"



UCLASS()
class MOUNTANDBLADE_API UQCameraAnimInstance : public UAnimInstance
{
	GENERATED_BODY()
public:
	virtual void NativeUpdateAnimation(float DeltaSeconds)override;
	virtual void NativeInitializeAnimation()override;
	void UpdateCharacterInfo();
public:
	APlayerController* PlayerController;
	APawn* ControlledPawn;
protected:
	UPROPERTY(BlueprintReadWrite)
		ECharacterMovementState MovementState;
	UPROPERTY(BlueprintReadWrite)
		ECharacterMovementAction MovementAction;
	UPROPERTY(BlueprintReadWrite)
		ECharacterMovementRotationMode RotationMode;
	UPROPERTY(BlueprintReadWrite)
		ECharacterMovementGait Gait;
	UPROPERTY(BlueprintReadWrite)
		ECharacterMovementStance MovementStance;
	UPROPERTY(BlueprintReadWrite)
		ECharacterViewMode ViewMode;
	UPROPERTY(BlueprintReadWrite)
		FTransform PivotTarget;
	UPROPERTY(BlueprintReadWrite)
		FCameraParameters  Parameters;
};
