// Witchery. Copyright Liemander. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "WTRTypes.h"
#include "WTRCombatComponent.generated.h"

class AWTRCharacter;
class AWTRWeapon;
class AWTRPlayerController;
class AWTR_HUD;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class WITCHERY_API UWTRCombatComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    friend class AWTRCharacter;

    UWTRCombatComponent();
    void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    void EquipWeapon(AWTRWeapon* WeaponToEquip);

protected:
    virtual void BeginPlay() override;

    void SetAiming(bool bAiming);
    void OnFireButtonPressed(bool bPressed);

    //////////
    // Multiplayer functions and callbacks
    //
    UFUNCTION()
    void OnRep_EquippedWeapon();

    UFUNCTION(Server, Reliable)
    void ServerSetAiming(bool bAiming);

    UFUNCTION(Server, Reliable)
    void ServerFire(const FVector_NetQuantize& TraceHitTarget);

    UFUNCTION(NetMulticast, Reliable)
    void MulticastFire(const FVector_NetQuantize& TraceHitTarget);

private:
    //////////
    // Multiplayer variables
    //
    UPROPERTY(ReplicatedUsing = OnRep_EquippedWeapon)
    AWTRWeapon* EquippedWeapon;

    UPROPERTY(Replicated)
    bool bIsAiming = false;

    //////////
    // Movement variables
    //
    UPROPERTY(EditDefaultsOnly, Category = "Movement")
    float BaseWalkSpeed = 600.f;

    UPROPERTY(EditDefaultsOnly, Category = "Movement")
    float AimWalkSpeed = 300.f;

    //////////
    // Shooting variables
    //
    UPROPERTY(EditDefaultsOnly, Category = "Shoot")
    float TraceRange = 300.f;

    UPROPERTY(EditDefaultsOnly, Category = "Shoot")
    FVector3d SpringArmOffsetWhileEquipped = FVector3d(-160.f, 0.f, 180.f);

    bool bFireButtonPressed = false;

    FVector HitTarget;

    //////////
    // Crosshair variables
    //
    UPROPERTY(EditDefaultsOnly, Category = "Crosshair | Base")
    float CrosshairSpread = 0.5f;

    UPROPERTY(EditDefaultsOnly, Category = "Crosshair | Air")
    float AirFactorSpread = 2.25f;

    UPROPERTY(EditDefaultsOnly, Category = "Crosshair | Air")
    float AirFactorSpeedUp = 2.25f;

    UPROPERTY(EditDefaultsOnly, Category = "Crosshair | Air")
    float AirFactorSpeedDown = 30.f;

    UPROPERTY(EditDefaultsOnly, Category = "Crosshair | Aim")
    float AimFactorSpread = 0.3f;

    UPROPERTY(EditDefaultsOnly, Category = "Crosshair | Aim")
    float AimFactorSpeedUp = 30.f;

    UPROPERTY(EditDefaultsOnly, Category = "Crosshair | Aim")
    float AimFactorSpeedDown = 30.f;

    UPROPERTY(EditDefaultsOnly, Category = "Crosshair | Aim")
    float DistanceFromCamera = 100.f;

    UPROPERTY(EditDefaultsOnly, Category = "Crosshair | Shooting")
    float ShootingFactorSpread = 0.75f;

    UPROPERTY(EditDefaultsOnly, Category = "Crosshair | Shooting")
    float ShootingFactorSpeedDown = 10.f;

    UPROPERTY(EditDefaultsOnly, Category = "Crosshair | Crouching")
    float CrouchingFactorSpread = 0.3f;

    UPROPERTY(EditDefaultsOnly, Category = "Crosshair | Crouching")
    float CrouchingFactorSpeedUp = 30.f;

    UPROPERTY(EditDefaultsOnly, Category = "Crosshair | Crouching")
    float CrouchingFactorSpeedDown = 30.f;

    UPROPERTY(EditDefaultsOnly, Category = "Crosshair | Color")
    FLinearColor CrosshairColorWithTarget = FLinearColor::Red;

    UPROPERTY(EditDefaultsOnly, Category = "Crosshair | Color")
    FLinearColor CrosshairColorWithoutTarget = FLinearColor::White;

    float CrosshairVelocityFactor = 0.f;
    float CrosshairAirFactor = 0.f;
    float CrosshairAimFactor = 0.f;
    float CrosshairShootingFactor = 0.f;
    float CrosshairCrouchingFactor = 0.f;

    FCrosshairHUDPackage HUDPackage;

    //////////
    // Zooming
    //
    UPROPERTY(EditDefaultsOnly, Category = "Zooming", meta = (ClampMin = "0.0", ClampMax = "90.0"))
    float DefaultZoomFOV = 30.f;

    UPROPERTY(EditDefaultsOnly, Category = "Zooming", meta = (ClampMin = "0.0", ClampMax = "90.0"))
    float ZoomInterpSpeed = 20.f;

    float CurrentZoomFOV = 0.f;

    //////////
    // Base variables
    //
    AWTRCharacter* Character;
    AWTRPlayerController* Controller;
    AWTR_HUD* HUD;

    //////////
    // Functions
    //
    void TraceFromScreen(FHitResult& TraceHitResult);
    void DrawCrosshair(float DeltaTime);
    void InterpFOV(float DeltaTime);
};
