// Witchery. Copyright Liemander. All Rights Reserved.

#include "Components/WTRBuffComponent.h"
#include "Character/WTRCharacter.h"

UWTRBuffComponent::UWTRBuffComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
}

void UWTRBuffComponent::BeginPlay()
{
    Super::BeginPlay();
}

void UWTRBuffComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    HealInTick(DeltaTime);
}

void UWTRBuffComponent::Heal(float HealAmount, float HealTime) 
{
    HealRate = HealAmount / HealTime;
    AmountToHeal += HealAmount;
    bIsHealing = true;
}

void UWTRBuffComponent::HealInTick(float DeltaTime)
{
    if (!bIsHealing || !Character || Character->IsElimmed()) return;

    const float HealthAmountInTick = HealRate * DeltaTime;
    const float NewHealthAmount = FMath::Clamp(Character->GetHealth() + HealthAmountInTick, 0.f, Character->GetMaxHealth());

    Character->SetHealth(NewHealthAmount);
    Character->UpdateHUDHealth();

    if (AmountToHeal <= 0.f || Character->IsFullHealth())
    {
        AmountToHeal = 0.f;
        bIsHealing = false;
    }
}
