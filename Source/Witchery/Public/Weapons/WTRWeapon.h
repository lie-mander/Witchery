// Witchery. Copyright Liemander. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WTRWeapon.generated.h"

UENUM(BlueprintType)
enum class EWeaponState : uint8
{
    EWS_Initial UMETA(DisplayName = "Initial"),
    EWS_Equipped UMETA(DisplayName = "Equipped"),
    EWS_Dropped UMETA(DisplayName = "Dropped"),

    EWS_MAX UMETA(DisplayName = "MAX")
};

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

    void SetShowWidget(bool bShowWidget);
    void SetWeaponState(EWeaponState NewState);

    FORCEINLINE USphereComponent* GetAreaSphere() const { return AreaSphere; }

protected:
    virtual void BeginPlay() override;

private:
    UPROPERTY(VisibleAnywhere, Category = "Weapon properties")
    USkeletalMeshComponent* WeaponMesh;

    UPROPERTY(VisibleAnywhere, Category = "Weapon properties")
    USphereComponent* AreaSphere;

    UPROPERTY(VisibleAnywhere, Category = "Weapon properties")
    UWidgetComponent* PickupWidget;

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
