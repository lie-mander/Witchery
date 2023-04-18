// Witchery. Copyright Liemander. All Rights Reserved.

#include "Character/WTRAnimInstance.h"
#include "Character/WTRCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"

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

    // Set offset yaw for strafing
    FRotator AimRotation = Character->GetBaseAimRotation();
    FRotator MovementRotation = UKismetMathLibrary::MakeRotFromX(Character->GetVelocity());
    FRotator DeltaRot = UKismetMathLibrary::NormalizedDeltaRotator(AimRotation, MovementRotation);
    DeltaRotation = FMath::RInterpTo(DeltaRotation, DeltaRot, DeltaSeconds, 6.f);
    YawOffset = DeltaRotation.Yaw;

    // Calculate and set lean use last and currecnt character rotation
    CharacterRotationLastFrame = CharacterRotation;
    CharacterRotation = Character->GetActorRotation();
    const FRotator Delta = UKismetMathLibrary::NormalizedDeltaRotator(CharacterRotation, CharacterRotationLastFrame);
    const float Target = Delta.Yaw / DeltaSeconds;
    const float Interp = FMath::FInterpTo(Lean, Target, DeltaSeconds, 6.f);
    Lean = FMath::Clamp(Interp, -90.f, 90.f);

    // Set yaw and pitch for aim offsets
    AO_Yaw = Character->GetAO_Yaw();
    AO_Pitch = Character->GetAO_Pitch();
}
