#pragma once

#include "Character/WTRCharacter.h"
#include "Character/WTRPlayerController.h"

class UWTRTools
{
public:
    template <class CharacterType = AWTRCharacter, class PlayerControllerType = AWTRPlayerController>
    static PlayerControllerType* GetPlayerControllerByActor(class AActor* Actor)
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
};
