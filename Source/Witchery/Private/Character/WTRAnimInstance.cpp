// Witchery. Copyright Liemander. All Rights Reserved.

#include "Character/WTRAnimInstance.h"
#include "Character/WTRCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"

void UWTRAnimInstance::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();

    Character = Cast<AWTRCharacter>(TryGetPawnOwner());
    if (Character)
    {
        Movement = Character->GetCharacterMovement();
    }
}

void UWTRAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
    Super::NativeUpdateAnimation(DeltaSeconds);

    // Check character with movement
    if (!Character)
    {
        Character = Cast<AWTRCharacter>(TryGetPawnOwner());
        if (!Character) return;

        Movement = Character->GetCharacterMovement();
        if (!Movement) return;
    }

    // Set character speed
    FVector Velocity = Character->GetVelocity();
    Velocity.Z = 0.f;
    Speed = Velocity.Size();

    // Set is the character in the air
    bIsInAir = Movement->IsFalling();

    // Set does the character accelerate
    bIsAccelerating = Movement->GetCurrentAcceleration().Size() ? true : false;

    // Set is the character has weapon
    bIsEquippedWeapon = Character->IsWeaponEquipped();

    // Set is the character is crouched
    bIsCrouched = Character->bIsCrouched;

    // Set is the character is aiming
    bIsAiming = Character->IsAiming();
}
