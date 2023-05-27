// Witchery. Copyright Liemander. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "WTRGameMode.generated.h"

class AWTRCharacter;
class AWTRPlayerController;

UCLASS()
class WITCHERY_API AWTRGameMode : public AGameMode
{
    GENERATED_BODY()

public:
    virtual void PlayerEliminated(
        AWTRCharacter* EliminatedCharacter, AWTRPlayerController* VictimController, AWTRPlayerController* AttackerController);
    virtual void RequestRespawn(ACharacter* EliminatedCharacter, AController* EliminatedController);
};
