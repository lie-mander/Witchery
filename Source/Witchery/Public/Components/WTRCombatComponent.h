// Witchery. Copyright Liemander. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "WTRCombatComponent.generated.h"

class AWTRCharacter;
class AWTRWeapon;
class AWTRPlayerController;
class AWTR_HUD;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class WITCHERY_API UWTRCombatComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    friend class AWTRCharacter;

    UWTRCombatComponent();
    void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    void EquipWeapon(AWTRWeapon* WeaponToEquip);

protected:
    virtual void BeginPlay() override;

    void SetAiming(bool bAiming);
    void OnFireButtonPressed(bool bPressed);

    UFUNCTION()
    void OnRep_EquippedWeapon();

    UFUNCTION(Server, Reliable)
    void ServerSetAiming(bool bAiming);
    
    UFUNCTION(Server, Reliable)
    void ServerFire(const FVector_NetQuantize& TraceHitTarget);

    UFUNCTION(NetMulticast, Reliable)
    void MulticastFire(const FVector_NetQuantize& TraceHitTarget);

private:
    AWTRCharacter* Character;
    AWTRPlayerController* Controller;
    AWTR_HUD* HUD;

    UPROPERTY(ReplicatedUsing = OnRep_EquippedWeapon)
    AWTRWeapon* EquippedWeapon;

    UPROPERTY(Replicated)
    bool bIsAiming = false;

    UPROPERTY(EditDefaultsOnly, Category = "Movement")
    float BaseWalkSpeed = 600.f;

    UPROPERTY(EditDefaultsOnly, Category = "Movement")
    float AimWalkSpeed = 300.f;

    UPROPERTY(EditDefaultsOnly, Category = "Shoot")
    float TraceRange = 300.f;

    UPROPERTY(EditDefaultsOnly, Category = "Shoot")
    FVector3d SpringArmOffsetWhileEquipped = FVector3d(-160.f, 0.f, 180.f);

    float CrosshairVelocityFactor = 0.f;
    float CrosshairAirFactor = 0.f;

    bool bFireButtonPressed = false;

    FVector HitTarget;

    void TraceFromScreen(FHitResult& TraceHitResult);
    void DrawCrosshair(float DeltaTime);
};
