// Witchery. Copyright Liemander. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "WTRTypes.h"
#include "WTRCharacter.generated.h"

class USpringArmComponent;

UCLASS()
class WITCHERY_API AWTRCharacter : public ACharacter
{
    GENERATED_BODY()

public:
    AWTRCharacter();
    void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
    virtual void Tick(float DeltaTime) override;
    virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
    virtual void PostInitializeComponents() override;
    virtual void Jump() override;

    void PlayFireMontage(bool bAiming);

    bool IsWeaponEquipped() const;
    bool IsAiming() const;

    void SetOverlappingWeapon(AWTRWeapon* Weapon);

    FORCEINLINE float GetAO_Yaw() const { return AO_Yaw; }
    FORCEINLINE float GetAO_Pitch() const { return AO_Pitch; }
    FORCEINLINE ETurningInPlace GetTurningState() const { return TurningInPlace; }
    FORCEINLINE USpringArmComponent* GetSpringArm() const { return SpringArmComponent; }
    AWTRWeapon* GetEquippedWeapon() const;

protected:
    void MoveForward(float Amount);
    void MoveRight(float Amount);
    void Turn(float Amount);
    void LookUp(float Amount);
    void UpdateAimOffset(float DeltaTime);

    void OnEquipButtonPressed();
    void OnCrouchButtonPressed();
    void OnAimButtonPressed();
    void OnAimButtonReleased();
    void OnPauseButtonPressed();
    void OnFireButtonPressed();
    void OnFireButtonReleased();

    virtual void BeginPlay() override;

private:
    UPROPERTY(VisibleAnywhere, Category = "Camera")
    USpringArmComponent* SpringArmComponent;

    UPROPERTY(VisibleAnywhere, Category = "Camera")
    class UCameraComponent* CameraComponent;

    UPROPERTY(VisibleAnywhere, Category = "Combat")
    class UWTRCombatComponent* Combat;

    UPROPERTY(EditAnywhere, Category = "Combat")
    class UAnimMontage* FireWeaponMontage;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = "OverheadText")
    class UWidgetComponent* OverheadWidget;

    UPROPERTY(VisibleAnywhere, Category = "OverheadText")
    class UTextRenderComponent* OverheadText;

    UPROPERTY(ReplicatedUsing = OnRep_OverlappingWeapon)
    class AWTRWeapon* OverlappingWeapon;

    UPROPERTY(ReplicatedUsing = OnRep_Username)
    FString Username;

    UFUNCTION()
    void OnRep_Username();

    UFUNCTION()
    void OnRep_OverlappingWeapon(AWTRWeapon* LastWeapon);

    UFUNCTION(Server, Reliable)
    void Server_OnEquippedButtonPressed();

    UFUNCTION(Server, Reliable)
    void Server_SetUsername();

    ETurningInPlace TurningInPlace;

    float AO_Yaw = 0.f;
    float AO_Pitch = 0.f;
    float InterpAO_Yaw = 0.f;
    FRotator StartAimRotation;

    void SetTurningInPlace(float DeltaTime);
    void UpdateIfIsNotStanding();
};
