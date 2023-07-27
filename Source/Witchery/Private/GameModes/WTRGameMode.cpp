// Witchery. Copyright Liemander. All Rights Reserved.

#include "GameModes/WTRGameMode.h"
#include "GameStates/WTRGameState.h"
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

void AWTRGameMode::PostLogin(APlayerController* NewPlayer)
{
    Super::PostLogin(NewPlayer);
}

void AWTRGameMode::BeginPlay()
{
    Super::BeginPlay();

    if (GetWorld())
    {
        TimeOfMapCreation = GetWorld()->GetTimeSeconds();
    }

    if (!bDelayedStart)
    {
        WarmupTime = 0.f;
    }
}

void AWTRGameMode::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Count GameMode time
    if (GetWorld() && MatchState == MatchState::WaitingToStart)
    {
        CountdownTime = WarmupTime - GetWorld()->GetTimeSeconds() + TimeOfMapCreation;
        if (CountdownTime <= 0.f)
        {
            StartMatch();
        }
    }
    else if (GetWorld() && MatchState == MatchState::InProgress)
    {
        CountdownTime = WarmupTime + MatchTime - GetWorld()->GetTimeSeconds() + TimeOfMapCreation;
        if (CountdownTime <= 0.f)
        {
            SetMatchState(FName(TEXT("Cooldown")));
        }
    }
    else if (GetWorld() && MatchState == MatchState::Cooldown)
    {
        CountdownTime = WarmupTime + MatchTime + CooldownTime - GetWorld()->GetTimeSeconds() + TimeOfMapCreation;
        if (CountdownTime <= 0.f)
        {
            RestartGame();
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

    WTRGameState = (WTRGameState == nullptr) ? Cast<AWTRGameState>(UGameplayStatics::GetGameState(this)) : WTRGameState;

    if (AttackerPlayerState && AttackerPlayerState != VictimPlayerState && WTRGameState)
    {
        TArray<AWTRPlayerState*> PrewLeadPlayers;
        for (auto PrewLeadPlayer : WTRGameState->GetTopPlayers())
        {
            PrewLeadPlayers.Add(PrewLeadPlayer);
        }

        AttackerPlayerState->AddToScore(1.f);
        WTRGameState->UpdateTopPlayers(AttackerPlayerState);

        UpdateCrowns(PrewLeadPlayers, AttackerPlayerState);
    }
    if (VictimPlayerState)
    {
        VictimPlayerState->AddToDefeats(1);
    }

    if (EliminatedCharacter)
    {
        EliminatedCharacter->Elim(false);
    }

    if (AttackerPlayerState && VictimPlayerState)
    {
        for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
        {
            AWTRPlayerController* WTRController = Cast<AWTRPlayerController>(*It);
            if (WTRController)
            {
                WTRController->BroadcastElim(AttackerPlayerState, VictimPlayerState);
            }
        }
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

void AWTRGameMode::LeaveGame(AWTRPlayerState* LeavingPlayerState)
{
    if (!LeavingPlayerState) return;

    WTRGameState = (WTRGameState == nullptr) ? Cast<AWTRGameState>(UGameplayStatics::GetGameState(this)) : WTRGameState;
    if (WTRGameState)
    {
        if (WTRGameState->GetTopPlayers().Contains(LeavingPlayerState))
        {
            WTRGameState->GetTopPlayers().Remove(LeavingPlayerState);
        }
    }

    AWTRCharacter* WTRCharacter = Cast<AWTRCharacter>(LeavingPlayerState->GetPawn());
    if (WTRCharacter)
    {
        WTRCharacter->Elim(true);
    }
}

void AWTRGameMode::SendChatMessagesToAllClients(APlayerState* Sender, const FString& Message)
{
    if (!Sender || !GetWorld()) return;

    for (FConstControllerIterator It = GetWorld()->GetControllerIterator(); It; ++It)
    {
        AWTRPlayerController* WTRController = Cast<AWTRPlayerController>(*It);
        if (WTRController)
        {
            WTRController->ApplyChatMessage(Sender, Message);
        }
    }
}

float AWTRGameMode::CalculateDamageByTeams(AController* Attacker, AController* Victim, float BaseDamage)
{
    return BaseDamage;
}

void AWTRGameMode::PlayerStartByTeam(APlayerController* Player)
{
    return;
}

void AWTRGameMode::UpdateCrowns(TArray<AWTRPlayerState*>& PrewLeadPlayers, AWTRPlayerState* AttackerPlayerState)
{
    WTRGameState = (WTRGameState == nullptr) ? Cast<AWTRGameState>(UGameplayStatics::GetGameState(this)) : WTRGameState;
    if (!WTRGameState) return;

    if (WTRGameState->GetTopPlayers().Contains(AttackerPlayerState))
    {
        AWTRCharacter* Leader = Cast<AWTRCharacter>(AttackerPlayerState->GetPawn());
        if (Leader)
        {
            Leader->Multicast_GetLead();
        }
    }

    for (auto PrewLeadPlayer : PrewLeadPlayers)
    {
        if (!WTRGameState->GetTopPlayers().Contains(PrewLeadPlayer))
        {
            AWTRCharacter* Loser = Cast<AWTRCharacter>(PrewLeadPlayer->GetPawn());
            if (Loser)
            {
                Loser->Multicast_LostLead();
            }
        }
    }
}
