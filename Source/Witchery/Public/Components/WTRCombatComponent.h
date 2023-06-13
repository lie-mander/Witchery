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
    void OnFireButtonPressed(bool bPressed);
    void SetAiming(bool bAiming);
    void FinishReloading();

protected:
    virtual void BeginPlay() override;

    void Reload();
    void Fire();

    //////////
    // Multiplayer functions and callbacks
    //
    UFUNCTION()
    void OnRep_EquippedWeapon();

    UFUNCTION(Server, Reliable)
    void Server_SetAiming(bool bAiming);

    UFUNCTION(Server, Reliable)
    void Server_Reload();

    UFUNCTION(Server, Reliable)
    void Server_Fire(const FVector_NetQuantize& TraceHitTarget);

    UFUNCTION(NetMulticast, Reliable)
    void Multicast_Fire(const FVector_NetQuantize& TraceHitTarget);

private:
    //////////
    // Movement
    //
    UPROPERTY(ReplicatedUsing = OnRep_CombatState)
    ECombatState CombatState = ECombatState::ECS_Unoccupied;

    UPROPERTY(EditDefaultsOnly, Category = "WTR | Movement")
    float BaseWalkSpeed = 600.f;

    UPROPERTY(EditDefaultsOnly, Category = "WTR | Movement")
    float AimWalkSpeed = 300.f;

    UFUNCTION()
    void OnRep_CombatState();

    //////////
    // Shooting
    //
    UPROPERTY(EditDefaultsOnly, Category = "WTR | Shoot")
    FVector3d SpringArmOffsetWhileEquipped = FVector3d(0.f, 180.f, 0.f);

    UPROPERTY(Replicated)
    bool bIsAiming = false;

    bool bFireButtonPressed = false;
    bool bCanFire = true;

    FVector HitTarget;

    FTimerHandle FireTimerHandle;

    //////////
    // Carried ammo
    //
    UPROPERTY(EditDefaultsOnly, Category = "WTR | Carried ammo")
    int32 AssaultRifleCarrAmmo = 30;

    UPROPERTY(EditDefaultsOnly, Category = "WTR | Carried ammo")
    int32 RocketLauncherCarrAmmo = 4;

    UPROPERTY(EditDefaultsOnly, Category = "WTR | Carried ammo")
    int32 PistolCarrAmmo = 25;

    UPROPERTY(EditDefaultsOnly, Category = "WTR | Carried ammo")
    int32 SubmachineGunCarrAmmo = 35;

    UPROPERTY(EditDefaultsOnly, Category = "WTR | Carried ammo")
    int32 ShotgunCarrAmmo = 10;

    UPROPERTY(EditDefaultsOnly, Category = "WTR | Carried ammo")
    int32 SniperRifleCarrAmmo = 6;

    // Carried ammo for current equipped weapon
    UPROPERTY(ReplicatedUsing = OnRep_CarriedAmmo);
    int32 CarriedAmmo;

    TMap<EWeaponType, int32> CarriedAmmoByWeaponTypeMap;

    UFUNCTION()
    void OnRep_CarriedAmmo();

    void InitCarriedAmmoMap();

    //////////
    // Crosshair
    //
    UPROPERTY(EditDefaultsOnly, Category = "WTR | Crosshair | Base")
    float CrosshairSpread = 0.6f;

    UPROPERTY(EditDefaultsOnly, Category = "WTR | Crosshair | Air")
    float AirFactorSpread = 2.25f;

    UPROPERTY(EditDefaultsOnly, Category = "WTR | Crosshair | Air")
    float AirFactorSpeedUp = 2.25f;

    UPROPERTY(EditDefaultsOnly, Category = "WTR | Crosshair | Air")
    float AirFactorSpeedDown = 30.f;

    UPROPERTY(EditDefaultsOnly, Category = "WTR | Crosshair | Aim")
    float AimFactorSpread = 0.3f;

    UPROPERTY(EditDefaultsOnly, Category = "WTR | Crosshair | Aim")
    float AimFactorSpeedUp = 30.f;

    UPROPERTY(EditDefaultsOnly, Category = "WTR | Crosshair | Aim")
    float AimFactorSpeedDown = 30.f;

    UPROPERTY(EditDefaultsOnly, Category = "WTR | Crosshair | Enemy")
    float HasEnemyFactorSpread = 0.2f;

    UPROPERTY(EditDefaultsOnly, Category = "WTR | Crosshair | Enemy")
    float HasEnemyFactorSpeedUp = 30.f;

    UPROPERTY(EditDefaultsOnly, Category = "WTR | Crosshair | Enemy")
    float HasEnemyFactorSpeedDown = 30.f;

    UPROPERTY(EditDefaultsOnly, Category = "WTR | Crosshair | Aim")
    float DistanceFromCamera = 100.f;

    UPROPERTY(EditDefaultsOnly, Category = "WTR | Crosshair | Shooting")
    float ShootingFactorSpread = 0.75f;

    UPROPERTY(EditDefaultsOnly, Category = "WTR | Crosshair | Shooting")
    float ShootingFactorSpeedDown = 10.f;

    UPROPERTY(EditDefaultsOnly, Category = "WTR | Crosshair | Crouching")
    float CrouchingFactorSpread = 0.3f;

    UPROPERTY(EditDefaultsOnly, Category = "WTR | Crosshair | Crouching")
    float CrouchingFactorSpeedUp = 30.f;

    UPROPERTY(EditDefaultsOnly, Category = "WTR | Crosshair | Crouching")
    float CrouchingFactorSpeedDown = 30.f;

    UPROPERTY(EditDefaultsOnly, Category = "WTR | Crosshair | Color")
    FLinearColor CrosshairColorWithTarget = FLinearColor::Red;

    UPROPERTY(EditDefaultsOnly, Category = "WTR | Crosshair | Color")
    FLinearColor CrosshairColorWithoutTarget = FLinearColor::White;

    float CrosshairVelocityFactor = 0.f;
    float CrosshairAirFactor = 0.f;
    float CrosshairAimFactor = 0.f;
    float CrosshairHasEnemyFactor = 0.f;
    float CrosshairShootingFactor = 0.f;
    float CrosshairCrouchingFactor = 0.f;

    FCrosshairHUDPackage HUDPackage;

    //////////
    // Zooming
    //
    UPROPERTY(EditDefaultsOnly, Category = "WTR | Zooming", meta = (ClampMin = "0.0", ClampMax = "90.0"))
    float DefaultZoomFOV = 30.f;

    UPROPERTY(EditDefaultsOnly, Category = "WTR | Zooming", meta = (ClampMin = "0.0", ClampMax = "90.0"))
    float ZoomInterpSpeed = 20.f;

    float CurrentZoomFOV = 0.f;

    //////////
    // Base variables
    //
    UPROPERTY()
    AWTRCharacter* Character;

    UPROPERTY()
    AWTRPlayerController* Controller;

    UPROPERTY()
    AWTR_HUD* HUD;

    UPROPERTY(ReplicatedUsing = OnRep_EquippedWeapon)
    AWTRWeapon* EquippedWeapon;

    UPROPERTY()
    AWTRWeapon* LastEquippedWeapon;

    FHitResult TraceHitResult;

    //////////
    // Functions
    //
    void TraceFromScreen(FHitResult& TraceHitResult);
    void DrawCrosshair(float DeltaTime);
    void InterpFOV(float DeltaTime);
    void FireTimerStart();
    void FireTimerUpdate();
    void ReloadHandle();
    void SetHUDCarriedAmmo();
    bool CanFire() const;
    int32 AmmoToReload();
    void ReloadWeaponAndSubCarriedAmmo();
    void StopReloadWhileEquip();
};
