#pragma once

#include "WTRTypes.generated.h"

class UTexture2D;
class AWTRCharacter;

// Trace distance
#define TRACE_RANGE 20000.f

// Depth colors
#define CUSTOM_DEPTH_PURPLE 250
#define CUSTOM_DEPTH_BLUE 251
#define CUSTOM_DEPTH_TAN 252

// Collision
#define ECC_SkeletalMesh ECollisionChannel::ECC_GameTraceChannel1
#define ECC_HitBox ECollisionChannel::ECC_GameTraceChannel2

// Delegates
DECLARE_MULTICAST_DELEGATE_OneParam(FOnNotifyPlayed, class USkeletalMeshComponent*);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FIsPingHigh, bool, bHighPing);

    // Animation
    UENUM(BlueprintType) enum class ETurningInPlace : uint8
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
    EWT_Flamethrower UMETA(DisplayName = "Flamethrower"),

    EWT_MAX UMETA(DisplayName = "MAX")
};

UENUM(BlueprintType)
enum class EFireType : uint8
{
    EFT_HitScan UMETA(DisplayName = "HitScan weapon"),
    EFT_Projectile UMETA(DisplayName = "Projectile weapon"),
    EFT_Shotgun UMETA(DisplayName = "Shotgun weapon"),
    EFT_Flamethrower UMETA(DisplayName = "Flamethrower weapon"),

    EWS_MAX UMETA(DisplayName = "MAX")
};

UENUM(BlueprintType)
enum class EWeaponState : uint8
{
    EWS_Initial UMETA(DisplayName = "Initial"),
    EWS_Equipped UMETA(DisplayName = "Equipped"),
    EWS_EquippedSecond UMETA(DisplayName = "EquippedSecond"),
    EWS_Dropped UMETA(DisplayName = "Dropped"),

    EWS_MAX UMETA(DisplayName = "MAX")
};

UENUM(BlueprintType)
enum class ECombatState : uint8
{
    ECS_Unoccupied UMETA(DisplayName = "Unoccupied"),
    ECS_Reloading UMETA(DisplayName = "Reloading"),
    ECS_ThrowingGrenade UMETA(DisplayName = "Throwing grenade"),

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

/*
 * Server-side rewind
 */
USTRUCT(BlueprintType)
struct FBoxInformation
{
    GENERATED_USTRUCT_BODY()

    UPROPERTY()
    FVector Location = FVector::ZeroVector;

    UPROPERTY()
    FRotator Rotation = FRotator::ZeroRotator;

    UPROPERTY()
    FVector BoxExtent = FVector::ZeroVector;
};

USTRUCT(BlueprintType)
struct FFramePackage
{
    GENERATED_USTRUCT_BODY()

    UPROPERTY()
    float Time = 0.f;

    UPROPERTY()
    TMap<FName, FBoxInformation> FrameInfo;

    UPROPERTY()
    AWTRCharacter* OwnerCharacter;
};

USTRUCT(BlueprintType)
struct FServerSideRewindResult
{
    GENERATED_USTRUCT_BODY()

    UPROPERTY()
    bool bConfrimHit = false;

    UPROPERTY()
    bool bHeadshot = false;
};

USTRUCT(BlueprintType)
struct FShotgunServerSideRewindResult
{
    GENERATED_USTRUCT_BODY()

    UPROPERTY()
    TMap<AWTRCharacter*, uint32> HeadShots;

    UPROPERTY()
    TMap<AWTRCharacter*, uint32> BodyShots;
};
