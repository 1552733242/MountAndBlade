#pragma once

UENUM(BlueprintType)
enum class ECharacterMovementGait : uint8
{
	Walk	UMETA(DisplayName = "Walk"),
	Run	UMETA(DisplayName = "Run"),
	Sprint UMETA(DisplayName = "Sprint"),
};
UENUM(BlueprintType)
enum class ECharacterMovementState : uint8
{
	None,
	InAir		UMETA(DisplayName = "InAir"),
	OnGround	UMETA(DisplayName = "OnGround")
};
UENUM(BlueprintType)
enum class ECharacterMovementRotationMode : uint8
{
	VelocityDirection	UMETA(DisplayName = "VelocityDirection"),
	LookingDirection	UMETA(DisplayName = "LookingDirection"),
	Aiming				UMETA(DisplayName = "Aiming")
};
UENUM(BlueprintType)
enum class ECharacterMovementStance : uint8
{
	Stance		UMETA(DisplayName = "Stance"),
	Crouching	UMETA(DisplayName = "Crouching"),
};
UENUM(BlueprintType)
enum class ECharacterMovementAction : uint8
{
	None		UMETA(DisplayName = "None"),
	LowMantle	UMETA(DisplayName = "LowMantle"),
	HighMantle	UMETA(DisplayName = "HighMantle"),
	Rolling		UMETA(DisplayName = "Rolling"),
	GettingUp		UMETA(DisplayName = "GettingUp"),
};
UENUM(BlueprintType)
enum class ECharacterViewMode : uint8
{
	ThirdPerson		UMETA(DisplayName = "ThirdPerson"),
	FirstPerson		UMETA(DisplayName = "FirstPerson"),
};
UENUM(BlueprintType)
enum class ECharacterOverlayState : uint8
{
	Default		UMETA(DisplayName = "Default"),
};
