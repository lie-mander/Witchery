// Witchery. Copyright Liemander. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "WTRCharacter.generated.h"

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

protected:
    void MoveForward(float Amount);
    void MoveRight(float Amount);
    void Turn(float Amount);
    void LookUp(float Amount);

    void OnEquipButtonPressed();
    void OnCrouchButtonPressed();
    void OnAimButtonPressed();
    void OnAimButtonReleased();

    virtual void BeginPlay() override;

private:
    UPROPERTY(VisibleAnywhere, Category = "Camera")
    class USpringArmComponent* SpringArmComponent;

    UPROPERTY(VisibleAnywhere, Category = "Camera")
    class UCameraComponent* CameraComponent;

    UPROPERTY(VisibleAnywhere, Category = "Combat")
    class UWTRCombatComponent* CombatComponent;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
    class UWidgetComponent* OverheadWidget;

    UPROPERTY(ReplicatedUsing = OnRep_OverlappingWeapon)
    class AWTRWeapon* OverlappingWeapon;

    UFUNCTION()
    void OnRep_OverlappingWeapon(AWTRWeapon* LastWeapon);

    UFUNCTION(Server, Reliable)
    void Server_OnEquippedButtonPressed();

public:
    void SetOverlappingWeapon(AWTRWeapon* Weapon);
    bool IsWeaponEquipped() const;
    bool IsAiming() const;
};
