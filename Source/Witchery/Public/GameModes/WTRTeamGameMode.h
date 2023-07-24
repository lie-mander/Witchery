// Witchery. Copyright Liemander. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameModes/WTRGameMode.h"
#include "WTRTeamGameMode.generated.h"

UCLASS()
class WITCHERY_API AWTRTeamGameMode : public AWTRGameMode
{
    GENERATED_BODY()

public:
    virtual void PostLogin(APlayerController* NewPlayer) override;
    virtual void Logout(AController* Exiting) override;

protected:
    virtual void HandleMatchHasStarted() override;
};
