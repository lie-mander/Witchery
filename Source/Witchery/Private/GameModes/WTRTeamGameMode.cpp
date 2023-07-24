// Witchery. Copyright Liemander. All Rights Reserved.

#include "GameModes/WTRTeamGameMode.h"
#include "GameStates/WTRGameState.h"
#include "WTRPlayerState.h"
#include "Kismet/GameplayStatics.h"

void AWTRTeamGameMode::PostLogin(APlayerController* NewPlayer)
{
    Super::PostLogin(NewPlayer);

    AWTRGameState* WTRGameState = Cast<AWTRGameState>(UGameplayStatics::GetGameState(this));
    AWTRPlayerState* WTRPlayerState = NewPlayer->GetPlayerState<AWTRPlayerState>();
    if (WTRGameState && WTRPlayerState)
    {
        if (WTRPlayerState->GetTeam() == ETeam::ET_NoTeam)
        {
            if (WTRGameState->RedTeam.Num() >= WTRGameState->BlueTeam.Num())
            {
                WTRGameState->BlueTeam.AddUnique(WTRPlayerState);
            }
            else
            {
                WTRGameState->RedTeam.AddUnique(WTRPlayerState);
            }
        }
    }
}

void AWTRTeamGameMode::Logout(AController* Exiting)
{
    Super::Logout(Exiting);

    AWTRGameState* WTRGameState = Cast<AWTRGameState>(UGameplayStatics::GetGameState(this));
    AWTRPlayerState* WTRPlayerState = Exiting->GetPlayerState<AWTRPlayerState>();
    if (WTRGameState && WTRPlayerState)
    {
        if (WTRPlayerState->GetTeam() == ETeam::ET_RedTeam)
        {
            WTRGameState->RedTeam.Remove(WTRPlayerState);
        }
        else if (WTRPlayerState->GetTeam() == ETeam::ET_BlueTeam)
        {
            WTRGameState->BlueTeam.Remove(WTRPlayerState);
        }
    }
}

void AWTRTeamGameMode::HandleMatchHasStarted()
{
    Super::HandleMatchHasStarted();

    AWTRGameState* WTRGameState = Cast<AWTRGameState>(UGameplayStatics::GetGameState(this));
    if (WTRGameState)
    {
        for (auto State : WTRGameState->PlayerArray)
        {
            AWTRPlayerState* WTRPlayerState = Cast<AWTRPlayerState>(State.Get());
            if (WTRPlayerState && WTRPlayerState->GetTeam() == ETeam::ET_NoTeam)
            {
                if (WTRGameState->RedTeam.Num() >= WTRGameState->BlueTeam.Num())
                {
                    WTRGameState->BlueTeam.AddUnique(WTRPlayerState);
                }
                else
                {
                    WTRGameState->RedTeam.AddUnique(WTRPlayerState);
                }
            }
        }
    }
}
