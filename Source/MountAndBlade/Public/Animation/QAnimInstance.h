// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "Character/CharacterEnum.h"
#include "QAnimInstance.generated.h"

class AQCharacter;
class UCurveVector;
class UCurveFloat;
class UAnimSequenceBase;

DECLARE_EVENT(UQAnimInstance, FAnimJumpRegister)


USTRUCT(BlueprintType)	
struct FVelocityBlend
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
		float VelocityBlendF;
	UPROPERTY(BlueprintReadWrite)
		float VelocityBlendB;
	UPROPERTY(BlueprintReadWrite)
		float VelocityBlendL;
	UPROPERTY(BlueprintReadWrite)
		float VelocityBlendR;
	FString ToString()const {
		return	FString::Printf(TEXT("VeclocityBlendF = %f\n"), VelocityBlendF)+
				FString::Printf(TEXT("VeclocityBlendB = %f\n"), VelocityBlendB) +
				FString::Printf(TEXT("VeclocityBlendL = %f\n"), VelocityBlendL) +
				FString::Printf(TEXT("VeclocityBlendR = %f\n"), VelocityBlendR);
	}
};
USTRUCT(BlueprintType)
struct FLeanAmount
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
		float LR;
	UPROPERTY(BlueprintReadWrite)
		float BF;
};
USTRUCT(BlueprintType)
struct FDynamicMontageParams
{
	GENERATED_BODY()
	UPROPERTY(BlueprintReadWrite)
		UAnimSequenceBase* Animation;
	UPROPERTY(BlueprintReadWrite)
		float BlendInTime;
	UPROPERTY(BlueprintReadWrite)
		float BlendOutTime;
	UPROPERTY(BlueprintReadWrite)
		float PlayRate;
	UPROPERTY(BlueprintReadWrite)
		float StartTime;
};
USTRUCT(BlueprintType)
struct FTurnInPlaceAsset
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere,BlueprintReadWrite)
		UAnimSequenceBase* Animation;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float AnimatedAngle;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FName SlotName;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float PlayRate;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		bool ScaleTurnAngle;
};
UENUM(BlueprintType)
enum class EMovementDirection : uint8
{
	Forward		UMETA(DisplayName = "Forward"),
	Right		UMETA(DisplayName = "Right"),
	Left		UMETA(DisplayName = "Left"),
	Backward	UMETA(DisplayName = "Backward"),
};
UENUM(BlueprintType)
enum class EGroundedEntryState : uint8
{
	None		UMETA(DisplayName = "None"),
	Roll		UMETA(DisplayName = "Roll"),
};
UENUM(BlueprintType)
enum class EHipsDirection : uint8
{
	F		UMETA(DisplayName = "F"),
	B		UMETA(DisplayName = "B"),
	RF		UMETA(DisplayName = "RF"),
	RB		UMETA(DisplayName = "RB"),
	LF		UMETA(DisplayName = "LF"),
	LB		UMETA(DisplayName = "LB")
};


DECLARE_EVENT_TwoParams(UQCameraAnimInstance, FPlayDynamicTransition, float, FDynamicMontageParams)
DECLARE_EVENT_OneParam(UQCameraAnimInstance, FPlayTransition,FDynamicMontageParams)

UCLASS()
class MOUNTANDBLADE_API UQAnimInstance : public UAnimInstance
{
	GENERATED_BODY()
	
public:
	UQAnimInstance();
	virtual void NativeUpdateAnimation(float DeltaSeconds)override;
	virtual void NativeInitializeAnimation()override;

public:
	FAnimJumpRegister JumpEvent;
protected:
	
