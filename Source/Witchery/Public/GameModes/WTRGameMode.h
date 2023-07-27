// Witchery. Copyright Liemander. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "WTRTypes.h"
#include "WTRGameMode.generated.h"

class AWTRCharacter;
class AWTRPlayerController;
class AWTRPlayerState;
class AWTRGameState;

namespace MatchState
{
extern WITCHERY_API const FName Cooldown;
}

UCLASS()
class WITCHERY_API AWTRGameMode : public AGameMode
{
    GENERATED_BODY()

public:
    AWTRGameMode();
    virtual void PostLogin(APlayerController* NewPlayer) override;
    virtual void PlayerEliminated(
        AWTRCharacter* EliminatedCharacter, AWTRPlayerController* VictimController, AWTRPlayerController* AttackerController);
    virtual void RequestRespawn(ACharacter* EliminatedCharacter, AController* EliminatedController);
    virtual void LeaveGame(AWTRPlayerState* LeavingPlayerState);
    virtual void SendChatMessagesToAllClients(APlayerState* Sender, const FString& Message);
    virtual float CalculateDamageByTeams(AController* Attacker, AController* Victim, float BaseDamage);
    virtual void PlayerStartByTeam(APlayerController* Player);
    virtual void Tick(float DeltaTime);

    FORCEINLINE float GetWarmupTime() const { return WarmupTime; }
    FORCEINLINE float GetMatchTime() const { return MatchTime; }
    FORCEINLINE float GetCooldownTime() const { return CooldownTime; }
    FORCEINLINE float GetTimeOfMapCreation() const { return TimeOfMapCreation; }
    FORCEINLINE float GetCountdownTime() const { return CountdownTime; }
    FORCEINLINE EGameModeType GetGameModeType() const { return GameModeType; }

protected:
    virtual void BeginPlay() override;
    virtual void OnMatchStateSet() override;

    EGameModeType GameModeType = EGameModeType::EGMT_DeathMatch;

    UPROPERTY()
    AWTRGameState* WTRGameState;

private:
    UPROPERTY(EditDefaultsOnly, Category = "WTR | GameTime")
    float WarmupTime = 10.f;

    UPROPERTY(EditDefaultsOnly, Category = "WTR | GameTime")
    float MatchTime = 10.f;

    UPROPERTY(EditDefaultsOnly, Category = "WTR | GameTime")
    float CooldownTime = 10.f;

    float CountdownTime = 0.f;
    float TimeOfMapCreation = 0.f;

    void UpdateCrowns(TArray<AWTRPlayerState*>& PrewLeadPlayers, AWTRPlayerState* AttackerPlayerState);
};
