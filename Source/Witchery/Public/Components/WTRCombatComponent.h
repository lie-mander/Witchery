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
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    void EquipWeapon(AWTRWeapon* WeaponToEquip);

protected:
    virtual void BeginPlay() override;

private:
    AWTRCharacter* Character;
    AWTRWeapon* EquippedWeapon;
};
