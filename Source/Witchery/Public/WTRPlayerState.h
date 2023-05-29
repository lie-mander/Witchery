// Witchery. Copyright Liemander. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "WTRPlayerState.generated.h"

class AWTRCharacter;
class AWTRPlayerController;

UCLASS()
class WITCHERY_API AWTRPlayerState : public APlayerState
{
    GENERATED_BODY()

public:
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
    virtual void OnRep_Score() override;

    UFUNCTION()
    virtual void OnRep_Defeats();

    void AddToScore(float ScoreToAdd);
    void AddToDefeats(int32 DefeatsToAdd);

private:
    UPROPERTY()
    AWTRCharacter* WTRCharacter;

    UPROPERTY()
    AWTRPlayerController* WTRPlayerController;

    UPROPERTY(ReplicatedUsing = OnRep_Defeats)
    int32 Defeats = 0;

    void UpdateHUDScore(float ScoreAmount);
    void UpdateHUDDefeats(int32 DefeatsAmount);
};
