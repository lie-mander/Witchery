#pragma once

#include "WTRTypes.generated.h"

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
enum class EWeaponState : uint8
{
    EWS_Initial UMETA(DisplayName = "Initial"),
    EWS_Equipped UMETA(DisplayName = "Equipped"),
    EWS_Dropped UMETA(DisplayName = "Dropped"),

    EWS_MAX UMETA(DisplayName = "MAX")
};

USTRUCT(BlueprintType)
struct FCrosshairHUDPackage
{
    GENERATED_USTRUCT_BODY()

    class UTexture2D* CrosshairsCenter = nullptr;
    UTexture2D* CrosshairsLeft = nullptr;
    UTexture2D* CrosshairsRight = nullptr;
    UTexture2D* CrosshairsTop = nullptr;
    UTexture2D* CrosshairsBottom = nullptr;
};
