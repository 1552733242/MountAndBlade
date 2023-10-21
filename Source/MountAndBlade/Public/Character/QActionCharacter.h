// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/QCharacter.h"
#include "QActionCharacter.generated.h"

/**
 * 
 */
UCLASS()
class MOUNTANDBLADE_API AQActionCharacter : public AQCharacter
{
	GENERATED_BODY()
public:
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
private:
	void OnLook(const FInputActionValue& Value);
	void OnMove(const FInputActionValue& Value);
	void OnZoom(const FInputActionValue& Value);
	//Input System
	UPROPERTY(EditDefaultsOnly, Category = "Input|MapContext")
		UInputMappingContext* IMBase;
	UPROPERTY(EditDefaultsOnly, Category = "Input|BasicLocomotion")
		UInputAction* IAMove;
	UPROPERTY(EditDefaultsOnly, Category = "Input|BasicLocomotion")
		UInputAction* IALook;
	UPROPERTY(EditDefaultsOnly, Category = "Input|BasicLocomotion")
		UInputAction* IAZoom;
	UPROPERTY(EditDefaultsOnly, Category = "Input|BasicLocomotion")
		UInputAction* IAJump;
	UPROPERTY(EditDefaultsOnly, Category = "Input|BasicLocomotion")
		UInputAction* IACrouch;
	UPROPERTY(EditDefaultsOnly, Category = "Input|BasicLocomotion")
		UInputAction* IASprint;
	UPROPERTY(EditDefaultsOnly, Category = "Input|Action")
		UInputAction* IAAim;
	UPROPERTY(EditDefaultsOnly, Category = "Input|Action")
		UInputAction* IARoll;
	UPROPERTY(EditDefaultsOnly, Category = "Input|Inventory")
		UInputAction* IAOpenInventoryPanel;
	UPROPERTY(EditDefaultsOnly, Category = "Input|Inventory")
		UInputAction* IAPickUp;
};
