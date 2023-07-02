// Witchery. Copyright Liemander. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WTRTypes.h"
#include "WTRWeapon.generated.h"

class USphereComponent;
class USkeletalMeshComponent;
class UWidgetComponent;
class UTexture2D;
class AWTRBulletShell;
class AWTRCharacter;
class AWTRPlayerController;
class USoundCue;

UCLASS()
class WITCHERY_API AWTRWeapon : public AActor
{
    GENERATED_BODY()

public:
    AWTRWeapon();
    void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
    virtual void Tick(float DeltaTime) override;
    virtual void Fire(const FVector& HitTarget);
    virtual void StopFire();
    void Dropped();
    void AddAmmo(int32 AmmoToAdd);
    void EnableCustomDepth(bool bEnable);

    FORCEINLINE bool IsAutomatic() const { return bAutomaticWeapon; }
    FORCEINLINE bool IsEmpty() const { return Ammo <= 0; }
    FORCEINLINE bool IsFull() const { return Ammo == MagazineCapacity; }

    void SetHUDAmmo();
    void SetShowWidget(bool bShowWidget);
    void SetWeaponState(EWeaponState NewState);

    FORCEINLINE USphereComponent* GetAreaSphere() const { return AreaSphere; }
    FORCEINLINE USkeletalMeshComponent* GetWeaponMesh() const { return WeaponMesh; }
    FORCEINLINE EWeaponType GetWeaponType() const { return WeaponType; }
    FORCEINLINE float GetZoomedFOV() const { return ZoomedFOV; }
    FORCEINLINE float GetZoomInterpSpeed() const { return ZoomInterpSpeed; }
    FORCEINLINE float GetWeaponFiringDelay() const { return FireDelay; }
    FORCEINLINE int32 GetAmmo() const { return Ammo; }
    FORCEINLINE int32 GetMagazineCapacity() const { return MagazineCapacity; }

    /*
     * If weapon needs destroy when owner elim or weapon dropped
     */
    bool bNeedDestroy = false;

    /*
     * Crosshairs textures
     */
    UPROPERTY(EditDefaultsOnly, Category = "WTR | Crosshair")
    UTexture2D* CrosshairsCenter = nullptr;

    UPROPERTY(EditDefaultsOnly, Category = "WTR | Crosshair")
    UTexture2D* CrosshairsLeft = nullptr;

    UPROPERTY(EditDefaultsOnly, Category = "WTR | Crosshair")
    UTexture2D* CrosshairsRight = nullptr;

    UPROPERTY(EditDefaultsOnly, Category = "WTR | Crosshair")
    UTexture2D* CrosshairsTop = nullptr;

    UPROPERTY(EditDefaultsOnly, Category = "WTR | Crosshair")
    UTexture2D* CrosshairsBottom = nullptr;

    /*
     * Sounds
     */
    UPROPERTY(EditDefaultsOnly, Category = "WTR | Sounds")
    USoundCue* PickupSound;

protected:
    /*
     * Socket`s names
     */
    UPROPERTY(EditDefaultsOnly, Category = "WTR | Sockets")
    FName MuzzleSocketName = "MuzzleFlash";

    UPROPERTY(EditDefaultsOnly, Category = "WTR | Sockets")
    FName AmmoEjectSocketName = "AmmoEject";

    /*
     * Automatic properties
     */
    UPROPERTY(EditDefaultsOnly, Category = "WTR | Weapon Firing")
    float FireDelay = 0.25f;

    UPROPERTY(EditDefaultsOnly, Category = "WTR | Weapon Firing")
    bool bAutomaticWeapon = true;

    /*
    * Sounds
    */
    UPROPERTY(EditDefaultsOnly, Category = "WTR | Shoot")
    USoundCue* ShootSound;

    /*
    * Fix firing through static meshes, we want to decrease ammo, but don`t want to spawn projectiles
    */
    bool bOverlapOtherStaticMeshes = false;

    /*
     * Functions
     */
    virtual void BeginPlay() override;
    virtual void OnWeaponStateChanged();
    virtual void HandleStateEquipped();
    virtual void HandleStateEquippedSecond();
    virtual void HandleStateDropped();

    AController* GetOwnerPlayerController() const;

private:
    /*
     * Components
     */
    UPROPERTY(EditDefaultsOnly, Category = "WTR | Components")
    USkeletalMeshComponent* WeaponMesh;

    UPROPERTY(VisibleAnywhere, Category = "WTR | Components")
    USphereComponent* AreaSphere;

    UPROPERTY(VisibleAnywhere, Category = "WTR | Components")
    UWidgetComponent* PickupWidget;

    /*
     * Weapon properties
     */
    UPROPERTY(EditDefaultsOnly, Category = "WTR | Weapon Firing")
    UAnimSequence* FireAnimation;

    UPROPERTY(EditDefaultsOnly, Category = "WTR | Bullet Shells")
    TSubclassOf<AWTRBulletShell> BulletShellClass;

    UPROPERTY(EditDefaultsOnly, Category = "WTR | Bullet Shells", meta = (ClampMin = "0.0", ClampMax = "90.0"))
    float RandRollForShellsSpawn = 30.f;

    UPROPERTY(EditDefaultsOnly, Category = "WTR | Bullet Shells", meta = (ClampMin = "0.0", ClampMax = "90.0"))
    float RandPitchForShellsSpawn = 30.f;

    UPROPERTY(ReplicatedUsing = OnRep_WeaponState, VisibleAnywhere, Category = "WTR | Weapon properties")
    EWeaponState WeaponState;

    UPROPERTY(EditDefaultsOnly, Category = "WTR | Weapon properties")
    EWeaponType WeaponType;

    /*
     * Ammo
     */
    UPROPERTY(ReplicatedUsing = OnRep_Ammo, EditDefaultsOnly, Category = "WTR | Weapon properties")
    int32 Ammo = 10;

    UPROPERTY(EditDefaultsOnly, Category = "WTR | Weapon properties")
    int32 MagazineCapacity = 10;

    UFUNCTION()
    void OnRep_Ammo();

    void DecreaseAmmo();

    /*
     * Zooming
     */
    UPROPERTY(EditDefaultsOnly, Category = "WTR | Zooming", meta = (ClampMin = "0.0", ClampMax = "90.0"))
    float ZoomedFOV = 30.f;

    UPROPERTY(EditDefaultsOnly, Category = "WTR | Zooming", meta = (ClampMin = "0.0", ClampMax = "90.0"))
    float ZoomInterpSpeed = 20.f;

    /*
     * Base variables
     */

    UPROPERTY()
    AWTRCharacter* WTROwnerCharacter;

    UPROPERTY()
    AWTRPlayerController* WTROwnerPlayerController;

    /*
     * Callbacks
     */
    UFUNCTION()
    void OnRep_WeaponState();

    virtual void OnRep_Owner() override;

    UFUNCTION()
    void OnSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
        int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnSphereEndOverlap(
        UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

    UFUNCTION()
    void OnWeaponMeshBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
        int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnWeaponMeshEndOverlap(
        UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
};
