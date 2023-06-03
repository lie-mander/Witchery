// Witchery. Copyright Liemander. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "WTRGameMode.generated.h"

class AWTRCharacter;
class AWTRPlayerController;

UCLASS()
class WITCHERY_API AWTRGameMode : public AGameMode
{
    GENERATED_BODY()

public:
    AWTRGameMode();
    virtual void PlayerEliminated(
        AWTRCharacter* EliminatedCharacter, AWTRPlayerController* VictimController, AWTRPlayerController* AttackerController);
    virtual void RequestRespawn(ACharacter* EliminatedCharacter, AController* EliminatedController);
    virtual void Tick(float DeltaTime);

    FORCEINLINE float GetWarmupTime() const { return WarmupTime; }
    FORCEINLINE float GetMatchTime() const { return MatchTime; }
    FORCEINLINE float GetTimeOfMapCreation() const { return TimeOfMapCreation; }

protected:
    virtual void BeginPlay() override;
    virtual void OnMatchStateSet() override;

private:
    UPROPERTY(EditDefaultsOnly, Category = "GameTime | WaitingStart")
    float WarmupTime = 10.f;

    UPROPERTY(EditDefaultsOnly, Category = "GameTime | InProgress")
    float MatchTime = 10.f;

    float CountdownWarmupTime = 0.f;
    float TimeOfMapCreation = 0.f;
};
