// Witchery. Copyright Liemander. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "WTRTypes.h"
#include "WTRPlayerController.generated.h"

class AWTR_HUD;
class AWTRCharacter;
class UWTRCharacterOverlayWidget;

UCLASS()
class WITCHERY_API AWTRPlayerController : public APlayerController
{
    GENERATED_BODY()

public:
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
    virtual void OnPossess(APawn* InPawn) override;
    virtual void ReceivedPlayer() override;
    virtual void Tick(float DeltaTime) override;
    virtual void DelayInit();

    void SetMatchState(const FName& State);

    void SetHUDHealth(float CurrentHealth, float MaxHealth);
    void SetHUDScore(float ScoreAmount);
    void SetHUDDefeats(int32 DefeatsAmount);
    void SetHUDDeathMessage(bool bVisible);
    void SetHUDWeaponAmmo(int32 AmmoAmount);
    void SetHUDCarriedAmmo(int32 AmmoAmount);
    void SetHUDWeaponType(EWeaponType Type);
    void SetHUDMatchCountdownTime(float Time);
    void SetHUDWarmupTime(float Time);

protected:
    virtual void BeginPlay() override;
    virtual float GetServerTime();

private:
    //////////
    // Base variables
    //
    UPROPERTY()
    AWTR_HUD* WTR_HUD;

    UPROPERTY()
    AWTRCharacter* WTRCharacter;

    UPROPERTY()
    UWTRCharacterOverlayWidget* CharacterOverlay;

    //////////
    // Match states
    //
    UPROPERTY(ReplicatedUsing = OnRep_MatchState)
    FName MatchState;

    UFUNCTION()
    void OnRep_MatchState();

    void HandleMatchStateInProgress();
    void HandleMatchCooldown();

    //////////
    // Sync client time to server
    //
    UPROPERTY(EditDefaultsOnly, Category = "Time")
    float TimeSyncUpdateFrequency = 5.f;

    float TimeToSyncUpdate = 0.f;

    // Time difference between the server and the client
    float ClientServerTimeDelta = 0.f;

    void UpdateSyncTime(float DeltaTime);

    // Sending the client time to the server to get the current time on the server
    UFUNCTION(Server, Reliable)
    void Server_SendClientTime(float ClientTimeOfSending);

    // RPC to reply from the server, which returns the current server time and the time when the client made the request to calculate the
    // round trip. Must be called from Server_SendClientTime
    UFUNCTION(Client, Reliable)
    void Client_SendServerTime(float ClientTimeOfSending, float ServerTimeResponse);

    //////////
    // Delay Init variables (need to be CharacterOverlay was created, and after that variables can set)
    //
    float DelayInit_CurrentHealth = 0.f;
    float DelayInit_MaxHealth = 0.f;
    float DelayInit_ScoreAmount = 0.f;
    int32 DelayInit_DefeatsAmount = 0;

    //////////
    // MatchType timers
    //
    UFUNCTION(Server, Reliable)
    void Server_CheckMatchState();

    UFUNCTION(Client, Reliable)
    void Client_ApplyMatchState(float TimeofWarmup, float TimeOfMatch, float TimeOfCooldown, float MapCreationTime, const FName& State);

    float WarmupTime = 0.f;
    float MatchTime = 0.f;
    float CooldownTime = 0.f;
    float TimeOfMapCreation = 0.f;

    // To use the SetHUDTime every second, not in a Tick
    int Previous = 0;
    int SecondsInteger = 0;

    //////////
    // UI
    //
    UPROPERTY(EditDefaultsOnly, Category = "UI")
    FString AnnounCooldownText = "NEW GAME STARTS IN:";

    UPROPERTY(EditDefaultsOnly, Category = "UI")
    FString AnnounInfoText = "NONE";

    //////////
    // Functions
    //
    UFUNCTION(Client, Reliable)
    void Client_OnPossess();

    AWTR_HUD* GetWTR_HUD();
    void SetHUDTime();
};
