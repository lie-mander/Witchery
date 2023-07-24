// Witchery. Copyright Liemander. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "WTRGameState.generated.h"

class AWTRPlayerState;

UCLASS()
class WITCHERY_API AWTRGameState : public AGameState
{
    GENERATED_BODY()

public:
    void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    void UpdateTopPlayers(AWTRPlayerState* PlayerState);

    FORCEINLINE TArray<AWTRPlayerState*> GetTopPlayers() { return TopPlayers; }

private:
    UPROPERTY(Replicated)
    TArray<AWTRPlayerState*> TopPlayers;

    float TopScore = 0.f;

    /*
     * Teams
     */
    TArray<AWTRPlayerState*> RedTeam;
    TArray<AWTRPlayerState*> BlueTeam;

    UPROPERTY(ReplicatedUsing = OnRep_RedTeamScore)
    float RedTeamScore = 0.f;

    UPROPERTY(ReplicatedUsing = OnRep_BlueTeamScore)
    float BlueTeamScore = 0.f;

    UFUNCTION()
    void OnRep_RedTeamScore();

    UFUNCTION()
    void OnRep_BlueTeamScore();
};
