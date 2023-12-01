// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/QActionCharacter.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"

#define Message(key,...) GEngine->AddOnScreenDebugMessage(key, 1, FColor::Red,FString::Format(TEXT("{0}"), { FStringFormatArg(##__VA_ARGS__)}));
#define Message2(key,Arg1,Arg2) GEngine->AddOnScreenDebugMessage(key, 1, FColor::Red,FString::Format(TEXT("{0}:{1}"), { FStringFormatArg(Arg1),FStringFormatArg(Arg2)}));


void AQActionCharacter::BeginPlay()
{
	Super::BeginPlay();
	SetOverlayState.Broadcast(OverlayStateState);
}

void AQActionCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
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
		EnhancedInputComponent->BindAction(IAMove, ETriggerEvent::Triggered, this, &AQActionCharacter::OnMove);
		EnhancedInputComponent->BindAction(IALook, ETriggerEvent::Triggered, this, &AQActionCharacter::OnLook);
		EnhancedInputComponent->BindAction(IAZoom, ETriggerEvent::Triggered, this, &AQActionCharacter::OnZoom);
		EnhancedInputComponent->BindAction(IASprint, ETriggerEvent::Started, this, &AQActionCharacter::Sprint);
		EnhancedInputComponent->BindAction(IASprint, ETriggerEvent::Completed, this, &AQActionCharacter::StopSprint);
		EnhancedInputComponent->BindAction(IAAim, ETriggerEvent::Started, this, &AQActionCharacter::Aim);
		EnhancedInputComponent->BindAction(IAAim, ETriggerEvent::Completed, this, &AQActionCharacter::StopAim);
		EnhancedInputComponent->BindAction(IAJump, ETriggerEvent::Started, this, &AQActionCharacter::Jump);
		EnhancedInputComponent->BindAction(IAJump, ETriggerEvent::Completed, this, &AQActionCharacter::StopJumping);
		EnhancedInputComponent->BindAction(IACrouch, ETriggerEvent::Started, this, &AQActionCharacter::ChangeMovementStance);
		EnhancedInputComponent->BindAction(IARoll, ETriggerEvent::Started, this, &AQActionCharacter::Roll);
	}

}

void AQActionCharacter::OnOverlayStateChanged(ECharacterOverlayState NewOverlayState)
{
	Super::OnOverlayStateChanged(NewOverlayState);
	UpdataHeldObject(NewOverlayState);
}

void AQActionCharacter::OnMove(const FInputActionValue& Value)
{

	FVector2D  MoveMent = Value.Get<FVector2D>();
	FRotator Dir = FRotator(0, GetControlRotation().Yaw, 0);
	AddMovementInput(Dir.Quaternion().GetForwardVector(), MoveMent.X);
	AddMovementInput(Dir.Quaternion().GetRightVector(), MoveMent.Y);

}
void AQActionCharacter::OnLook(const FInputActionValue& Value)
{
	FVector2D  LookMent = Value.Get<FVector2D>();
	AddControllerYawInput(LookMent.X);
	AddControllerPitchInput(LookMent.Y);
}
void AQActionCharacter::OnZoom(const FInputActionValue& Value)
{

}


