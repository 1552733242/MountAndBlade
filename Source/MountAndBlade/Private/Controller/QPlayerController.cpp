// Fill out your copyright notice in the Description page of Project Settings.


#include "Controller/QPlayerController.h"
#include "Camera/QPlayerCameraManager.h"
AQPlayerController::AQPlayerController()
{
	PlayerCameraManagerClass = AQPlayerCameraManager::StaticClass();
}
void AQPlayerController::OnPossess(APawn* aPawn)
{
	Super::OnPossess(aPawn);

	if (AQPlayerCameraManager* QPlayerCameraManager = Cast<AQPlayerCameraManager>(PlayerCameraManager)) {
		QPlayerCameraManager->OnPossess(aPawn);
	}
}
