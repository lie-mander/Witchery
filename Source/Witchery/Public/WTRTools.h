#pragma once

#include "Character/WTRCharacter.h"
#include "Character/WTRPlayerController.h"

class UAnimMontage;
class AActor;

class UWTRTools
{
public:
    template <class CharacterType = AWTRCharacter, class PlayerControllerType = AWTRPlayerController>
    static PlayerControllerType* GetPlayerControllerByActor(AActor* Actor)
    {
        if (Actor)
        {
            CharacterType* Character = Cast<CharacterType>(Actor);
            if (Character)
            {
                PlayerControllerType* Controller = Cast<PlayerControllerType>(Character->Controller);
                if (Controller)
                {
                    return Controller;
                }
            }
        }
        return nullptr;
    }

    template <class T>
    static T* FindNotifyByClass(UAnimMontage* AnimMontage)
    {
        if (!AnimMontage) return nullptr;

        const auto NotifyEvents = AnimMontage->Notifies;
        for (auto NotifyEvent : NotifyEvents)
        {
            auto ResultNotify = Cast<T>(NotifyEvent.Notify);
            if (ResultNotify)
            {
                return ResultNotify;
            }
        }
        return nullptr;
    }
};
