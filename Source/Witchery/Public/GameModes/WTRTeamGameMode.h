// Witchery. Copyright Liemander. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameModes/WTRGameMode.h"
#include "WTRTypes.h"
#include "WTRTeamGameMode.generated.h"

UCLASS()
class WITCHERY_API AWTRTeamGameMode : public AWTRGameMode
{
    GENERATED_BODY()

public:
    AWTRTeamGameMode();
    virtual void PlayerEliminated(
        AWTRCharacter* EliminatedCharacter, AWTRPlayerController* VictimController, AWTRPlayerController* AttackerController) override;
    virtual void PostLogin(APlayerController* NewPlayer) override;
    virtual void Logout(AController* Exiting) override;
    virtual void PlayerStartByTeam(APlayerController* Player) override;
    virtual float CalculateDamageByTeams(AController* Attacker, AController* Victim, float BaseDamage) override;

protected:
    virtual void HandleMatchHasStarted() override;

private:
    void SetTeamToAllPlayers();
};
