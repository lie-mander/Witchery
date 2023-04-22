// Witchery. Copyright Liemander. All Rights Reserved.

#include "Components/WTRCombatComponent.h"
#include "Net/UnrealNetwork.h"
#include "Components/SphereComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Weapons/WTRWeapon.h"
#include "Character/WTRCharacter.h"
#include "Engine/SkeletalMeshSocket.h"

UWTRCombatComponent::UWTRCombatComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UWTRCombatComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

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

    Character->GetCharacterMovement()->bOrientRotationToMovement = false;
    Character->bUseControllerRotationYaw = true;
    Character->GetSpringArm()->SetRelativeTransform(FTransform(FQuat4d(FRotator::ZeroRotator), FVector3d(-160.f, 0.f, 180.f)));

    const USkeletalMeshSocket* HandSocket = Character->GetMesh()->GetSocketByName(FName("RightHandSocket"));
    if (HandSocket)
    {
        HandSocket->AttachActor(EquippedWeapon, Character->GetMesh());
    }
}

void UWTRCombatComponent::OnRep_EquippedWeapon()
{
    if (Character && EquippedWeapon)
    {
        Character->GetCharacterMovement()->bOrientRotationToMovement = false;
        Character->bUseControllerRotationYaw = true;
    }
}

void UWTRCombatComponent::SetAiming(bool bAiming)
{
    bIsAiming = bAiming;

    if (Character)
    {
        Character->GetCharacterMovement()->MaxWalkSpeed = bIsAiming ? AimWalkSpeed : BaseWalkSpeed;
    }

    ServerSetAiming(bAiming);
}

void UWTRCombatComponent::ServerSetAiming_Implementation(bool bAiming)
{
    bIsAiming = bAiming;

    if (Character)
    {
        Character->GetCharacterMovement()->MaxWalkSpeed = bIsAiming ? AimWalkSpeed : BaseWalkSpeed;
    }
}
