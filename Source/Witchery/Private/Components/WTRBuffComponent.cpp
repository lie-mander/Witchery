// Witchery. Copyright Liemander. All Rights Reserved.

#include "Components/WTRBuffComponent.h"
#include "Character/WTRCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"

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

void UWTRBuffComponent::SpeedBuff(float BuffBaseSpeed, float BuffCrouchSpeed, float BuffTime)
{
    if (!Character || !Character->GetCharacterMovement()) return;

    Character->GetCharacterMovement()->MaxWalkSpeed = BuffBaseSpeed;
    Character->GetCharacterMovement()->MaxWalkSpeedCrouched = BuffCrouchSpeed;

    Character->GetWorldTimerManager().SetTimer(  //
        SpeedBuffTimerHandle,                    //
        this,                                    //
        &UWTRBuffComponent::ResetSpeed,          //
        BuffTime                                 //
    );

    Multicast_SpeedBuff(BuffBaseSpeed, BuffCrouchSpeed);
}

void UWTRBuffComponent::ResetSpeed()
{
    if (!Character || !Character->GetCharacterMovement()) return;

    Character->GetCharacterMovement()->MaxWalkSpeed = InitialBaseSpeed;
    Character->GetCharacterMovement()->MaxWalkSpeedCrouched = InitialCrouchSpeed;

    Multicast_SpeedBuff(InitialBaseSpeed, InitialCrouchSpeed);
}

void UWTRBuffComponent::Multicast_SpeedBuff_Implementation(float BaseSpeed, float CrouchSpeed)
{
    if (!Character || !Character->GetCharacterMovement()) return;

    Character->GetCharacterMovement()->MaxWalkSpeed = BaseSpeed;
    Character->GetCharacterMovement()->MaxWalkSpeedCrouched = CrouchSpeed;
}

void UWTRBuffComponent::JumpBuff(float BuffJumpZVelocity, float BuffTime)
{
    if (!Character || !Character->GetCharacterMovement()) return;

    Character->GetCharacterMovement()->JumpZVelocity = BuffJumpZVelocity;

    Character->GetWorldTimerManager().SetTimer(  //
        JumpBuffTimerHandle,                     //
        this,                                    //
        &UWTRBuffComponent::ResetJump,           //
        BuffTime                                 //
    );

    Multicast_JumpBuff(BuffJumpZVelocity);
}

void UWTRBuffComponent::ResetJump()
{
    if (!Character || !Character->GetCharacterMovement()) return;

    Character->GetCharacterMovement()->JumpZVelocity = InitialJumpZVelocity;

    Multicast_JumpBuff(InitialJumpZVelocity);
}

void UWTRBuffComponent::Multicast_JumpBuff_Implementation(float JumpVelocity) 
{
    if (!Character || !Character->GetCharacterMovement()) return;

    Character->GetCharacterMovement()->JumpZVelocity = JumpVelocity;
}
