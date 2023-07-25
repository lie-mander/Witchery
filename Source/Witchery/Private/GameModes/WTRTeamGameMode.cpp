// Witchery. Copyright Liemander. All Rights Reserved.

#include "GameModes/WTRTeamGameMode.h"
#include "GameStates/WTRGameState.h"
#include "WTRPlayerState.h"
#include "Kismet/GameplayStatics.h"
#include "Character/WTRPlayerController.h"
#include "WTRPlayerState.h"

AWTRTeamGameMode::AWTRTeamGameMode() 
{
    GameModeType = EGameModeType::EGMT_TeamsMatch;
}

void AWTRTeamGameMode::PlayerEliminated(
    AWTRCharacter* EliminatedCharacter, AWTRPlayerController* VictimController, AWTRPlayerController* AttackerController)
{
    Super::PlayerEliminated(EliminatedCharacter, VictimController, AttackerController);

    WTRGameState = (WTRGameState == nullptr) ? Cast<AWTRGameState>(UGameplayStatics::GetGameState(this)) : WTRGameState;
    if (AttackerController && WTRGameState)
    {
        AWTRPlayerState* WTRPlayerState = AttackerController->GetPlayerState<AWTRPlayerState>();
        if (VictimController == AttackerController)
        {
            return;
        }
        else if (WTRPlayerState->GetTeam() == ETeam::ET_RedTeam)
        {
            WTRGameState->RedTeamScores();
        }
        else if (WTRPlayerState->GetTeam() == ETeam::ET_BlueTeam)
        {
            WTRGameState->BlueTeamScores();
        }
    }
}

void AWTRTeamGameMode::PostLogin(APlayerController* NewPlayer)
{
    Super::PostLogin(NewPlayer);

    WTRGameState = (WTRGameState == nullptr) ? Cast<AWTRGameState>(UGameplayStatics::GetGameState(this)) : WTRGameState;
    AWTRPlayerState* WTRPlayerState = NewPlayer->GetPlayerState<AWTRPlayerState>();
    if (WTRGameState && WTRPlayerState)
    {
        if (WTRPlayerState->GetTeam() == ETeam::ET_NoTeam)
        {
            if (WTRGameState->RedTeam.Num() >= WTRGameState->BlueTeam.Num())
            {
                WTRGameState->BlueTeam.AddUnique(WTRPlayerState);
                WTRPlayerState->SetTeam(ETeam::ET_BlueTeam);
            }
            else
            {
                WTRGameState->RedTeam.AddUnique(WTRPlayerState);
                WTRPlayerState->SetTeam(ETeam::ET_RedTeam);
            }
        }
    }
}

void AWTRTeamGameMode::Logout(AController* Exiting)
{
    Super::Logout(Exiting);

    WTRGameState = (WTRGameState == nullptr) ? Cast<AWTRGameState>(UGameplayStatics::GetGameState(this)) : WTRGameState;
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

float AWTRTeamGameMode::CalculateDamageByTeams(AController* Attacker, AController* Victim, float BaseDamage)
{
    if (!Attacker || !Victim) return BaseDamage;
    if (Attacker == Victim) return BaseDamage;

    // If players in the same team, return 0 damage
    AWTRPlayerState* AttackerState = Attacker->GetPlayerState<AWTRPlayerState>();
    AWTRPlayerState* VictimState = Victim->GetPlayerState<AWTRPlayerState>();
    if (AttackerState && VictimState && AttackerState->GetTeam() == VictimState->GetTeam())
    {
        return 0.f;
    }

    return BaseDamage;
}

void AWTRTeamGameMode::HandleMatchHasStarted()
{
    Super::HandleMatchHasStarted();

    WTRGameState = (WTRGameState == nullptr) ? Cast<AWTRGameState>(UGameplayStatics::GetGameState(this)) : WTRGameState;
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
                    WTRPlayerState->SetTeam(ETeam::ET_BlueTeam);
                }
                else
                {
                    WTRGameState->RedTeam.AddUnique(WTRPlayerState);
                    WTRPlayerState->SetTeam(ETeam::ET_RedTeam);
                }
            }
        }
    }
}
