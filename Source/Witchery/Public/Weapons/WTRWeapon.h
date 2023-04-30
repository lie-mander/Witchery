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

UCLASS()
class WITCHERY_API AWTRWeapon : public AActor
{
    GENERATED_BODY()

public:
    AWTRWeapon();
    void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
    virtual void Tick(float DeltaTime) override;
    virtual void Fire(const FVector& HitTarget);

    void SetShowWidget(bool bShowWidget);
    void SetWeaponState(EWeaponState NewState);

    FORCEINLINE USphereComponent* GetAreaSphere() const { return AreaSphere; }
    FORCEINLINE USkeletalMeshComponent* GetWeaponMesh() const { return WeaponMesh; }

    UPROPERTY(EditDefaultsOnly, Category = "Crosshair")
    UTexture2D* CrosshairsCenter = nullptr;

    UPROPERTY(EditDefaultsOnly, Category = "Crosshair")
    UTexture2D* CrosshairsLeft = nullptr;

    UPROPERTY(EditDefaultsOnly, Category = "Crosshair")
    UTexture2D* CrosshairsRight = nullptr;

    UPROPERTY(EditDefaultsOnly, Category = "Crosshair")
    UTexture2D* CrosshairsTop = nullptr;

    UPROPERTY(EditDefaultsOnly, Category = "Crosshair")
    UTexture2D* CrosshairsBottom = nullptr;

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditDefaultsOnly, Category = "Sockets")
    FName MuzzleSocketName = "MuzzleFlash";

    UPROPERTY(EditDefaultsOnly, Category = "Sockets")
    FName AmmoEjectSocketName = "AmmoEject";

private:
    UPROPERTY(EditDefaultsOnly, Category = "Components")
    USkeletalMeshComponent* WeaponMesh;

    UPROPERTY(VisibleAnywhere, Category = "Components")
    USphereComponent* AreaSphere;

    UPROPERTY(VisibleAnywhere, Category = "Components")
    UWidgetComponent* PickupWidget;

    UPROPERTY(EditDefaultsOnly, Category = "Weapon Firing")
    UAnimSequence* FireAnimation;

    UPROPERTY(EditDefaultsOnly, Category = "Bullet Shells")
    TSubclassOf<AWTRBulletShell> BulletShellClass;

    UPROPERTY(EditDefaultsOnly, Category = "Bullet Shells", meta = (ClampMin = "0.0", ClampMax = "90.0"))
    float RandRollForShellsSpawn = 30.f;

    UPROPERTY(EditDefaultsOnly, Category = "Bullet Shells", meta = (ClampMin = "0.0", ClampMax = "90.0"))
    float RandPitchForShellsSpawn = 30.f;

    UPROPERTY(ReplicatedUsing = OnRep_WeaponState, VisibleAnywhere, Category = "Weapon properties")
    EWeaponState WeaponState;

    UFUNCTION()
    void OnRep_WeaponState();

    UFUNCTION()
    void OnSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
        int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnSphereEndOverlap(
        UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
};
