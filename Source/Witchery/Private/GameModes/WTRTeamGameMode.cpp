// Witchery. Copyright Liemander. All Rights Reserved.

#include "GameModes/WTRTeamGameMode.h"
#include "GameStates/WTRGameState.h"
#include "WTRPlayerState.h"
#include "Kismet/GameplayStatics.h"
#include "Character/WTRPlayerController.h"
#include "Character/WTRCharacter.h"
#include "PlayerStarts/WTRTeamPlayerStart.h"

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
                SpawnNewPlayerInCorrectSpot(NewPlayer);
            }
            else
            {
                WTRGameState->RedTeam.AddUnique(WTRPlayerState);
                WTRPlayerState->SetTeam(ETeam::ET_RedTeam);
                SpawnNewPlayerInCorrectSpot(NewPlayer);
            }
        }
    }
    else if (MatchState == MatchState::InProgress)
    {
        AWTRPlayerController* WTRPlayerController = Cast<AWTRPlayerController>(NewPlayer);
        if (WTRPlayerController)
        {
            WTRPlayerController->bNeedToSpawnByTeam = true;
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

void AWTRTeamGameMode::PlayerStartByTeam(APlayerController* Player)
{
    if (!Player || !Player->GetPawn()) return;

    AWTRPlayerState* WTRPlayerState = Player->GetPlayerState<AWTRPlayerState>();
    if (!WTRPlayerState) return;

    APlayerStart* PlayerStart = nullptr;
    if (WTRPlayerState->GetTeam() == ETeam::ET_RedTeam)
    {
        PlayerStart = Cast<APlayerStart>(FindPlayerStart(Player, FString("RedTeam")));
        if (!PlayerStart)
        {
            PlayerStart = Cast<APlayerStart>(FindPlayerStart(Player, FString("RedTeamNext")));
        }
    }
    else if (WTRPlayerState->GetTeam() == ETeam::ET_BlueTeam)
    {
        PlayerStart = Cast<APlayerStart>(FindPlayerStart(Player, FString("BlueTeam")));
        if (!PlayerStart)
        {
            PlayerStart = Cast<APlayerStart>(FindPlayerStart(Player, FString("BlueTeamNext")));
        }
    }

    if (PlayerStart)
    {
        Player->GetPawn()->SetActorTransform(PlayerStart->GetActorTransform());

        if (WTRPlayerState->GetTeam() == ETeam::ET_RedTeam)
            PlayerStart->PlayerStartTag = "RedTeamNext";
        else if (WTRPlayerState->GetTeam() == ETeam::ET_BlueTeam)
            PlayerStart->PlayerStartTag = "BlueTeamNext";
    }
}

void AWTRTeamGameMode::RequestRespawn(ACharacter* EliminatedCharacter, AController* EliminatedController)
{
    if (EliminatedCharacter)
    {
        EliminatedCharacter->Reset();
        EliminatedCharacter->Destroy();
    }

    if (EliminatedController)
    {
        AWTRPlayerState* WTRPlayerState = EliminatedController->GetPlayerState<AWTRPlayerState>();
        if (!WTRPlayerState) return;

        APlayerStart* PlayerStart = (WTRPlayerState->GetTeam() == ETeam::ET_RedTeam)
                                        ? Cast<APlayerStart>(FindPlayerStart(EliminatedController, FString("RedTeamNext")))
                                        : Cast<APlayerStart>(FindPlayerStart(EliminatedController, FString("BlueTeamNext")));

        RestartPlayerAtPlayerStart(EliminatedController, PlayerStart);
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

    SetTeamToAllPlayers();
}

void AWTRTeamGameMode::SetTeamToAllPlayers()
{
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

void AWTRTeamGameMode::SpawnNewPlayerInCorrectSpot(APlayerController* NewPlayer)
{
    if (!NewPlayer) return;
    
    if (MatchState == MatchState::InProgress)
    {
        AWTRPlayerState* WTRPlayerState = NewPlayer->GetPlayerState<AWTRPlayerState>();
        if (!WTRPlayerState) return;

        if (WTRPlayerState->GetTeam() == ETeam::ET_BlueTeam)
        {
            APlayerStart* PlayerStart = Cast<APlayerStart>(FindPlayerStart(NewPlayer, FString("BlueTeam")));
            if (NewPlayer->GetPawn() && PlayerStart)
            {
                NewPlayer->GetPawn()->SetActorTransform(PlayerStart->GetActorTransform());
                PlayerStart->PlayerStartTag = "BlueTeamNext";
            }
            else if (NewPlayer->GetPawn() && !PlayerStart)
            {
                PlayerStart = Cast<APlayerStart>(FindPlayerStart(NewPlayer, FString("BlueTeamNext")));
                if (PlayerStart)
                {
                    NewPlayer->GetPawn()->SetActorTransform(PlayerStart->GetActorTransform());
                }
            }
        }
        else if (WTRPlayerState->GetTeam() == ETeam::ET_RedTeam)
        {
            APlayerStart* PlayerStart = Cast<APlayerStart>(FindPlayerStart(NewPlayer, FString("RedTeam")));
            if (NewPlayer->GetPawn() && PlayerStart)
            {
                NewPlayer->GetPawn()->SetActorTransform(PlayerStart->GetActorTransform());
                PlayerStart->PlayerStartTag = "RedTeamNext";
            }
            else if (NewPlayer->GetPawn() && !PlayerStart)
            {
                PlayerStart = Cast<APlayerStart>(FindPlayerStart(NewPlayer, FString("RedTeamNext")));
                if (PlayerStart)
                {
                    NewPlayer->GetPawn()->SetActorTransform(PlayerStart->GetActorTransform());
                }
            }
        }
    }
}
