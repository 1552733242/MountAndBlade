// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "QPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class MOUNTANDBLADE_API AQPlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:
	AQPlayerController();


protected:
	virtual void OnPossess(APawn* aPawn)override;

};
