// Witchery. Copyright Liemander. All Rights Reserved.

#include "GameModes/WTRGameMode.h"
#include "Character/WTRCharacter.h"
#include "Character/WTRPlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerStart.h"
#include "WTRPlayerState.h"

namespace MatchState
{
const FName Cooldown = FName(TEXT("Cooldown"));
}

AWTRGameMode::AWTRGameMode()
{
    bDelayedStart = true;
}

void AWTRGameMode::BeginPlay()
{
    Super::BeginPlay();

    if (GetWorld())
    {
        TimeOfMapCreation = GetWorld()->GetTimeSeconds();
    }
}

void AWTRGameMode::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (GetWorld() && MatchState == MatchState::WaitingToStart)
    {
        CountdownWarmupTime = WarmupTime - GetWorld()->GetTimeSeconds() + TimeOfMapCreation;
        if (CountdownWarmupTime <= 0.f)
        {
            StartMatch();
        }
    }
    else if (GetWorld() && MatchState == MatchState::InProgress)
    {
        CountdownWarmupTime = WarmupTime + MatchTime - GetWorld()->GetTimeSeconds() + TimeOfMapCreation;
        if (CountdownWarmupTime <= 0.f)
        {
            SetMatchState(FName(TEXT("Cooldown")));
        }
    }
}

void AWTRGameMode::OnMatchStateSet()
{
    Super::OnMatchStateSet();

    if (GetWorld())
    {
        for (FConstControllerIterator It = GetWorld()->GetControllerIterator(); It; ++It)
        {
            AWTRPlayerController* WTRController = Cast<AWTRPlayerController>(*It);
            if (WTRController)
            {
                WTRController->SetMatchState(MatchState);
            }
        }
    }
}

void AWTRGameMode::PlayerEliminated(
    AWTRCharacter* EliminatedCharacter, AWTRPlayerController* VictimController, AWTRPlayerController* AttackerController)
{
    AWTRPlayerState* AttackerPlayerState = AttackerController ? Cast<AWTRPlayerState>(AttackerController->PlayerState) : nullptr;
    AWTRPlayerState* VictimPlayerState = VictimController ? Cast<AWTRPlayerState>(VictimController->PlayerState) : nullptr;

    if (AttackerPlayerState && AttackerPlayerState != VictimPlayerState)
    {
        AttackerPlayerState->AddToScore(1.f);
    }
    if (VictimPlayerState)
    {
        VictimPlayerState->AddToDefeats(1);
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
