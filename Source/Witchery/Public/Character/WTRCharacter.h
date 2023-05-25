// Witchery. Copyright Liemander. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "WTRTypes.h"
#include "Interfaces/InteractWithCrosshairInterface.h"
#include "WTRCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UWTRCombatComponent;
class UAnimMontage;
class UWidgetComponent;
class UTextRenderComponent;
class AWTRWeapon;
class AWTRPlayerController;

UCLASS()
class WITCHERY_API AWTRCharacter : public ACharacter, public IInteractWithCrosshairInterface
{
    GENERATED_BODY()

public:
    AWTRCharacter();
    void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
    virtual void Tick(float DeltaTime) override;
    virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
    virtual void PostInitializeComponents() override;
    virtual void Jump() override;

    virtual void OnRep_ReplicateMovement() override;

    void PlayFireMontage(bool bAiming);

    bool IsWeaponEquipped() const;
    bool IsAiming() const;
    FORCEINLINE bool ShouldRotateRootBone() const { return bRotateRootBone; }

    void SetOverlappingWeapon(AWTRWeapon* Weapon);

    FORCEINLINE float GetAO_Yaw() const { return AO_Yaw; }
    FORCEINLINE float GetAO_Pitch() const { return AO_Pitch; }
    FORCEINLINE ETurningInPlace GetTurningState() const { return TurningInPlace; }
    FORCEINLINE USpringArmComponent* GetSpringArm() const { return SpringArmComponent; }
    FORCEINLINE UCameraComponent* GetCameraComponent() const { return CameraComponent; }
    AWTRWeapon* GetEquippedWeapon() const;
    FVector GetHitTarget() const;

protected:
    //////////
    // Input functions and callbacks
    //
    void MoveForward(float Amount);
    void MoveRight(float Amount);
    void Turn(float Amount);
    void LookUp(float Amount);
    void UpdateAimOffset(float DeltaTime);
    void CalculateAO_Pitch();
    void SimProxiesTurn();

    void OnEquipButtonPressed();
    void OnCrouchButtonPressed();
    void OnAimButtonPressed();
    void OnAimButtonReleased();
    void OnPauseButtonPressed();
    void OnFireButtonPressed();
    void OnFireButtonReleased();

    //////////
    // Other functions
    //
    virtual void BeginPlay() override;

private:
    //////////
    // Components
    //
    UPROPERTY(VisibleAnywhere, Category = "Camera")
    USpringArmComponent* SpringArmComponent;

    UPROPERTY(VisibleAnywhere, Category = "Camera")
    UCameraComponent* CameraComponent;

    UPROPERTY(VisibleAnywhere, Category = "Combat")
    UWTRCombatComponent* Combat;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = "OverheadWidget")
    UWidgetComponent* OverheadWidget;

    UPROPERTY(VisibleAnywhere, Category = "OverheadText")
    UTextRenderComponent* OverheadText;

    //////////
    // Multiplayer variables
    //
    UPROPERTY(ReplicatedUsing = OnRep_OverlappingWeapon)
    AWTRWeapon* OverlappingWeapon;

    UPROPERTY(ReplicatedUsing = OnRep_Username)
    FString Username;

    //////////
    // Animation
    //
    UPROPERTY(EditDefaultsOnly, Category = "Combat")
    UAnimMontage* FireWeaponMontage;

    UPROPERTY(EditDefaultsOnly, Category = "Combat")
    UAnimMontage* HitReactMontage;

    void PlayHitReactMontage();

    //////////
    // Movement
    //
    UPROPERTY(EditDefaultsOnly, Category = "Movement")
    float AngleToTurn = 80.f;

    UPROPERTY(EditDefaultsOnly, Category = "Movement", meta = (ClampMin = 0.0))
    float SimProxyTurnThreshold = 10.f;

    float AO_Yaw = 0.f;
    float AO_Pitch = 0.f;
    float InterpAO_Yaw = 0.f;

    ETurningInPlace TurningInPlace;
    FRotator StartAimRotation;

    bool bRotateRootBone = false;
    float SimProxyDeltaYaw = 0.0f;
    float TimeSinceLastMovementReplication = 0.0f;
    FRotator SimProxyLastFrameRotation;
    FRotator SimProxyRotation;

    //////////
    // Health
    //
    UPROPERTY(EditDefaultsOnly, Category = "Player Stats")
    float MaxHealth = 100.f;

    UPROPERTY(ReplicatedUsing = OnRep_Health, VisibleAnywhere, Category = "Player Stats")
    float Health = 100.f;

    void UpdateHUDHealth();

    //////////
    // Base variables
    //
    AWTRPlayerController* WTRPlayerController;

    //////////
    // Other variables
    //
    UPROPERTY(EditDefaultsOnly, Category = "Camera")
    float DistanceForHidingCamera = 200.f;

    //////////
    // Multiplayer functions
    //
    UFUNCTION()
    void OnRep_Health();

    UFUNCTION()
    void OnRep_Username();

    UFUNCTION()
    void OnRep_OverlappingWeapon(AWTRWeapon* LastWeapon);

    UFUNCTION(Server, Reliable)
    void Server_OnEquippedButtonPressed();

    UFUNCTION(Server, Reliable)
    void Server_SetUsername();

    //////////
    // Functions
    //
    void SetTurningInPlace(float DeltaTime);
    void UpdateIfIsNotStanding();
    void HideCharacterWithWeaponIfCameraClose();
    float CalculateSpeed() const;

    //////////
    // Callbacks
    //
    UFUNCTION()
    void OnTakeAnyDamageCallback(
        AActor* DamagedActor, float Damage, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser);
};
