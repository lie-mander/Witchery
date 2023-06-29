// Witchery. Copyright Liemander. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Pickup.generated.h"

class USphereComponent;
class USoundCue;
class UStaticMeshComponent;
class UNiagaraComponent;
class UNiagaraSystem;

UCLASS()
class WITCHERY_API APickup : public AActor
{
    GENERATED_BODY()

public:
    APickup();
    virtual void Tick(float DeltaTime) override;

protected:
    virtual void BeginPlay() override;
    virtual void Destroyed() override;

    UFUNCTION()
    virtual void OnSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
        int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

private:
    UPROPERTY(VisibleAnywhere, Category = "WTR | Components")
    USphereComponent* AreaSphere;

    UPROPERTY(VisibleAnywhere, Category = "WTR | Components")
    UStaticMeshComponent* PickupMesh;

    UPROPERTY(VisibleAnywhere, Category = "WTR | Components")
    UNiagaraComponent* NiagaraComponent;

    UPROPERTY(EditDefaultsOnly, Category = "WTR | Sound")
    USoundCue* PickupSound;

    UPROPERTY(EditDefaultsOnly, Category = "WTR | Movement")
    float BaseTurnRate = 45.f;

    UPROPERTY(EditDefaultsOnly, Category = "WTR | FX")
    UNiagaraSystem* PickupFX;

    FTimerHandle BindOverlapTimerHandle;
    float BindOverlapDelay = 0.25f;

    void BindOverlapTimerStart();
    void BindOverlapTimerFinished();
    void EnableCustomDepth(bool bEnable);
};
