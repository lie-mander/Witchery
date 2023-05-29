// Witchery. Copyright Liemander. All Rights Reserved.

#include "GameModes/WTRGameMode.h"
#include "Character/WTRCharacter.h"
#include "Character/WTRPlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerStart.h"
#include "WTRPlayerState.h"

void AWTRGameMode::PlayerEliminated(
    AWTRCharacter* EliminatedCharacter, AWTRPlayerController* VictimController, AWTRPlayerController* AttackerController)
{
    AWTRPlayerState* AttackerPlayerState = AttackerController ? Cast<AWTRPlayerState>(AttackerController->PlayerState) : nullptr;
    AWTRPlayerState* VictimPlayerState = VictimController ? Cast<AWTRPlayerState>(VictimController->PlayerState) : nullptr;

    if (AttackerPlayerState && AttackerPlayerState != VictimPlayerState)
    {
        AttackerPlayerState->AddToScore(1.f);
    }

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
