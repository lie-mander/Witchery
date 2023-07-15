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
class AWTRProjectile;
class USoundCue;

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
    void JumpToShotgunEnd();
    void AddPickupAmmo(EWeaponType Type, int32 Ammo);

    FORCEINLINE int32 GetCurrentGrenades() const { return Grenades; }
    FORCEINLINE int32 GetCarriedAmmo() const { return CarriedAmmo; }
    EWeaponType GetEquippedWeaponType() const;
    int32 GetEquippedWeaponAmmo() const;

    UFUNCTION(BlueprintCallable)
    void ShotgunShellReload();

    UFUNCTION(BlueprintCallable)
    void ThrowGrenadeFinished();

    UFUNCTION(BlueprintCallable)
    void LaunchGrenade();

    UFUNCTION(BlueprintCallable)
    void SwapFinished();

    UFUNCTION(BlueprintCallable)
    void SwapAttachedWeapons();

protected:
    virtual void BeginPlay() override;

    void Reload();
    void Fire();
    void LocalFire(const FVector_NetQuantize& TraceHitTarget);
    void LocalFireShotgun(const TArray<FVector_NetQuantize>& TraceHitTargets);
    void LocalStopFire();

    void EquipFirstWeapon(AWTRWeapon* WeaponToEquip);
    void EquipSecondWeapon(AWTRWeapon* WeaponToEquip);

    /*
     * Multiplayer functions and callbacks
     */
    UFUNCTION()
    void OnRep_EquippedWeapon();

    UFUNCTION()
    void OnRep_SecondWeapon();

    UFUNCTION(Server, Reliable)
    void Server_SetAiming(bool bAiming);

    UFUNCTION(Server, Reliable)
    void Server_Reload();

    UFUNCTION(Server, Reliable, WithValidation)
    void Server_Fire(const FVector_NetQuantize& TraceHitTarget, float Check_FireDelay);

    UFUNCTION(Server, Reliable, WithValidation)
    void Server_FireShotgun(const TArray<FVector_NetQuantize>& TraceHitTargets, float Check_FireDelay);

    UFUNCTION(Server, Reliable)
    void Server_StopFire();

    UFUNCTION(NetMulticast, Reliable)
    void Multicast_Fire(const FVector_NetQuantize& TraceHitTarget);

    UFUNCTION(NetMulticast, Reliable)
    void Multicast_FireShotgun(const TArray<FVector_NetQuantize>& TraceHitTargets);

    UFUNCTION(NetMulticast, Reliable)
    void Multicast_StopFire();

    UFUNCTION(Server, Reliable)
    void Server_ThrowGrenade();

    UFUNCTION(Server, Reliable)
    void Server_LaunchGrenade(const FVector_NetQuantize& Target);

