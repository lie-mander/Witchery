// Witchery. Copyright Liemander. All Rights Reserved.

#include "Components/WTRCombatComponent.h"
#include "Net/UnrealNetwork.h"
#include "Components/SphereComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Weapons/WTRWeapon.h"
#include "Character/WTRCharacter.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"

UWTRCombatComponent::UWTRCombatComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
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

    FHitResult HitResult;
    TraceFromScreen(HitResult);
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
        Character->GetSpringArm()->SetRelativeTransform(FTransform(FQuat4d(FRotator::ZeroRotator), FVector3d(-160.f, 0.f, 180.f)));
    }
}

void UWTRCombatComponent::OnFireButtonPressed(bool bPressed)
{
    bFireButtonPressed = bPressed;
    if (bFireButtonPressed)
    {
        ServerFire();
    }
}

void UWTRCombatComponent::ServerFire_Implementation()
{
    MulticastFire();
}

void UWTRCombatComponent::MulticastFire_Implementation()
{
    if (Character && EquippedWeapon)
    {
        Character->PlayFireMontage(bIsAiming);
        EquippedWeapon->Fire();
    }
}

void UWTRCombatComponent::TraceFromScreen(FHitResult TraceHitResult)
{
    FVector2D ViewportSize;
    if (GEngine && GEngine->GameViewport)
    {
        GEngine->GameViewport->GetViewportSize(ViewportSize);
    }

    FVector2D CrosshairLocation = FVector2D(ViewportSize.X / 2.f, ViewportSize.Y / 2.f);
    FVector CrosshairWorldPosition;
    FVector CrosshairWorldDirection;

    bool bDeprojectScreen = UGameplayStatics::DeprojectScreenToWorld(  //
        UGameplayStatics::GetPlayerController(this, 0),                //
        CrosshairLocation,                                             //
        CrosshairWorldPosition,                                        //
        CrosshairWorldDirection                                        //
    );

    if (bDeprojectScreen && GetWorld())
    {
        const FVector Start = CrosshairWorldPosition;
        const FVector End = CrosshairWorldPosition + CrosshairWorldDirection * TraceRange;
        GetWorld()->LineTraceSingleByChannel(  //
            TraceHitResult,                    //
            Start,                             //
            End,                               //
            ECC_Visibility                     //
        );

        if (!TraceHitResult.bBlockingHit)
        {
            TraceHitResult.ImpactPoint = End;
        }
        else
        {
            UKismetSystemLibrary::DrawDebugSphere(  //
                this,                               //
                TraceHitResult.ImpactPoint,         //
                12.f,                               //
                12,                                 //
                FLinearColor::Red                   //
            );
        }
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
