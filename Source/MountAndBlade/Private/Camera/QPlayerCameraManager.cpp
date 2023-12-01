// Fill out your copyright notice in the Description page of Project Settings.


#include "Camera/QPlayerCameraManager.h"
#include "Animation/QCameraAnimInstance.h"
#include "Kismet/GameplayStatics.h"
#define Message(key,...) GEngine->AddOnScreenDebugMessage(key, 1, FColor::Red,FString::Format(TEXT("{0}"), { FStringFormatArg(##__VA_ARGS__)}));
#define Message2(key,Arg1,Arg2) GEngine->AddOnScreenDebugMessage(key, 1, FColor::Red,FString::Format(TEXT("{0}:{1}"), { FStringFormatArg(Arg1),FStringFormatArg(Arg2)}));


AQPlayerCameraManager::AQPlayerCameraManager()
{
	Camera = CreateDefaultSubobject<USkeletalMeshComponent>("Camera");
}

void AQPlayerCameraManager::OnPossess(APawn* aPawn)
{
	ControlledPawn = aPawn;
	if (UQCameraAnimInstance* CameraAnimInstan = Cast<UQCameraAnimInstance>(Camera->GetAnimInstance())){
		CameraAnimInstan->PlayerController = GetOwningPlayerController();
		CameraAnimInstan->ControlledPawn = aPawn;
	}
}

void AQPlayerCameraManager::UpdateViewTargetInternal(FTViewTarget& OutVT, float DeltaTime)
{
	if (OutVT.Target)
	{
		FVector OutLocation;
		FRotator OutRotation;
		float OutFOV;

		if (NativeUpdateCamera(OutVT.Target, OutLocation, OutRotation, OutFOV))
		{
			OutVT.POV.Location = OutLocation;
			OutVT.POV.Rotation = OutRotation;
			OutVT.POV.FOV = OutFOV;
		}
		else
		{
			OutVT.Target->CalcCamera(DeltaTime, OutVT.POV);
		}
	}
}
		
bool AQPlayerCameraManager::NativeUpdateCamera(AActor* CameraTarget, FVector& NewCameraLocation, FRotator& NewCameraRotation, float& NewCameraFOV)
{
	
	CustomCameraBehavior();
	NewCameraLocation = TargetCameraLocation;
	NewCameraRotation = TargerCameraRotation;
	NewCameraFOV = 90.f;
	return true;
}

void AQPlayerCameraManager::CustomCameraBehavior()
{
	
	FTransform PivotTransform;
	Cast<IQCameraInterface>(ControlledPawn)->Get3PPivotTarget(PivotTransform);

	
	FRotator CameraRotation = GetCameraRotation();
	FRotator ControllerRotation = GetOwningPlayerController()->GetControlRotation();
	float RotationSpeed = GetCameraBehaviorParam(TEXT("RotationLagSpeed"));

	TargerCameraRotation = FMath::RInterpTo(CameraRotation, ControllerRotation,
		UGameplayStatics::GetWorldDeltaSeconds(GetWorld()), RotationSpeed);
	FVector LagSpeed = FVector(GetCameraBehaviorParam(TEXT("PivotLagSpeed_X")),
		GetCameraBehaviorParam(TEXT("PivotLagSpeed_Y")),
		GetCameraBehaviorParam(TEXT("PivotLagSpeed_Z")));

	SmoothedPivotTarget.SetLocation(CalculateAxisIndependentLag(SmoothedPivotTarget.GetLocation(), PivotTransform.GetLocation(), TargerCameraRotation, LagSpeed));
	SmoothedPivotTarget.SetRotation(PivotTransform.GetRotation());

	PivotLocation = SmoothedPivotTarget.GetLocation();
	PivotLocation += SmoothedPivotTarget.GetRotation().GetForwardVector()* GetCameraBehaviorParam(TEXT("PivotOffset_X"));
	PivotLocation += SmoothedPivotTarget.GetRotation().GetRightVector()* GetCameraBehaviorParam(TEXT("PivotOffset_Y"));
	PivotLocation += SmoothedPivotTarget.GetRotation().GetUpVector()* GetCameraBehaviorParam(TEXT("PivotOffset_Z"));

	FVector NewCameramLocation;
	NewCameramLocation = PivotLocation;
	NewCameramLocation += TargerCameraRotation.Quaternion().GetForwardVector()* GetCameraBehaviorParam(TEXT("CameraOffset_X"));
	NewCameramLocation += TargerCameraRotation.Quaternion().GetRightVector() * GetCameraBehaviorParam(TEXT("CameraOffset_Y"));
	NewCameramLocation += TargerCameraRotation.Quaternion().GetUpVector() * GetCameraBehaviorParam(TEXT("CameraOffset_Z"));

	TargetCameraLocation = NewCameramLocation;
}

float AQPlayerCameraManager::GetCameraBehaviorParam(FName CurveName)
{

	UAnimInstance* AnimInstance = Camera->GetAnimInstance();
	if (!AnimInstance)return 0.0f;
	return AnimInstance->GetCurveValue(CurveName);
	
}

FVector AQPlayerCameraManager::CalculateAxisIndependentLag(const FVector& CurrentLocation, 
	const FVector& TargetLocation, const FRotator& CameraRotation, const FVector& LagSpeed)
{

	FRotator CameraRotationYaw = FRotator(0.0, CameraRotation.Yaw, 0.0);
	FVector CurrentLocationUnRotatorVector = CameraRotationYaw.UnrotateVector(CurrentLocation);
	FVector TargetLocationUnRotatorVector = CameraRotationYaw.UnrotateVector(TargetLocation);
	FVector NewLocation = FVector(
		FMath::FInterpTo(CurrentLocationUnRotatorVector.X, TargetLocationUnRotatorVector.X,
			UGameplayStatics::GetWorldDeltaSeconds(GetWorld()), LagSpeed.X),
		FMath::FInterpTo(CurrentLocationUnRotatorVector.Y, TargetLocationUnRotatorVector.Y,
			UGameplayStatics::GetWorldDeltaSeconds(GetWorld()), LagSpeed.Y),
		FMath::FInterpTo(CurrentLocationUnRotatorVector.Z, TargetLocationUnRotatorVector.Z,
			UGameplayStatics::GetWorldDeltaSeconds(GetWorld()), LagSpeed.Z));
	
	return CameraRotationYaw.RotateVector(NewLocation);
}


