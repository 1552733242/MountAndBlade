// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "QCharacterMovementInterface.h"
#include "Camera/QCameraInterface.h"
#include "QCharacter.generated.h"

struct FInputActionValue;
class UCameraComponent;
class USpringArmComponent;
class UInputMappingContext;
class UInputAction;

DECLARE_EVENT(AQCharacter, BreakFallRegister)


USTRUCT(BlueprintType)
struct FCharacterMovementSetting : public FTableRowBase
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float WalkSpeed;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float RunSpeed;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float SprintSpeed;
	UPROPERTY(EditAnywhere,	BlueprintReadWrite)
		UCurveVector* MovementCurve;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		UCurveFloat* RotationRateCurve;
};
USTRUCT(BlueprintType)
struct FCharacterMovementSettingStance : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FCharacterMovementSetting Standing;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FCharacterMovementSetting Crouching;
};
USTRUCT(BlueprintType)
struct FCharacterMovementSettingState : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FCharacterMovementSettingStance VelocityDirection;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FCharacterMovementSettingStance LookingDirection;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FCharacterMovementSettingStance Aiming;
};


UCLASS()
class MOUNTANDBLADE_API AQCharacter : public ACharacter,
	public IQCharacterMovementInterface,
	public IQCameraInterface
{
	GENERATED_BODY()

public:
	AQCharacter();

protected:
	virtual void BeginPlay() override;
public:
	virtual void Tick(float DeltaTime) override;
	virtual void OnMovementModeChanged(EMovementMode PrevMovementMode, uint8 PreviousCustomMode = 0)override;
	virtual void OnStartCrouch(float HeightAdjust, float ScaledHeightAdjust)override;
	virtual void OnEndCrouch(float HeightAdjust, float ScaledHeightAdjust)override;
	virtual void Landed(const FHitResult& Hit)override;
	virtual void OnJumped_Implementation()override;
	virtual void GetCurrentStates(FCharacterMovementCurrentStates& Info)override;
	virtual void GetEssentialValues(FCharacterMovementEssentialValues& Info)override;
	virtual void Get3PPivotTarget(FTransform& Transform)override;
	virtual void GetCameraParameters(FCameraParameters& CameraParameters)override;
protected:
	//Some Action BPI
	 void Aim();
	 void StopAim();
	 void Sprint();
	 void StopSprint();
	 void ChangeMovementStance();

private:
	//Begin Play
	void SetMovementModel();
	void BindDeclares();
	//Tick
	void CacheValues();
	void SetEssentialValues();
	FVector CalculateAcceleration();
	//Some Update State (Not Tick)
	float GetAnimCurveValue(const FName& CurveName);
	void UpdateDynamicMovementSettings(ECharacterMovementGait AllowedGait);
	void UpdateCharacterMovement();
	void UpdateGroudedRotation();
	bool CanUpdateMovingRotation();
	void SmoothCharacterRotation(const FRotator& Target, float TargetInterpSpeed,float ActorInterpSpeed);
	float CalcuateGroundRotationRate();
	void LimitRotation(float AimYawMin, float AimYawMax, float InterpSpeed);
	//Some Temp Function
	ECharacterMovementGait GetAllowedGait();				//获取允许的步态	
	ECharacterMovementGait GetActualGait(ECharacterMovementGait AllowedGait);
	FCharacterMovementSetting GetTargetMovementSettings();
	float GetMappedSpeed();
	bool CanSprint();

private:
	//References
	UAnimInstance* MainAnimInstacne;
	//Essential Information
	FVector Acceleration;
	bool IsMoving;
	bool HasMovementInput;
	FRotator LastMovementInputRotation;
	FRotator LastVelocityRotation;
	float Speed;
	float MovementInputAmount;
	float AimYawRate;
	//Character System
	bool RightShoulder = true;
	float ThirdPersonFOV = 90.f;
	float FirstPersonFOV = 90.f;
	//State Values
	ECharacterMovementState MovementState = ECharacterMovementState::None;
	ECharacterMovementState PreviousMovementState = ECharacterMovementState::None;
	ECharacterMovementAction MovementAction = ECharacterMovementAction::None;
	ECharacterMovementRotationMode MovementRotationMode = ECharacterMovementRotationMode::LookingDirection;
	ECharacterMovementGait MovementGait = ECharacterMovementGait::Walk;
	ECharacterMovementStance MovementStance = ECharacterMovementStance::Stance;
	ECharacterViewMode ViewMode = ECharacterViewMode::ThirdPerson;
	ECharacterOverlayState OverlayState = ECharacterOverlayState::Default;
	//Movement System
	FCharacterMovementSetting CurrentMovementSetting;
	//Rotation System
	FRotator TargetRotation;
	FRotator InAirRotation;
	float YawOffset;
	//Mantle System
	//Ragdoll System
	bool RagdollOnGround;
	bool RagdollFaceUp;
	FVector LastRagdollVelocity;
	//Cache Values
	FVector PreviousVelocity;
	float	PreviousAimYaw;
	//Input
	ECharacterMovementRotationMode DesiredRotationMode = ECharacterMovementRotationMode::LookingDirection;
	ECharacterMovementGait DesiredGit = ECharacterMovementGait::Walk;
	ECharacterMovementStance  DesiredStance = ECharacterMovementStance::Stance;
	float LookUPDownRate = 1.25f;
	float LookLeftRightRate = 1.25f;
	bool BreakFall = false;
	bool SprintHeld = false;
	//References Event Declare
	FSetOverlayStateRegister SetOverlayState;
	FSetViewModeRegister SetViewMode;
	FSetGaitRegister SetGait;
	FSetRotationRegister SetRotation;
	FSetMovementActionRegister SetMovementAction;
	FSetMovementStateRegister SetMovementState;
	//Self Declare
	BreakFallRegister BreakFallEvent;
private:
	UPROPERTY(EditAnywhere, Category = "Movement")
		FCharacterMovementSettingState MovementConfigData;
	UPROPERTY(EditAnywhere, Category = "Movement")
		FDataTableRowHandle MovementDataHandle;
private:
	void OnMovementStateChanged(ECharacterMovementState NewState);
	void OnMovementActionChanged(ECharacterMovementAction NewAction);
	void OnRotationModeChanged(ECharacterMovementRotationMode NewRotationMode);
	void OnGaitChanged(ECharacterMovementGait NewGait);
	void OnViewModeChanged(ECharacterViewMode NewViewMode);
	void OnOverlayStateChanged(ECharacterOverlayState NewOverlayState);
	void OnStanceChanged(ECharacterMovementStance NewStance);
	void OnBreakFall();
};