	/// Character Values;    (Character Information)
	UPROPERTY(BlueprintReadWrite)
		FRotator AimRotation;
	UPROPERTY(BlueprintReadWrite)
		FVector Velocity;
	UPROPERTY(BlueprintReadWrite)
		FVector RelativeVelocityDirection;
	UPROPERTY(BlueprintReadWrite)
		FVector Acceleration;
	UPROPERTY(BlueprintReadWrite)
		FVector MovementInput;//Charactermovement Acceleration be used;
	UPROPERTY(BlueprintReadWrite)
		bool  IsMoving;
	UPROPERTY(BlueprintReadWrite)
		bool  HasMovementInput;
	UPROPERTY(BlueprintReadWrite)
		float Speed;
	UPROPERTY(BlueprintReadWrite)
		float MovementInputAmount;
	UPROPERTY(BlueprintReadWrite)
		float AimYawRate;
	UPROPERTY(BlueprintReadWrite)
		float ZoomAmount;
	UPROPERTY(BlueprintReadWrite)
		ECharacterMovementState MovementState;
	UPROPERTY(BlueprintReadWrite)
		ECharacterMovementState PreviousMovementState;
	UPROPERTY(BlueprintReadWrite)
		ECharacterMovementAction MovementAction;
	UPROPERTY(BlueprintReadWrite)
		ECharacterMovementRotationMode RotationMode;
	UPROPERTY(BlueprintReadWrite)
		ECharacterMovementGait MovementGait;
	UPROPERTY(BlueprintReadWrite)
		ECharacterMovementStance MovementStance;
	UPROPERTY(BlueprintReadWrite)
		ECharacterViewMode ViewMode;
	UPROPERTY(BlueprintReadWrite)
		ECharacterOverlayState OverlayState;
	//Aim Graph - Grounded
	UPROPERTY(BlueprintReadWrite)
		EGroundedEntryState GroundedEntryState;
	UPROPERTY(BlueprintReadWrite)
		EMovementDirection MovementDirection;
	UPROPERTY(BlueprintReadWrite)
		EHipsDirection TrackedHipsDirection;
	UPROPERTY(BlueprintReadWrite)
		FVector RelativeAccelerationAmount;
	UPROPERTY(BlueprintReadWrite)
		bool ShouldMove;
	UPROPERTY(BlueprintReadWrite)
		bool RotateL;
	UPROPERTY(BlueprintReadWrite)
		bool RotateR;
	UPROPERTY(BlueprintReadWrite)
		bool Pivot;
	UPROPERTY(BlueprintReadWrite)
		float RotateRate;
	UPROPERTY(BlueprintReadWrite)
		float RotationScale;
	UPROPERTY(BlueprintReadWrite)
		float DiagonalScaleAmount;
	UPROPERTY(BlueprintReadWrite)
		float WalkRunBlend;
	UPROPERTY(BlueprintReadWrite)
		float StandingPlayRate = 1.0f;
	UPROPERTY(BlueprintReadWrite)
		float CrouchingPlayRate = 1.0f;
	UPROPERTY(BlueprintReadWrite)
		float StrideBlend;
	UPROPERTY(BlueprintReadWrite)
		FVelocityBlend VelocityBlend;
	UPROPERTY(BlueprintReadWrite)
		FLeanAmount LeanAmount = { 0.f,0.f };
	UPROPERTY(BlueprintReadWrite)
		float FYaw;
	UPROPERTY(BlueprintReadWrite)
		float BYaw;
	UPROPERTY(BlueprintReadWrite)
		float LYaw;
	UPROPERTY(BlueprintReadWrite)
		float RYaw;
	//Anim Graph - In Air
	UPROPERTY(BlueprintReadWrite)
		bool Jumped;
	UPROPERTY(BlueprintReadWrite)
		float JumpPlayRate;
	UPROPERTY(BlueprintReadWrite)
		float FallSpeed;
	UPROPERTY(BlueprintReadWrite)
		float LandPrediction;
	//Aim Graph - Aiming Values
		FRotator SmoothedAimingRotation;
		FRotator SpineRotation;
		FVector2D AimingAngle;
		FVector2D SmoothedAimingAngle;
	UPROPERTY(BlueprintReadWrite)
		float AimSweepTime;
		float InputYawOffsetTime;
		float ForwardYawTime;
		float LeftYawTime;
		float RightYawTime;

