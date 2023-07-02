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
class UWTRBuffComponent;
class UAnimMontage;
class UWidgetComponent;
class UTextRenderComponent;
class AWTRWeapon;
class AWTRPlayerController;
class AWTRPlayerState;
class USoundCue;
class AWTR_HUD;
class AWTRGameMode;

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
    virtual void Destroyed() override;
    virtual void OnRep_ReplicateMovement() override;
    virtual void Jump() override;

    // Try to declare variables in Tick that can`t be declare in BeginPlay
    void PullInit();

    void UpdateHUDHealth();
    void UpdateHUDShield();
    void Elim();
    void PlayFireMontage(bool bAiming);
    void PlayReloadMontage();
    void StopReloadMontage();
    void PlayHitReactMontage();
    void PlayEliminationMontage();
    void PlayThrowGrenadeMontage();
    void StopThrowGrenadeMontage();

    bool IsWeaponEquipped() const;
    bool IsAiming() const;
    FORCEINLINE bool IsElimmed() const { return bElimmed; }
    FORCEINLINE bool ShouldRotateRootBone() const { return bRotateRootBone; }
    FORCEINLINE bool IsDisableGameplay() const { return bDisableGameplay; }
    FORCEINLINE bool IsFullHealth() const { return Health >= MaxHealth; }
    FORCEINLINE bool IsFullShield() const { return Shield >= MaxShield; }

    FORCEINLINE void SetHealth(float NewHealth) { Health = NewHealth; }
    FORCEINLINE void SetShield(float NewShield) { Shield = NewShield; }
    FORCEINLINE void SetDisableGameplay(bool bDisable) { bDisableGameplay = bDisable; }
    void SetOverlappingWeapon(AWTRWeapon* Weapon);
    void OnPossessHandle(AWTRPlayerController* NewController, AWTR_HUD* NewHUD);

    FORCEINLINE int32 GetCarriedAmmo() const;
    FORCEINLINE float GetHealth() const { return Health; }
    FORCEINLINE float GetMaxHealth() const { return MaxHealth; }
    FORCEINLINE float GetShield() const { return Shield; }
    FORCEINLINE float GetMaxShield() const { return MaxShield; }
    FORCEINLINE float GetAO_Yaw() const { return AO_Yaw; }
    FORCEINLINE float GetAO_Pitch() const { return AO_Pitch; }
    FORCEINLINE ETurningInPlace GetTurningState() const { return TurningInPlace; }
    FORCEINLINE USpringArmComponent* GetSpringArm() const { return SpringArmComponent; }
    FORCEINLINE UCameraComponent* GetCameraComponent() const { return CameraComponent; }
    FORCEINLINE UAnimMontage* GetReloadMontage() const { return ReloadMontage; }
    FORCEINLINE UStaticMeshComponent* GetGrenadeMesh() const { return GrenadeMesh; }
    FORCEINLINE UWTRBuffComponent* GetBuffComponent() const { return Buff; }
    ECombatState GetCombatState() const;
    AWTRWeapon* GetEquippedWeapon() const;
    FVector GetHitTarget() const;

    UFUNCTION(BlueprintCallable)
    FORCEINLINE UWTRCombatComponent* GetCombatComponent() const { return Combat; }

    UFUNCTION(NetMulticast, Reliable)
    void Multicast_Elim();

    UFUNCTION(BlueprintImplementableEvent)
    void SetShowScopeAnimation(bool bShowScope);

protected:
    /*
     * Input functions and callbacks
     */
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
    void OnAudioUpButtonPressed();
    void OnAudioDownButtonPressed();
    void OnGrenadeButtonPressed();
    void OnWeaponSwapButtonPressed();

    /*
     * Other functions
     */
    virtual void BeginPlay() override;

