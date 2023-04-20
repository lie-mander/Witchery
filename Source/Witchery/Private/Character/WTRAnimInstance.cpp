// Witchery. Copyright Liemander. All Rights Reserved.

#include "Character/WTRAnimInstance.h"
#include "Character/WTRCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Weapons/WTRWeapon.h"

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

    // Set character`s current weapon
    EquippedWeapon = Character->GetEquippedWeapon();

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

    // For FABRIK
    if (bIsEquippedWeapon && EquippedWeapon && EquippedWeapon->GetWeaponMesh() && Character->GetMesh())
    {
        // Set world transform of LeftHandSocket on equipped weapon (all weapons must be have this socket)
        LeftHandTransform =
            EquippedWeapon->GetWeaponMesh()->GetSocketTransform(FName("LeftHandSocket"), ERelativeTransformSpace::RTS_World);

        FVector OutPosition;
        FRotator OutRotation;

        // Transform world location and rotation to bone (local) space and save it in OutPosition and OutRotation
        Character->GetMesh()->TransformToBoneSpace(
            FName("hand_r"), LeftHandTransform.GetLocation(), FRotator::ZeroRotator, OutPosition, OutRotation);

        // Set completed variables to LeftHandTransform for using in blueprints
        LeftHandTransform.SetLocation(OutPosition);
        LeftHandTransform.SetRotation(FQuat4d(OutRotation));
    }
}
