// Witchery. Copyright Liemander. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Weapons/WTRWeapon.h"
#include "WTRFlamethrower.generated.h"

class USphereComponent;
class UNiagaraSystem;
class UNiagaraComponent;
class USoundCue;
class UAudioComponent;
class AWTRCharacter;

UCLASS()
class WITCHERY_API AWTRFlamethrower : public AWTRWeapon
{
    GENERATED_BODY()

public:
    AWTRFlamethrower();

protected:
    virtual void Fire(const FVector& HitTarget) override;
    virtual void StopFire() override;

private:
    UPROPERTY(VisibleAnywhere, Category = "WTR | Components")
    UStaticMeshComponent* DamageArea;

    UPROPERTY(EditDefaultsOnly, Category = "WTR | Shoot")
    USoundCue* FlameLoopSound;

    UPROPERTY(EditDefaultsOnly, Category = "WTR | Shoot")
    USoundCue* FlameEndSound;

    UPROPERTY(EditDefaultsOnly, Category = "WTR | Hit")
    float Damage = 20.f;

    UPROPERTY(EditDefaultsOnly, Category = "WTR | Fire FX")
    FVector FireSystemScale;

    UPROPERTY(EditDefaultsOnly, Category = "WTR | Fire FX")
    UNiagaraSystem* FireSystem;

    UPROPERTY()
    UNiagaraComponent* FireFXComponent;

    UPROPERTY()
    UAudioComponent* FlameLoopComponent;

    bool bCanDamage = false;

    FTimerHandle DamageTimerHandle;
    TArray<AWTRCharacter*> CharactersToDamage;
    TArray<AWTRCharacter*> ElimmedCharacters;

    void DamageTimerUpdated();

    UFUNCTION()
    void OnDamageAreaBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
        int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnDamageAreaEndOverlap(
        UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
};
