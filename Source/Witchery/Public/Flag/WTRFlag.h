// Witchery. Copyright Liemander. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WTRTypes.h"
#include "WTRFlag.generated.h"

class USkeletalMeshComponent;
class USphereComponent;
class AWTRCharacter;
class USceneComponent;
class UMaterialInstance;

UCLASS()
class WITCHERY_API AWTRFlag : public AActor
{
	GENERATED_BODY()
	
public:	
	AWTRFlag();
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
    void Dropped();

    FORCEINLINE void SetFlagState(EFlagState State) { FlagState = State; }

protected:
	virtual void BeginPlay() override;

private:	
	UPROPERTY(EditDefaultsOnly, Category = "WTR | Components")
    USkeletalMeshComponent* FlagMesh;

	UPROPERTY(VisibleAnywhere, Category = "WTR | Components")
    USceneComponent* SceneComponent;

	UPROPERTY(VisibleAnywhere, Category = "WTR | Components")
    USphereComponent* AreaSphere;

    UPROPERTY(EditDefaultsOnly, Category = "WTR | Flag")
    UMaterialInstance* RedTeamMaterial;

    UPROPERTY(EditDefaultsOnly, Category = "WTR | Flag")
    UMaterialInstance* BlueTeamMaterial;

    UPROPERTY(EditAnywhere, Category = "WTR | Flag")
    ETeam FlagTeam = ETeam::ET_BlueTeam;

    UPROPERTY(ReplicatedUsing = OnRep_FlagState)
    EFlagState FlagState = EFlagState::EFS_Dropped;

    UFUNCTION()
    void OnRep_FlagState();

    UPROPERTY()
    AWTRCharacter* OwnerCharacter;

	/*
     * Callbacks
     */
    UFUNCTION()
    void OnSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
        int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
};
