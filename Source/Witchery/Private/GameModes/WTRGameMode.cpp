// Witchery. Copyright Liemander. All Rights Reserved.

#include "GameModes/WTRGameMode.h"
#include "Character/WTRCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerStart.h"

void AWTRGameMode::PlayerEliminated(
    AWTRCharacter* EliminatedCharacter, AWTRPlayerController* VictimController, AWTRPlayerController* AttackerController)
{
    if (EliminatedCharacter)
    {
        EliminatedCharacter->Elim();
    }
}

void AWTRGameMode::RequestRespawn(ACharacter* EliminatedCharacter, AController* EliminatedController) 
{
    if (EliminatedCharacter)
    {
        EliminatedCharacter->Reset();
        EliminatedCharacter->Destroy();
    }

    if (EliminatedController)
    {
        TArray<AActor*> PlayerStarts;
        UGameplayStatics::GetAllActorsOfClass(this, APlayerStart::StaticClass(), PlayerStarts);

        const int32 PlayerStartIndex = FMath::RandRange(0, PlayerStarts.Num() - 1);

        RestartPlayerAtPlayerStart(EliminatedController, PlayerStarts[PlayerStartIndex]);
    }
}
