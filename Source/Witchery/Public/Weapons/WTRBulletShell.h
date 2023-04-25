// Witchery. Copyright Liemander. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WTRBulletShell.generated.h"

UCLASS()
class WITCHERY_API AWTRBulletShell : public AActor
{
    GENERATED_BODY()

public:
    AWTRBulletShell();

protected:
    virtual void BeginPlay() override;

    UFUNCTION()
    virtual void OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse,
        const FHitResult& Hit);

private:
    UPROPERTY(VisibleAnywhere, Category = "Components")
    class UStaticMeshComponent* BulletMesh;

    UPROPERTY(EditDefaultsOnly, Category = "Sounds")
    class USoundCue* GroundHitSound;

    UPROPERTY(EditDefaultsOnly, Category = "Speed", meta = (ClampMin = "0.0"))
    float EjectImpulse = 10.f;

    bool bSingleSound = true;
};
