// Witchery. Copyright Liemander. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WTRTypes.h"
#include "WTRWeapon.generated.h"

class USphereComponent;
class USkeletalMeshComponent;
class UWidgetComponent;

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

    UPROPERTY(EditDefaultsOnly, Category = "Weapon Firing")
    TSubclassOf<class AWTRBulletShell> BulletShellClass;

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
