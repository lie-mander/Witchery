// Witchery. Copyright Liemander. All Rights Reserved.

#include "Weapons/WTRFlamethrower.h"
#include "Components/SphereComponent.h"
#include "Components/AudioComponent.h"
#include "Components/StaticMeshComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "Character/WTRCharacter.h"

AWTRFlamethrower::AWTRFlamethrower()
{
    DamageArea = CreateDefaultSubobject<UStaticMeshComponent>("DamageArea");
    DamageArea->SetupAttachment(GetRootComponent());
    DamageArea->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    DamageArea->SetCollisionResponseToAllChannels(ECR_Ignore);
    DamageArea->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
    DamageArea->SetVisibility(false);

    DamageArea->OnComponentBeginOverlap.AddDynamic(this, &AWTRFlamethrower::OnDamageAreaBeginOverlap);
    DamageArea->OnComponentEndOverlap.AddDynamic(this, &AWTRFlamethrower::OnDamageAreaEndOverlap);
}

void AWTRFlamethrower::Fire(const FVector& HitTarget)
{
    Super::Fire(HitTarget);

    if (GetWeaponMesh() && !IsEmpty())
    {
        bCanDamage = true;

        if (FlameLoopSound && !FlameLoopComponent && ShootSound)
        {
            UGameplayStatics::PlaySoundAtLocation(  //
                this,                               //
                ShootSound,                         //
                GetActorLocation()                  //

            );

            FlameLoopComponent = UGameplayStatics::SpawnSoundAttached(  //
                FlameLoopSound,                                         //
                GetRootComponent()                                      //
            );
        }

        const USkeletalMeshSocket* MuzzleSocket = GetWeaponMesh()->GetSocketByName(FName(MuzzleSocketName));
        if (MuzzleSocket && FireSystem && !FireFXComponent)
        {
            FireFXComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(                 //
                FireSystem,                                                                 //
                GetRootComponent(),                                                         //
                MuzzleSocketName,                                                           //
                MuzzleSocket->GetSocketLocation(GetWeaponMesh()),                           //
                MuzzleSocket->GetSocketTransform(GetWeaponMesh()).GetRotation().Rotator(),  //
                FireSystemScale,                                                            //
                EAttachLocation::KeepWorldPosition,                                         //
                false,                                                                      //
                ENCPoolMethod::None                                                         //
            );
        }

        if (HasAuthority() && !GetWorldTimerManager().IsTimerActive(DamageTimerHandle))
        {
            GetWorldTimerManager().SetTimer(            //
                DamageTimerHandle,                      //
                this,                                   //
                &AWTRFlamethrower::DamageTimerUpdated,  //
                FireDelay,                              //
                true                                    //
            );
        }
    }
    else
    {
        StopFire();
    }
}

void AWTRFlamethrower::StopFire()
{
    Super::StopFire();

    bCanDamage = false;
    if (HasAuthority())
    {
        GetWorldTimerManager().ClearTimer(DamageTimerHandle);
    }

    if (FireFXComponent)
    {
        FireFXComponent->Deactivate();
        FireFXComponent = nullptr;
    }

    if (FlameLoopComponent)
    {
        FlameLoopComponent->Deactivate();
        FlameLoopComponent = nullptr;
    }

    if (FlameEndSound)
    {
        UGameplayStatics::PlaySoundAtLocation(  //
            this,                               //
            FlameEndSound,                      //
            GetActorLocation()                  //
        );
    }
}

void AWTRFlamethrower::OnDamageAreaBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (HasAuthority())
    {
        AWTRCharacter* ToDamageCharacter = Cast<AWTRCharacter>(OtherActor);
        if (ToDamageCharacter)
        {
            CharactersToDamage.AddUnique(ToDamageCharacter);
        }
    }
}

void AWTRFlamethrower::OnDamageAreaEndOverlap(
    UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    if (HasAuthority())
    {
        AWTRCharacter* OutDamageCharacter = Cast<AWTRCharacter>(OtherActor);
        if (OutDamageCharacter && !OutDamageCharacter->IsElimmed())
        {
            const bool bCharacterInArray = CharactersToDamage.Contains(OutDamageCharacter);
            if (bCharacterInArray)
            {
                CharactersToDamage.Remove(OutDamageCharacter);
            }
        }
    }
}

void AWTRFlamethrower::DamageTimerUpdated()
{
    if (HasAuthority() && !CharactersToDamage.IsEmpty())
    {
        AController* OwnerController = GetOwnerPlayerController();
        if (OwnerController)
        {
            for (auto CharacterToDamage : CharactersToDamage)
            {
                UGameplayStatics::ApplyDamage(  //
                    CharacterToDamage,          //
                    Damage,                     //
                    OwnerController,            //
                    this,                       //
                    UDamageType::StaticClass()  //
                );

                if (CharacterToDamage->IsElimmed())
                {
                    ElimmedCharacters.AddUnique(CharacterToDamage);
                }
            }

            for (auto ElimmedCharacter : ElimmedCharacters)
            {
                CharactersToDamage.Remove(ElimmedCharacter);
            }
        }
    }
}
