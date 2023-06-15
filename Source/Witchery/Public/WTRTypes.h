#pragma once

#include "WTRTypes.generated.h"

class UTexture2D;

// Trace distance
#define TRACE_RANGE 20000.f

// Collision
#define ECC_SkeletalMesh ECollisionChannel::ECC_GameTraceChannel1

// Delegates
DECLARE_MULTICAST_DELEGATE_OneParam(FOnNotifyPlayed, class USkeletalMeshComponent*)

// Animation
UENUM(BlueprintType)
enum class ETurningInPlace : uint8
{
    ETIP_Right UMETA(DisplayName = "Turn right"),
    ETIP_Left UMETA(DisplayName = "Turn left"),
    ETIP_NotTurning UMETA(DisplayName = "Not turning"),

    ETIP_MAX UMETA(DisplayName = "MAX"),
};

// Weapon
UENUM(BlueprintType)
enum class EWeaponType : uint8
{
    EWT_AssaultRifle UMETA(DisplayName = "Assault Rifle"),
    EWT_RocketLauncher UMETA(DisplayName = "Rocket Launcher"),
    EWT_Pistol UMETA(DisplayName = "Pistol"),
    EWT_SubmachineGun UMETA(DisplayName = "SubmachineGun"),
    EWT_Shotgun UMETA(DisplayName = "Shotgun"),
    EWT_SniperRifle UMETA(DisplayName = "SniperRifle"),
    EWT_GrenadeLauncher UMETA(DisplayName = "GrenadeLauncher"),

    EWT_MAX UMETA(DisplayName = "MAX")
};

UENUM(BlueprintType)
enum class EWeaponState : uint8
{
    EWS_Initial UMETA(DisplayName = "Initial"),
    EWS_Equipped UMETA(DisplayName = "Equipped"),
    EWS_Dropped UMETA(DisplayName = "Dropped"),

    EWS_MAX UMETA(DisplayName = "MAX")
};

UENUM(BlueprintType)
enum class ECombatState : uint8
{
    ECS_Unoccupied UMETA(DisplayName = "Unoccupied"),
    ECS_Reloading UMETA(DisplayName = "Reloading"),

    ECS_MAX UMETA(DisplayName = "MAX")
};

USTRUCT(BlueprintType)
struct FCrosshairHUDPackage
{
    GENERATED_USTRUCT_BODY()

    UPROPERTY()
    UTexture2D* CrosshairsCenter = nullptr;

    UPROPERTY()
    UTexture2D* CrosshairsLeft = nullptr;

    UPROPERTY()
    UTexture2D* CrosshairsRight = nullptr;

    UPROPERTY()
    UTexture2D* CrosshairsTop = nullptr;

    UPROPERTY()
    UTexture2D* CrosshairsBottom = nullptr;

    float CrosshairSpread = 0.0f;
    FLinearColor CrosshairColor = FLinearColor::White;
};
