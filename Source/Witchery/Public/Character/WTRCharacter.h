// Witchery. Copyright Liemander. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "WTRTypes.h"
#include "Interfaces/InteractWithCrosshairInterface.h"
#include "Components/TimelineComponent.h"
#include "WTRCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UWTRCombatComponent;
class UAnimMontage;
class UWidgetComponent;
class UTextRenderComponent;
class AWTRWeapon;
class AWTRPlayerController;
class AWTRPlayerState;
class USoundCue;
class AWTR_HUD;

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

    // Try to declare variables in Tick that can`t be declare in BeginPlay
    void PullInit();

    virtual void OnRep_ReplicateMovement() override;

    UFUNCTION(NetMulticast, Reliable)
    void Multicast_Elim();

    void Elim();
    virtual void Jump() override;
    void PlayFireMontage(bool bAiming);
    void PlayReloadMontage();
    void PlayHitReactMontage();
    void PlayEliminationMontage();

    bool IsWeaponEquipped() const;
    bool IsAiming() const;
    FORCEINLINE bool IsElimmed() const { return bElimmed; }
    FORCEINLINE bool ShouldRotateRootBone() const { return bRotateRootBone; }

    void SetOverlappingWeapon(AWTRWeapon* Weapon);
    void OnPossessHandle(AWTRPlayerController* NewController, AWTR_HUD* NewHUD);

    FORCEINLINE int32 GetCarriedAmmo() const;
    FORCEINLINE float GetHealth() const { return Health; }
    FORCEINLINE float GetMaxHealth() const { return MaxHealth; }
    FORCEINLINE float GetAO_Yaw() const { return AO_Yaw; }
    FORCEINLINE float GetAO_Pitch() const { return AO_Pitch; }
    FORCEINLINE ETurningInPlace GetTurningState() const { return TurningInPlace; }
    FORCEINLINE USpringArmComponent* GetSpringArm() const { return SpringArmComponent; }
    FORCEINLINE UCameraComponent* GetCameraComponent() const { return CameraComponent; }
    ECombatState GetCombatState() const;
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
    void OnReloadButtonPressed();

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

    UPROPERTY(VisibleAnywhere, Category = "Elimination")
    UTimelineComponent* DissolveTimelineComponent;

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
    UPROPERTY(EditDefaultsOnly, Category = "Animation")
    UAnimMontage* FireWeaponMontage;

    UPROPERTY(EditDefaultsOnly, Category = "Animation")
    UAnimMontage* ReloadMontage;

    UPROPERTY(EditDefaultsOnly, Category = "Animation")
    UAnimMontage* EliminationMontage;

    UPROPERTY(EditDefaultsOnly, Category = "Animation")
    UAnimMontage* HitReactMontage;

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
    // Eliminated
    //
    UPROPERTY(EditDefaultsOnly, Category = "Respawn")
    float EliminatedTimerDelay = 3.f;

    bool bElimmed = false;
    FTimerHandle EliminatedTimerHandle;

    void OnEliminatedTimerFinished();

    //////////
    // Dissolve
    //
    UPROPERTY(EditDefaultsOnly, Category = "Dissolve | Base")
    UCurveFloat* DissolveCurve;

    UPROPERTY(EditDefaultsOnly, Category = "Dissolve | Base")
    UMaterialInstance* DissolveMaterialInst;

    UPROPERTY(EditDefaultsOnly, Category = "Dissolve | Base")
    float DissolveMaterialGlow = 200.f;

    UPROPERTY(VisibleAnywhere, Category = "Dissolve | Base")
    UMaterialInstanceDynamic* DissolveMaterialInstDynamic;

    FOnTimelineFloat OnDissolveTimelineFloat;

    UFUNCTION()
    void OnDissolveTrackFloatChange(float DissolveValue);

    void StartDissolve();

    //////////
    // ElimBot
    //
    UPROPERTY(EditDefaultsOnly, Category = "Dissolve | ElimBot")
    UParticleSystem* ElimBotParticleSys;

    UPROPERTY(EditDefaultsOnly, Category = "Dissolve | ElimBot")
    USoundCue* ElimBotSound;

    UPROPERTY(EditDefaultsOnly, Category = "Dissolve | ElimBot")
    float ElimBotHeightAbovePlayer = 200.f;

    UPROPERTY()
    UParticleSystemComponent* ElimBotParticleSysComponent;

    //////////
    // Base variables
    //
    AWTRPlayerController* WTRPlayerController;
    AWTRPlayerState* WTRPlayerState;

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

    UFUNCTION(NetMulticast, Reliable)
    void Multicast_OnDestroyed();

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

    void OnReloadFinishedNotifyPlayed(USkeletalMeshComponent* MeshComp);
};
