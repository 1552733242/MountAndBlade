// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "CharacterEnum.h"
#include "QCharacterMovementInterface.generated.h"

DECLARE_EVENT_OneParam(UQCharacterMovementInterface, FSetOverlayStateRegister, ECharacterOverlayState)
DECLARE_EVENT_OneParam(UQCharacterMovementInterface, FSetViewModeRegister, ECharacterViewMode)
DECLARE_EVENT_OneParam(UQCharacterMovementInterface, FSetGaitRegister, ECharacterMovementGait)
DECLARE_EVENT_OneParam(UQCharacterMovementInterface, FSetRotationRegister, ECharacterMovementRotationMode)
DECLARE_EVENT_OneParam(UQCharacterMovementInterface, FSetMovementActionRegister, ECharacterMovementAction)
DECLARE_EVENT_OneParam(UQCharacterMovementInterface, FSetMovementStateRegister, ECharacterMovementState)


USTRUCT(BlueprintType)
struct FCharacterMovementCurrentStates
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
		ECharacterMovementState MovementState;
	UPROPERTY(BlueprintReadWrite)
		ECharacterMovementState PrevMovementState;
	UPROPERTY(BlueprintReadWrite)
		ECharacterMovementAction Action;
	UPROPERTY(BlueprintReadWrite)
		ECharacterMovementRotationMode RotationMode;
	UPROPERTY(BlueprintReadWrite)
		ECharacterMovementGait	ActualGait;
	UPROPERTY(BlueprintReadWrite)
		ECharacterMovementStance ActualStance;
	UPROPERTY(BlueprintReadWrite)
		ECharacterViewMode ViewMode;
	UPROPERTY(BlueprintReadWrite)
		ECharacterOverlayState OverlayState;
};

USTRUCT(BlueprintType)
struct FCharacterMovementEssentialValues
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
		FVector Velocity;
	UPROPERTY(BlueprintReadWrite)
		FVector Acceleration;
	UPROPERTY(BlueprintReadWrite)
		FVector MovementInput;
	UPROPERTY(BlueprintReadWrite)
		bool IsMoving;
	UPROPERTY(BlueprintReadWrite)
		bool HasMovementInput;
	UPROPERTY(BlueprintReadWrite)
		float Speed;
	UPROPERTY(BlueprintReadWrite)
		float MovementInputAmount;
	UPROPERTY(BlueprintReadWrite)
		FRotator AimingRotation;
	UPROPERTY(BlueprintReadWrite)
		float AimYawRate;
};

UINTERFACE(MinimalAPI)
class UQCharacterMovementInterface : public UInterface
{
	GENERATED_BODY()
};

class MOUNTANDBLADE_API IQCharacterMovementInterface
{
	GENERATED_BODY()

public:
	//Character Information
	virtual void GetCurrentStates(FCharacterMovementCurrentStates& Info) = 0;
	virtual void GetEssentialValues(FCharacterMovementEssentialValues& Info) = 0;
};
