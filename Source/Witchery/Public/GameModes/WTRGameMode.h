// Witchery. Copyright Liemander. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "WTRGameMode.generated.h"

class AWTRCharacter;
class AWTRPlayerController;
class AWTRPlayerState;

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
    virtual void PlayerEliminated(
        AWTRCharacter* EliminatedCharacter, AWTRPlayerController* VictimController, AWTRPlayerController* AttackerController);
    virtual void RequestRespawn(ACharacter* EliminatedCharacter, AController* EliminatedController);
    virtual void LeaveGame(AWTRPlayerState* LeavingPlayerState);
    virtual void Tick(float DeltaTime);

    FORCEINLINE float GetWarmupTime() const { return WarmupTime; }
    FORCEINLINE float GetMatchTime() const { return MatchTime; }
    FORCEINLINE float GetCooldownTime() const { return CooldownTime; }
    FORCEINLINE float GetTimeOfMapCreation() const { return TimeOfMapCreation; }
    FORCEINLINE float GetCountdownTime() const { return CountdownTime; }

protected:
    virtual void BeginPlay() override;
    virtual void OnMatchStateSet() override;

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
