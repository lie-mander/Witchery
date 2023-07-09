// Witchery. Copyright Liemander. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "WTRLagCompensationComponent.generated.h"

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class WITCHERY_API UWTRLagCompensationComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UWTRLagCompensationComponent();
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

protected:
    virtual void BeginPlay() override;

private:
};