	//Anim Graph - Layer Blending
		int OverlayOverrideState;
		float Enable_AimOffset;
	UPROPERTY(BlueprintReadWrite)
		float BasePose_N = 1.f;
	UPROPERTY(BlueprintReadWrite)
		float BasePose_CLF;
	UPROPERTY(BlueprintReadWrite)
		float Arm_L;
	UPROPERTY(BlueprintReadWrite)
		float Arm_L_Add;
	UPROPERTY(BlueprintReadWrite)
		float Arm_L_LS;
	UPROPERTY(BlueprintReadWrite)
		float Arm_L_MS;
	UPROPERTY(BlueprintReadWrite)
		float Arm_R;
	UPROPERTY(BlueprintReadWrite)
		float Arm_R_Add;
	UPROPERTY(BlueprintReadWrite)
		float Arm_R_LS;
	UPROPERTY(BlueprintReadWrite)
		float Arm_R_MS;
	UPROPERTY(BlueprintReadWrite)
		float Hand_L;
	UPROPERTY(BlueprintReadWrite)
		float Hand_R;
	UPROPERTY(BlueprintReadWrite)
		float Legs;
	UPROPERTY(BlueprintReadWrite)
		float Legs_Add;
	UPROPERTY(BlueprintReadWrite)
		float Pelvis;
	UPROPERTY(BlueprintReadWrite)
		float Pelvis_Add;
	UPROPERTY(BlueprintReadWrite)
		float Spine;
	UPROPERTY(BlueprintReadWrite)
		float Spine_Add;
	UPROPERTY(BlueprintReadWrite)
		float Head;
	UPROPERTY(BlueprintReadWrite)
		float Head_Add;
	UPROPERTY(BlueprintReadWrite)
		float Enable_HandIK_L;
	UPROPERTY(BlueprintReadWrite)
		float Enable_HandIK_R;
	//Anim Grap - Foot IK
	UPROPERTY(BlueprintReadWrite)
		float FootLock_L_Alplha;
	UPROPERTY(BlueprintReadWrite)
		float FootLock_R_Alplha;
	UPROPERTY(BlueprintReadWrite)
		FVector FootLock_L_Location;
	UPROPERTY(BlueprintReadWrite)
		FVector FootLock_R_Location;
	UPROPERTY(BlueprintReadWrite)
		FRotator FootLock_L_Rotation;
	UPROPERTY(BlueprintReadWrite)
		FRotator FootLock_R_Rotation;
	UPROPERTY(BlueprintReadWrite)
		FVector FootOffset_L_Location;
	UPROPERTY(BlueprintReadWrite)
		FVector FootOffset_R_Location;
	UPROPERTY(BlueprintReadWrite)
		FRotator FootOffset_L_Rotation;
	UPROPERTY(BlueprintReadWrite)
		FRotator FootOffset_R_Rotation;
	UPROPERTY(BlueprintReadWrite)
		FVector PelvisOffset;
	UPROPERTY(BlueprintReadWrite)
		float PelvisAlpha;
	//Trun In Place
		float ElapsedDelayTime;
private:
	void UpdateCharacterInfo();
	void UpdateAimingValues();
	void UpdateLayerValues();
	void UpdateRotationValues();
	void UpDateFootIK();
	void UpdateMovementValues();
	void TurninPlace(const FRotator& TargetRotation, float PlayRateScale, float StartTime, bool OverrideCurrent);

	void SetFootOffsets(const FName& EnableFootIKCurve,const FName& IKFootBone,
		const FName& RootBone,FVector& CurrentLocationTarget, 
		FVector& CurrentLocationOffset, FRotator& CurrentRotationOffset);
	void SetPelvisIKOffset(const FVector& FootOffset_L_Target, const FVector& FootOffset_R_Target);
	void SetFootLocking(const FName& EnableFootIKCurve, const FName& FootLockCurve, 
		const FName& IKFootBone, float& CurrentFootLockAlpha,
		FVector& CurrentFootLockLocation, FRotator& CurrentFootLockRotation);
	void SetFootLockOffsets(FVector& LocalLocation, FRotator& LocalRotation);

	void ResetIKOffsets();
private:
	void UpdateOnGround();
	float CalculateWalkRunBlend();
	float CalculateStrideBlend();
	float CalculateStandingPlayRate();
	float CalculateCrouchingPlayRate();
	float CalculateDiagonalScaleAmount();
	bool  ShouldMoveCheck();
	bool  CanRotateInPlace();
	bool  CanTurnInPlace();
	bool  CanDynamicTransition();
	void  RotateInPlaceCheck();
	void  TurnInPlaceCheck();
	bool AngleInRange(float Angle, float MinAngle, float MaxAngle,float Buffer, bool IncreaseBuffer);

	float GetAnimCurveClamped(FName name, float Bias, float ClampMin, float ClampMax);
	FLeanAmount InterpLeanAmount(const FLeanAmount& Current, FLeanAmount& Target, float InterpSpeed, float DeltaTime);
	EMovementDirection CalculateMovementDirection();
	EMovementDirection CalculateQuadrant(EMovementDirection Current,float FRThreshold, 
		float FLThreshold, 
		float BRThreshold, 
		float BLThreshold,float Buffer,float Angle);
	FVector CalculateRelativeAccelerationAmount();
	FVelocityBlend CalculateVelocityBlend();
	FVelocityBlend InterpVelocityBlend(const FVelocityBlend& Current, const FVelocityBlend& Target, float InterpSpeed, float DeltaTime);
private:
	//References
	float DeltaTimeX;
	AQCharacter* Character;

	FPlayTransition PlayTransition;
	FPlayDynamicTransition PlayDynamicTransition;
private:
	//Bind Curves
	UPROPERTY(EditDefaultsOnly, Category = "Curve")
		UCurveVector* YawOffsetFB;
	UPROPERTY(EditDefaultsOnly, Category = "Curve")
		UCurveVector* YawOffsetLR;
	UPROPERTY(EditDefaultsOnly, Category = "Curve")
		UCurveFloat* StrideBlendNWalk;
	UPROPERTY(EditDefaultsOnly, Category = "Curve")
		UCurveFloat* StrideBlendNRun;
	UPROPERTY(EditDefaultsOnly, Category = "Curve")
		UCurveFloat* StrideBlendCWalk;
	UPROPERTY(EditDefaultsOnly, Category = "Curve")
		UCurveFloat* DiagonalScaleAmountCurve;

