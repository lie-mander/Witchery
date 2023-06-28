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
    void SpeedBuff(float BuffBaseSpeed, float BuffCrouchSpeed, float BuffTime);
    void JumpBuff(float BuffJumpZVelocity, float BuffTime);
    void ShieldBuff(float ShieldAmount, float BuffTime);

protected:
    virtual void BeginPlay() override;

private:
    UPROPERTY()
    AWTRCharacter* Character;

    /*
    * Heal buff
    */
    bool bIsHealing = false;
    float AmountToHeal = 0.f;
    float HealRate = 0.f;

    void HealInTick(float DeltaTime);

    /*
    * Speed buff
    */
    float InitialBaseSpeed = 0.f;
    float InitialCrouchSpeed = 0.f;

    FTimerHandle SpeedBuffTimerHandle;

    UFUNCTION(NetMulticast, Reliable)
    void Multicast_SpeedBuff(float BaseSpeed, float CrouchSpeed);

    void ResetSpeed();

    /*
    * Jump buff
    */
    float InitialJumpZVelocity = 0.f;

    FTimerHandle JumpBuffTimerHandle;

    UFUNCTION(NetMulticast, Reliable)
    void Multicast_JumpBuff(float JumpVelocity);

    void ResetJump();

    /*
    * Shield buff
    */
    bool bIsShieldAdding = false;
    float ShieldToAdd = 0.f;
    float ShieldRate = 0.f;

    void ShieldAddInTick(float DeltaTime);
};
