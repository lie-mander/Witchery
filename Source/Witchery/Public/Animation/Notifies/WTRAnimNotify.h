// Witchery. Copyright Liemander. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "WTRTypes.h"
#include "WTRAnimNotify.generated.h"

UCLASS()
class WITCHERY_API UWTRAnimNotify : public UAnimNotify
{
    GENERATED_BODY()

public:
    virtual void Notify(
        USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;

    FOnNotifyPlayed OnNotifyPlayed;
};
