// Witchery. Copyright Liemander. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "WTRRocketMovementComponent.generated.h"

UCLASS()
class WITCHERY_API UWTRRocketMovementComponent : public UProjectileMovementComponent
{
    GENERATED_BODY()

protected:
    virtual EHandleBlockingHitResult HandleBlockingHit(
        const FHitResult& Hit, float TimeTick, const FVector& MoveDelta, float& SubTickTimeRemaining);
    virtual void HandleImpact(const FHitResult& Hit, float TimeSlice = 0.f, const FVector& MoveDelta = FVector::ZeroVector) override;
};
