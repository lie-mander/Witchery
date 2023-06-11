// Witchery. Copyright Liemander. All Rights Reserved.

#include "Components/WTRRocketMovementComponent.h"

UWTRRocketMovementComponent::EHandleBlockingHitResult UWTRRocketMovementComponent::HandleBlockingHit(
    const FHitResult& Hit, float TimeTick, const FVector& MoveDelta, float& SubTickTimeRemaining)
{
    Super::HandleBlockingHit(Hit, TimeTick, MoveDelta, SubTickTimeRemaining);
    return EHandleBlockingHitResult::AdvanceNextSubstep;
}

void UWTRRocketMovementComponent::HandleImpact(const FHitResult& Hit, float TimeSlice, const FVector& MoveDelta)
{
    // Must not calculate bounces
    return;
}