private:
    /*
     * Movement
     */
    UPROPERTY(ReplicatedUsing = OnRep_CombatState)
    ECombatState CombatState = ECombatState::ECS_Unoccupied;

    UPROPERTY(EditDefaultsOnly, Category = "WTR | Movement")
    float BaseWalkSpeed = 600.f;

    UPROPERTY(EditDefaultsOnly, Category = "WTR | Movement")
    float AimWalkSpeed = 300.f;

    UFUNCTION()
    void OnRep_CombatState();

    /*
     * Shooting
     */
    UPROPERTY(EditDefaultsOnly, Category = "WTR | Shoot")
    FVector3d SpringArmOffsetWhileEquipped = FVector3d(0.f, 180.f, 0.f);

    UPROPERTY(ReplicatedUsing = OnRep_IsAiming)
    bool bIsAiming = false;

    bool bAimButtonPressed = false;
    bool bFireButtonPressed = false;
    bool bCanFire = true;
    bool bLocallyReloading = false;

    FVector HitTarget;
    FTimerHandle FireTimerHandle;

    UFUNCTION()
    void OnRep_IsAiming();

    /*
     * Carried ammo
     */
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

    UPROPERTY(EditDefaultsOnly, Category = "WTR | Carried ammo")
    int32 GrenadeLauncherCarrAmmo = 5;

    UPROPERTY(EditDefaultsOnly, Category = "WTR | Carried ammo")
    int32 FlamethrowerCarrAmmo = 90;

    UPROPERTY(EditDefaultsOnly, Category = "WTR | Max Carried ammo")
    int32 Max_AssaultRifleCarrAmmo = 90;

    UPROPERTY(EditDefaultsOnly, Category = "WTR | Max Carried ammo")
    int32 Max_RocketLauncherCarrAmmo = 9;

    UPROPERTY(EditDefaultsOnly, Category = "WTR | Max Carried ammo")
    int32 Max_PistolCarrAmmo = 70;

    UPROPERTY(EditDefaultsOnly, Category = "WTR | Max Carried ammo")
    int32 Max_SubmachineGunCarrAmmo = 90;

    UPROPERTY(EditDefaultsOnly, Category = "WTR | Max Carried ammo")
    int32 Max_ShotgunCarrAmmo = 30;

    UPROPERTY(EditDefaultsOnly, Category = "WTR | Max Carried ammo")
    int32 Max_SniperRifleCarrAmmo = 12;

    UPROPERTY(EditDefaultsOnly, Category = "WTR | Max Carried ammo")
    int32 Max_GrenadeLauncherCarrAmmo = 10;

    UPROPERTY(EditDefaultsOnly, Category = "WTR | Max Carried ammo")
    int32 Max_FlamethrowerCarrAmmo = 100;

    // Carried ammo for current equipped weapon
    UPROPERTY(ReplicatedUsing = OnRep_CarriedAmmo);
    int32 CarriedAmmo;

    TMap<EWeaponType, int32> CarriedAmmoByWeaponTypeMap;

    UFUNCTION()
    void OnRep_CarriedAmmo();

    void InitCarriedAmmoMap();

    /*
     * Crosshair
     */
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

    /*
     * Zooming
     */
    UPROPERTY(EditDefaultsOnly, Category = "WTR | Zooming", meta = (ClampMin = "0.0", ClampMax = "90.0"))
    float DefaultZoomFOV = 30.f;

    UPROPERTY(EditDefaultsOnly, Category = "WTR | Zooming", meta = (ClampMin = "0.0", ClampMax = "90.0"))
    float ZoomInterpSpeed = 20.f;

    float CurrentZoomFOV = 0.f;

    /*
     * Grenades
     */
    UPROPERTY(EditDefaultsOnly, Category = "WTR | Grenades")
    TSubclassOf<AWTRProjectile> GrenadeClass;

    UPROPERTY(ReplicatedUsing = OnRep_Grenades, EditDefaultsOnly, Category = "WTR | Grenades")
    int32 Grenades = 4;

    UPROPERTY(EditDefaultsOnly, Category = "WTR | Grenades")
    int32 MaxGrenades = 4;

    UFUNCTION()
    void OnRep_Grenades();

    /*
    * Sounds
    */
    UPROPERTY(EditDefaultsOnly, Category = "WTR | Sounds")
    USoundCue* EmptyWeapon;

    /*
     * Base variables
     */
    UPROPERTY()
    AWTRCharacter* Character;

    UPROPERTY()
    AWTRPlayerController* Controller;

    UPROPERTY()
    AWTR_HUD* HUD;

    UPROPERTY(ReplicatedUsing = OnRep_EquippedWeapon)
    AWTRWeapon* EquippedWeapon;

    UPROPERTY(ReplicatedUsing = OnRep_SecondWeapon)
    AWTRWeapon* SecondWeapon;

    UPROPERTY()
    AWTRWeapon* LastEquippedWeapon;

    UPROPERTY(EditDefaultsOnly, Category = "WTR | Default weapon")
    TSubclassOf<AWTRWeapon> DefautlWeaponClass;

    FHitResult TraceHitResult;

    /*
    * Functions
    */
    void TraceFromScreen(FHitResult& TraceHitResult);
    void DrawCrosshair(float DeltaTime);
    void InterpFOV(float DeltaTime);
    void FireTimerStart();
    void FireTimerUpdate();
    void ReloadHandle();
    void SetHUDCarriedAmmo();
    void ReloadWeaponAndSubCarriedAmmo();
    void ReloadShotgunAndSubCarriedAmmo();
    void StopReloadWhileEquip();
    void ThrowGrenade();
    void DroppedWeapon(AWTRWeapon* Weapon);
    void UpdateCarriedAmmoAndHUD();
    void PlayPickupSound(AWTRWeapon* WeaponToPickup);
    void ReloadEmptyWeapon();
    void SetCharacterSettingsWhenEquip();
    void AttachWeaponByTypeToRightHand(AWTRWeapon* WeaponToAttach);
    void AttachActorToRightHand(AActor* ActorToAttach);
    void AttachActorToFlamethrowerRightHand(AActor* ActorToAttach);
    void AttachActorToLeftHand(AActor* ActorToAttach);
    void AttachActorToBackpack(AActor* ActorToAttach);
    void UpdateHUDWeaponType();
    void UpdateHUDAmmo();
    void UpdateHUDGrenades();
    void SetShowGrenadeMesh(bool bShow);
    void SpawnAndEquipDefaultWeapon();
    void DropOrDestroyWeapon(AWTRWeapon* Weapon);
    void SwapWeapon();
    void HandleSwapWeapon();
    void FireByWeaponFireType();
    void HandleHitScanWeaponFire();
    void HandleProjectileWeaponFire();
    void HandleShotgunWeaponFire();
    void HandleFlamethrowerWeaponFire();
    bool CanFire() const;
    bool CanSwapWeapon() const;
    int32 AmmoToReload();
};
