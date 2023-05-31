// Witchery. Copyright Liemander. All Rights Reserved.

#include "Animation/Notifies/WTRAnimNotify.h"

void UWTRAnimNotify::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation) 
{
    Super::Notify(MeshComp, Animation);

    OnNotifyPlayed.Broadcast(MeshComp);
}
