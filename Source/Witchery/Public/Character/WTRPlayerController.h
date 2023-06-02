// Witchery. Copyright Liemander. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "WTRTypes.h"
#include "WTRPlayerController.generated.h"

class AWTR_HUD;
class AWTRCharacter;

UCLASS()
class WITCHERY_API AWTRPlayerController : public APlayerController
{
    GENERATED_BODY()

public:
    virtual void OnPossess(APawn* InPawn) override;
    virtual void ReceivedPlayer() override;
    virtual void Tick(float DeltaTime) override;

    void SetHUDHealth(float CurrentHealth, float MaxHealth);
    void SetHUDScore(float ScoreAmount);
    void SetHUDDefeats(int32 DefeatsAmount);
    void SetHUDDeathMessage(bool bVisible);
    void SetHUDWeaponAmmo(int32 AmmoAmount);
    void SetHUDCarriedAmmo(int32 AmmoAmount);
    void SetHUDWeaponType(EWeaponType Type);
    void SetHUDMatchCountdownTime(float Time);

protected:
    virtual void BeginPlay() override;
    virtual float GetServerTime();

private:
    UPROPERTY()
    AWTR_HUD* WTR_HUD;

    UPROPERTY()
    AWTRCharacter* WTRCharacter;

    UFUNCTION(Client, Reliable)
    void Client_OnPossess();

    //////////
    // Sync client time to server
    //
    UPROPERTY(EditDefaultsOnly, Category = "Time")
    float TimeSyncUpdateFrequency = 5.f;

    float TimeToSyncUpdate = 0.f;

    // Time difference between the server and the client
    float ClientServerTimeDelta = 0.f;

    // For test timer
    float WorldTime = 0.f;

    void UpdateSyncTime(float DeltaTime);

    // Sending the client time to the server to get the current time on the server
    UFUNCTION(Server, Reliable)
    void Server_SendClientTime(float ClientTimeOfSending);

    // RPC to reply from the server, which returns the current server time and the time when the client made the request to calculate the
    // round trip. Must be called from Server_SendClientTime
    UFUNCTION(Client, Reliable)
    void Client_SendServerTime(float ClientTimeOfSending, float ServerTimeResponse);
};