	UPROPERTY(EditDefaultsOnly, Category = "Config|Speed")
		float AnimatedCrouchSpeed = 150.f;
	UPROPERTY(EditDefaultsOnly, Category = "Config|Speed")
		float AnimatedWalkSpeed  = 150.f;
	UPROPERTY(EditDefaultsOnly, Category = "Config|Speed")
		float AnimatedRunSpeed	 = 350.f;
	UPROPERTY(EditDefaultsOnly, Category = "Config|Speed")
		float AnimatedSprintSpeed = 600.f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Config|Speed", meta = (AllowPrivateAccess = "true"))
		float TriggerPivotSpeedLimit = 200.f;
	UPROPERTY(EditDefaultsOnly, Category = "Config|Aiming")
		float SmoothedAimingRotationInterpSpeed = 10.f;
	UPROPERTY(EditDefaultsOnly, Category = "Config|Aiming")
		float InputYawOffsetInterpSpeed = 8.f;
	UPROPERTY(EditDefaultsOnly, Category = "Config|Turn In Place")
		float TurnCheckMinAngle = 45.f;
	UPROPERTY(EditDefaultsOnly, Category = "Config|Turn In Place")
		float Turn180Threshold = 130.f;
	UPROPERTY(EditDefaultsOnly, Category = "Config|Turn In Place")
		float AimYawRateLimit = 50.f;
	
	UPROPERTY(EditDefaultsOnly, Category = "Config|Turn In Place")
		float MinAngleDelay = 0.75f;
	UPROPERTY(EditDefaultsOnly, Category = "Config|Turn In Place")
		float MaxAngleDelay = 0.f;
	UPROPERTY(EditDefaultsOnly, Category = "Config|Turn In Place")
		FTurnInPlaceAsset N_TurnIP_L90;
	UPROPERTY(EditDefaultsOnly, Category = "Config|Turn In Place")
		FTurnInPlaceAsset N_TurnIP_R90;
	UPROPERTY(EditDefaultsOnly, Category = "Config|Turn In Place")
		FTurnInPlaceAsset N_TurnIP_L180;
	UPROPERTY(EditDefaultsOnly, Category = "Config|Turn In Place")
		FTurnInPlaceAsset N_TurnIP_R180;
	UPROPERTY(EditDefaultsOnly, Category = "Config|Turn In Place")
		FTurnInPlaceAsset CLF_TurnIP_L90;
	UPROPERTY(EditDefaultsOnly, Category = "Config|Turn In Place")
		FTurnInPlaceAsset CLF_TurnIP_R90;
	UPROPERTY(EditDefaultsOnly, Category = "Config|Turn In Place")
		FTurnInPlaceAsset CLF_TurnIP_L180;
	UPROPERTY(EditDefaultsOnly, Category = "Config|Turn In Place")
		FTurnInPlaceAsset CLF_TurnIP_R180;

	UPROPERTY(EditDefaultsOnly, Category = "Config|Rotate In Place")
		float RotateMinThreshold = -50.f;
	UPROPERTY(EditDefaultsOnly, Category = "Config|Rotate In Place")
		float RotateMaxThreshold = 50.f;
	UPROPERTY(EditDefaultsOnly, Category = "Config|Rotate In Place")
		float AimYawRateMinRange = 90.f;
	UPROPERTY(EditDefaultsOnly, Category = "Config|Rotate In Place")
		float AimYawRateMaxRange = 270.0;
	UPROPERTY(EditDefaultsOnly, Category = "Config|Rotate In Place")
		float MinPlayRate = 1.15f;
	UPROPERTY(EditDefaultsOnly, Category = "Config|Rotate In Place")
		float MaxPlayRate = 3.f;

	UPROPERTY(EditDefaultsOnly, Category = "Config|FootIK")
		float IK_TraceDistanceAboveFoot = 50.0f;
	UPROPERTY(EditDefaultsOnly, Category = "Config|FootIK")
		float IK_TraceDistanceBelowFoot = 45.0f;
	UPROPERTY(EditDefaultsOnly, Category = "Config|FootIK")
		float FootHeight = 13.5f;
		
private:
	void BindDeclares();
	void OnPlayDynamicTransition(float ReTriggerDelay, FDynamicMontageParams Parameters);
	void OnPlayTransition(FDynamicMontageParams Parameters);
	void OnJump();
protected:
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "PlayTransition"))
		void K2_PlayTransition(FDynamicMontageParams Parameters);
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "PlayDynamicTransition"))
		void K2_PlayDynamicTransition(float ReTriggerDelay, FDynamicMontageParams Parameters);
};


