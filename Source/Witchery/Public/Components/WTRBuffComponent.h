// Witchery. Copyright Liemander. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "WTRBuffComponent.generated.h"

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class WITCHERY_API UWTRBuffComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    friend class AWTRCharacter;

    UWTRBuffComponent();
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    void Heal(float HealAmount, float HealTime);

protected:
    virtual void BeginPlay() override;

private:
    UPROPERTY()
    AWTRCharacter* Character;

    bool bIsHealing = false;
    float AmountToHeal = 0.f;
    float HealRate = 0.f;

    void HealInTick(float DeltaTime);
};
