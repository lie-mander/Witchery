// Witchery. Copyright Liemander. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "LobbyGameMode.generated.h"

UCLASS()
class WITCHERY_API ALobbyGameMode : public AGameMode
{
    GENERATED_BODY()

public:
    virtual void PostLogin(APlayerController* NewPlayer) override;

protected:
    UPROPERTY(EditAnywhere, Category = "Lobby Config")
    int32 PlayersToStart = 3;
};