private:
    /*
     * Components
     */

    UPROPERTY(VisibleAnywhere, Category = "WTR | Camera")
    USpringArmComponent* SpringArmComponent;

    UPROPERTY(VisibleAnywhere, Category = "WTR | Camera")
    UCameraComponent* CameraComponent;

    UPROPERTY(VisibleAnywhere, Category = "WTR | Combat")
    UWTRCombatComponent* Combat;

    UPROPERTY(VisibleAnywhere, Category = "WTR | Buff")
    UWTRBuffComponent* Buff;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = "WTR | OverheadWidget")
    UWidgetComponent* OverheadWidget;

    UPROPERTY(VisibleAnywhere, Category = "WTR | OverheadText")
    UTextRenderComponent* OverheadText;

    UPROPERTY(VisibleAnywhere, Category = "WTR | Elimination")
    UTimelineComponent* DissolveTimelineComponent;

    /*
     * Multiplayer variables
     */
    UPROPERTY(ReplicatedUsing = OnRep_OverlappingWeapon)
    AWTRWeapon* OverlappingWeapon;

    UPROPERTY(ReplicatedUsing = OnRep_Username)
    FString Username;

    UPROPERTY(Replicated)
    bool bDisableGameplay = false;

    /*
     * Animation
     */
    UPROPERTY(EditDefaultsOnly, Category = "WTR | Animation")
    UAnimMontage* FireWeaponMontage;

    UPROPERTY(EditDefaultsOnly, Category = "WTR | Animation")
    UAnimMontage* ReloadMontage;

    UPROPERTY(EditDefaultsOnly, Category = "WTR | Animation")
    UAnimMontage* EliminationMontage;

    UPROPERTY(EditDefaultsOnly, Category = "WTR | Animation")
    UAnimMontage* HitReactMontage;

    UPROPERTY(EditDefaultsOnly, Category = "WTR | Animation")
    UAnimMontage* ThrowGrenadeMontage;

    UPROPERTY(Replicated)
    bool bDamageFromFlamethrower = false;

    /*
     * Movement
     */
    UPROPERTY(EditDefaultsOnly, Category = "WTR | Movement")
    float AngleToTurn = 80.f;

    UPROPERTY(EditDefaultsOnly, Category = "WTR | Movement", meta = (ClampMin = 0.0))
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

    /*
     * Health
     */
    UPROPERTY(EditDefaultsOnly, Category = "WTR | Player Stats")
    float MaxHealth = 100.f;

    UPROPERTY(ReplicatedUsing = OnRep_Health, EditDefaultsOnly, Category = "WTR | Player Stats")
    float Health = 100.f;

    /*
     * Shield
     */
    UPROPERTY(EditDefaultsOnly, Category = "WTR | Player Stats")
    float MaxShield = 100.f;

    UPROPERTY(ReplicatedUsing = OnRep_Shield, EditDefaultsOnly, Category = "WTR | Player Stats")
    float Shield = 100.f;

    /*
     * Eliminated
     */
    UPROPERTY(EditDefaultsOnly, Category = "WTR | Respawn")
    float EliminatedTimerDelay = 3.f;

    bool bElimmed = false;
    FTimerHandle EliminatedTimerHandle;

    void OnEliminatedTimerFinished();

    /*
     * Dissolve
     */
    UPROPERTY(EditDefaultsOnly, Category = "WTR | Dissolve | Base")
    UCurveFloat* DissolveCurve;

    UPROPERTY(EditDefaultsOnly, Category = "WTR | Dissolve | Base")
    UMaterialInstance* DissolveMaterialInst;

    UPROPERTY(EditDefaultsOnly, Category = "WTR | Dissolve | Base")
    float DissolveMaterialGlow = 200.f;

    UPROPERTY(VisibleAnywhere, Category = "WTR | Dissolve | Base")
    UMaterialInstanceDynamic* DissolveMaterialInstDynamic;

    FOnTimelineFloat OnDissolveTimelineFloat;

    UFUNCTION()
    void OnDissolveTrackFloatChange(float DissolveValue);

    void StartDissolve();

    /*
     * ElimBot
     */
    UPROPERTY(EditDefaultsOnly, Category = "WTR | Dissolve | ElimBot")
    UParticleSystem* ElimBotParticleSys;

    UPROPERTY(EditDefaultsOnly, Category = "WTR | Dissolve | ElimBot")
    USoundCue* ElimBotSound;

    UPROPERTY(EditDefaultsOnly, Category = "WTR | Dissolve | ElimBot")
    float ElimBotHeightAbovePlayer = 200.f;

    UPROPERTY()
    UParticleSystemComponent* ElimBotParticleSysComponent;

    /*
     * Grenade
     */
    UPROPERTY(EditDefaultsOnly, Category = "WTR | Grenade")
    UStaticMeshComponent* GrenadeMesh;

    /*
     * Base variables
     */
    AWTRPlayerController* WTRPlayerController;
    AWTRPlayerState* WTRPlayerState;

    /*
     * Other variables
     */
    UPROPERTY(EditDefaultsOnly, Category = "WTR | Camera")
    float DistanceForHidingCamera = 200.f;

    UPROPERTY(EditDefaultsOnly, Category = "WTR | Spam")
    float DelaySwapButton = 0.4f;

    bool bCanSwap = true;

    FTimerHandle SwapButtonTimerHandle;

    void SwapButtonTimerFinished();

    /*
     * Multiplayer functions
     */
    UFUNCTION()
    void OnRep_Health(float LastHealth);

    UFUNCTION()
    void OnRep_Shield(float LastShield);

    UFUNCTION()
    void OnRep_Username();

    UFUNCTION()
    void OnRep_OverlappingWeapon(AWTRWeapon* LastWeapon);

    UFUNCTION(Server, Reliable)
    void Server_OnEquippedButtonPressed();

    UFUNCTION(Server, Reliable)
    void Server_OnWeaponSwapButtonPressed();

    UFUNCTION(NetMulticast, Reliable)
    void Multicast_OnDestroyed();

    /*
     * Functions
     */
    void RotateInPlace(float DeltaTime);
    void SetTurningInPlace(float DeltaTime);
    void UpdateIfIsNotStanding();
    void HideCharacterWithWeaponIfCameraClose();
    void DropOrDestroyWeapons();
    float CalculateSpeed() const;
    AWTRGameMode* GetWTRGameMode() const;

    /*
     * Callbacks
     */
    UFUNCTION()
    void OnTakeAnyDamageCallback(
        AActor* DamagedActor, float Damage, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser);

    void OnReloadFinishedNotifyPlayed(USkeletalMeshComponent* MeshComp);
};
