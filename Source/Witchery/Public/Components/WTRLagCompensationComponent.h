// Witchery. Copyright Liemander. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "WTRTypes.h"
#include "WTRLagCompensationComponent.generated.h"

class AWTRCharacter;
class AWTRPlayerController;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class WITCHERY_API UWTRLagCompensationComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    friend class AWTRCharacter;

    UWTRLagCompensationComponent();
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    void SaveFramePackage(FFramePackage& Package);
    void ShowFramePackage(const FFramePackage& Package, const FColor& Color);

protected:
    virtual void BeginPlay() override;

private:
    UPROPERTY()
    AWTRCharacter* Character;

    UPROPERTY()
    AWTRPlayerController* Controller;
    
    UPROPERTY(EditDefaultsOnly, Category = "WTR | RewindTime")
    float MaxRecordTime = 4.f;

    TDoubleLinkedList<FFramePackage> FrameHistory;

    void RecordFrameHistory();
    void SaveThisFrame();
    float TimeBetweenHeadAndTail();
};
