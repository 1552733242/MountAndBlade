// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "QCameraInterface.generated.h"

USTRUCT(BlueprintType)
struct FCameraParameters
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
		float ThirdPersonFOV;
	UPROPERTY(BlueprintReadWrite)
		float FirstPersonFOV;
	UPROPERTY(BlueprintReadWrite)
		bool RightShoulder;
};

UINTERFACE(MinimalAPI)
class UQCameraInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class MOUNTANDBLADE_API IQCameraInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	virtual void Get3PPivotTarget(FTransform& Transform) = 0;
	virtual void GetCameraParameters(FCameraParameters& CameraParameters) = 0;
};
