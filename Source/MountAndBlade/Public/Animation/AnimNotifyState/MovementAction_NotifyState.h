// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "Character/CharacterEnum.h"

#include "MovementAction_NotifyState.generated.h"


class AQCharacter;
enum class ECharacterMovementAction :uint8;

UCLASS()
class MOUNTANDBLADE_API UMovementAction_NotifyState : public UAnimNotifyState
{
	GENERATED_BODY()
	
private:
	
	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference);
	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference);
	UFUNCTION()
		FString GetNotifyName_Implementation() const;

	ECharacterMovementAction MovementAction = ECharacterMovementAction::None;

};
