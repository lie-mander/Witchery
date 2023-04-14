// Witchery. Copyright Liemander. All Rights Reserved.

#include "Components/WTRCombatComponent.h"
#include "Net/UnrealNetwork.h"
#include "Components/SphereComponent.h"
#include "Weapons/WTRWeapon.h"
#include "Character/WTRCharacter.h"
#include "Engine/SkeletalMeshSocket.h"

UWTRCombatComponent::UWTRCombatComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UWTRCombatComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const 
{
    DOREPLIFETIME(UWTRCombatComponent, EquippedWeapon);
    DOREPLIFETIME(UWTRCombatComponent, bIsAiming);
}

void UWTRCombatComponent::BeginPlay()
{
    Super::BeginPlay();
}

void UWTRCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UWTRCombatComponent::EquipWeapon(AWTRWeapon* WeaponToEquip)
{
    if (!Character || !WeaponToEquip) return;

    EquippedWeapon = WeaponToEquip;
    EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);
    EquippedWeapon->SetOwner(Character);

    const USkeletalMeshSocket* HandSocket = Character->GetMesh()->GetSocketByName(FName("RightHandSocket"));
    if (HandSocket)
    {
        HandSocket->AttachActor(EquippedWeapon, Character->GetMesh());
    }
}

void UWTRCombatComponent::SetAiming(bool bAiming) 
{
    bIsAiming = bAiming;
    ServerSetAiming(bAiming);
}

void UWTRCombatComponent::ServerSetAiming_Implementation(bool bAiming) 
{
    bIsAiming = bAiming;
}
