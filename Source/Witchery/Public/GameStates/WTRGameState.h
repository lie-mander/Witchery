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

    FORCEINLINE TArray<AWTRPlayerState*> GetTopPlayers() const { return TopPlayers; }

private:
    UPROPERTY(Replicated)
    TArray<AWTRPlayerState*> TopPlayers;

    float TopScore = 0.f;
};
