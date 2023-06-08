// Witchery. Copyright Liemander. All Rights Reserved.

#include "Animation/Notifies/WTRAnimNotify.h"

void UWTRAnimNotify::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
    Super::Notify(MeshComp, Animation, EventReference);

    OnNotifyPlayed.Broadcast(MeshComp);
}
