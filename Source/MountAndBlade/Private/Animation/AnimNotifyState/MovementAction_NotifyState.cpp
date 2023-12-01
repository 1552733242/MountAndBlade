// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/AnimNotifyState/MovementAction_NotifyState.h"
#include "Character/QCharacter.h"
void UMovementAction_NotifyState::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	AQCharacter* Character = Cast<AQCharacter>(MeshComp->GetOwner());
	Character->SetMovementAction.Broadcast(MovementAction);
}

void UMovementAction_NotifyState::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	AQCharacter* Character = Cast<AQCharacter>(MeshComp->GetOwner());
	FCharacterMovementCurrentStates Info;
	Character->GetCurrentStates(Info);
	if (Info.MovementAction == MovementAction) {
		Character->SetMovementAction.Broadcast(ECharacterMovementAction::None);
	}
	
	
}

FString UMovementAction_NotifyState::GetNotifyName_Implementation() const
{
	 //MovementAction
		


	return FString();
}
