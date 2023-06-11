// Witchery. Copyright Liemander. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "WTRGameInstance.generated.h"

UCLASS()
class WITCHERY_API UWTRGameInstance : public UGameInstance
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintReadWrite, Category = "WTR | Lobby")
    int32 LobbyPlayersToStart = 3;
};
