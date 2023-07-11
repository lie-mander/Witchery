// Witchery. Copyright Liemander. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Weapons/WTRHitScanWeapon.h"
#include "WTRShotgun.generated.h"

UCLASS()
class WITCHERY_API AWTRShotgun : public AWTRHitScanWeapon
{
    GENERATED_BODY()

public:
    virtual void FireShotgun(const TArray<FVector_NetQuantize> HitTargets);
    void ShotgunTraceEndWithScatter(const FVector& HitTarget, TArray<FVector_NetQuantize>& HitTargets);

private:
    UPROPERTY(EditDefaultsOnly, Category = "WTR | Weapon properties")
    uint32 NumberOfShotgunShells = 12;

    void CalculateNumOfHits(
        const FVector& TraceStart, const FVector_NetQuantize& HitTarget, TMap<AWTRCharacter*, uint32>& HitsMap, FHitResult& FireHit);

    void HandleShotgunEffects(const FHitResult& FireHit);
    void ApplyDamageWithoutSSR(TPair<AWTRCharacter*, uint32>& Pair, AController* InstigatorController);
    void ApplyDamageWithSSR(const TArray<AWTRCharacter*>& HitCharacters, const FVector& TraceStart,
        const TArray<FVector_NetQuantize> HitTargets, AController* InstigatorController);
};
