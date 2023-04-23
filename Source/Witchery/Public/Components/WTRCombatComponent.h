// Witchery. Copyright Liemander. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "WTRCombatComponent.generated.h"

class AWTRCharacter;
class AWTRWeapon;

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

    UFUNCTION()
    void OnRep_EquippedWeapon();

    UFUNCTION(Server, Reliable)
    void ServerSetAiming(bool bAiming);

    void OnFireButtonPressed(bool bPressed);

private:
    AWTRCharacter* Character;

    UPROPERTY(ReplicatedUsing = OnRep_EquippedWeapon)
    AWTRWeapon* EquippedWeapon;

    UPROPERTY(Replicated)
    bool bIsAiming = false;

    UPROPERTY(EditAnywhere)
    float BaseWalkSpeed = 600.f;

    UPROPERTY(EditAnywhere)
    float AimWalkSpeed = 300.f;

    bool bFireButtonPressed = false;
};
