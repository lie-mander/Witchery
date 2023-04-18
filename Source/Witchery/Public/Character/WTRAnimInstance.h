// Witchery. Copyright Liemander. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "WTRAnimInstance.generated.h"

UCLASS()
class WITCHERY_API UWTRAnimInstance : public UAnimInstance
{
    GENERATED_BODY()

public:
    virtual void NativeInitializeAnimation() override;
    virtual void NativeUpdateAnimation(float DeltaSeconds) override;

private:
    UPROPERTY(BlueprintReadOnly, Category = "Character", meta = (AllowPrivateAccess = true))
    class AWTRCharacter* Character = nullptr;

    UPROPERTY(BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = true))
    float Speed = 0.f;

    UPROPERTY(BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = true))
    float YawOffset = 0.f;

    UPROPERTY(BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = true))
    float Lean = 0.f;

    UPROPERTY(BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = true))
    bool bIsInAir = false;

    UPROPERTY(BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = true))
    bool bIsAccelerating = false;

    UPROPERTY(BlueprintReadOnly, Category = "Equip", meta = (AllowPrivateAccess = true))
    bool bIsEquippedWeapon = false;

    UPROPERTY(BlueprintReadOnly, Category = "Crouch", meta = (AllowPrivateAccess = true))
    bool bIsCrouched = false;

    UPROPERTY(BlueprintReadOnly, Category = "Aiming", meta = (AllowPrivateAccess = true))
    bool bIsAiming = false;

    UPROPERTY(BlueprintReadOnly, Category = "Aiming", meta = (AllowPrivateAccess = true))
    float AO_Yaw = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Aiming", meta = (AllowPrivateAccess = true))
    float AO_Pitch = 0.0f;

    FRotator CharacterRotationLastFrame;
    FRotator CharacterRotation;
    FRotator DeltaRotation;

    class UCharacterMovementComponent* Movement = nullptr;
};
