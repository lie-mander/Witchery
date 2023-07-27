// Witchery. Copyright Liemander. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "WTRTypes.h"
#include "WTRPlayerController.generated.h"

class AWTR_HUD;
class AWTRCharacter;
class AWTRGameMode;
class APlayerState;
class UWTRCharacterOverlayWidget;
class UWTRAnnouncementWidget;
class USoundClass;
class UUserWidget;
class UWTRReturnToMainMenu;

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

    virtual float GetServerTime();

    void SetMatchState(const FName& State);
    void BroadcastElim(const APlayerState* AttackerState, const APlayerState* VictimState);

    void TurnDownTheVolume();
    void VolumeUp();
    void ShowTeamsScore();
    void HideTeamsScore();

    void SetHUDHealth(float CurrentHealth, float MaxHealth);
    void SetHUDShield(float CurrentShield, float MaxShield);
    void SetHUDScore(float ScoreAmount);
    void SetHUDDefeats(int32 DefeatsAmount);
    void SetHUDDeathMessage(bool bVisible);
    void SetHUDWeaponAmmo(int32 AmmoAmount);
    void SetHUDCarriedAmmo(int32 AmmoAmount);
    void SetHUDWeaponType(EWeaponType Type);
    void SetHUDMatchCountdownTime(float Time);
    void SetHUDWarmupTime(float Time);
    void SetHUD_FPS();
    void SetHUDGrenades(int32 Grenades);
    void SetHUDRedScore(int32 Score);
    void SetHUDBlueScore(int32 Score);

    UFUNCTION(Server, Reliable)
    void Server_CheckMatchState();

    AWTR_HUD* GetWTR_HUD();
    FORCEINLINE FName GetMatchState() const { return MatchState; }

    FIsPingHigh IsPingHighDelegate;
    FOnMatchStateChanged OnMatchStateChanged;

    float SingleTripTime = 0.f;

    /*
     * Chat
     */
    bool bChatOpen = false;

    void ApplyChatMessage(APlayerState* Sender, const FString& Message);

    UFUNCTION(Client, Reliable)
    void Client_ApplyChatMessage(APlayerState* Sender, const FString& Message);

    UFUNCTION(Server, Reliable)
    void Server_SendChatMessage(APlayerState* Sender, const FString& Message);

protected:
    virtual void BeginPlay() override;
    virtual void SetupInputComponent() override;

private:
    /*
     * Base variables
     */
    UPROPERTY()
    AWTR_HUD* WTR_HUD;

    UPROPERTY()
    AWTRCharacter* WTRCharacter;

    UPROPERTY()
    AWTRGameMode* WTRGameMode;

    EGameModeType GameModeType = EGameModeType::EGMT_MAX;

    UPROPERTY()
    UWTRCharacterOverlayWidget* CharacterOverlay;

    UPROPERTY()
    UWTRAnnouncementWidget* AnnouncementWidget;

    UPROPERTY(EditDefaultsOnly, Category = "WTR | Debug")
    bool bShowTime = false;

    UPROPERTY(EditDefaultsOnly, Category = "WTR | Debug")
    bool bShowFPS = false;

    UPROPERTY(EditDefaultsOnly, Category = "WTR | Debug")
    bool bShowDelayInit = false;

    /*
     * Sound
     */
    UPROPERTY(EditDefaultsOnly, Category = "WTR | Sound")
    USoundClass* MasterSoundClass;

    /*
     * Match states
     */
    UPROPERTY(ReplicatedUsing = OnRep_MatchState)
    FName MatchState = NAME_None;

    UFUNCTION()
    void OnRep_MatchState();

    void HandleMatchStateWaitingToStart();
    void HandleMatchStateInProgress();
    void HandleMatchCooldown();

    /*
     * Sync client time to server
     */
    UPROPERTY(EditDefaultsOnly, Category = "WTR | Time")
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

    /*
     * Delay Init variables (need to be CharacterOverlay was created, and after that variables can set)
     */
    float DelayInit_CurrentHealth = 0.f;
    float DelayInit_MaxHealth = 0.f;
    float DelayInit_CurrentShield = 0.f;
    float DelayInit_MaxShield = 0.f;
    float DelayInit_ScoreAmount = 0.f;
    int32 DelayInit_DefeatsAmount = 0;
    int32 DelayInit_WeaponAmmo = 0;
    int32 DelayInit_CarriedAmmo = 0;
    EWeaponType DelayInit_WeaponType = EWeaponType::EWT_MAX;

    bool bDelayInit_AnnouncementWidget = false;
    bool bDelayInit_CharacterOverlayDelayInit = false;

    /*
     * MatchType timers
     */
    UFUNCTION(Client, Reliable)
    void Client_ApplyMatchState(float TimeofWarmup, float TimeOfMatch, float TimeOfCooldown, const FName& State, EGameModeType GameType);

    float WarmupTime = 0.f;
    float MatchTime = 0.f;
    float CooldownTime = 0.f;

    UPROPERTY(Replicated)
    float TimeOfMapCreation = 0.f;

    // To use the SetHUDTime every second, not in a Tick
    int Previous = 0;
    int SecondsInteger = 0;

    /*
     * UI
     */
    UPROPERTY(EditDefaultsOnly, Category = "WTR | UI")
    FString AnnounInfoText = "NO WINNER.";

    UPROPERTY(EditDefaultsOnly, Category = "WTR | UI")
    FString AnnounCooldownText = "NEW GAME STARTS IN:";

    UPROPERTY(EditDefaultsOnly, Category = "WTR | UI | DeathMatch")
    FString TextYouWinner = "YOU`RE A WINNER!";

    UPROPERTY(EditDefaultsOnly, Category = "WTR | UI | TeamsMatch")
    FString TextTeamFoughtForWin = "TEAMS FOUGHT FOR WIN!";

    UPROPERTY(EditDefaultsOnly, Category = "WTR | UI")
    float BlinkStartTime = 30.f;

    UPROPERTY(EditDefaultsOnly, Category = "WTR | UI")
    TSubclassOf<UUserWidget> ReturnToMainMenuWidgetClass;

    UPROPERTY()
    UWTRReturnToMainMenu* ReturnToMainMenuWidget;

    bool bReturnToMainMenuOpen = false;

    /*
     * FPS timer
     */
    UPROPERTY(EditDefaultsOnly, Category = "WTR | Time")
    float TimeFPSUpdateFrequency = 2.f;

    float TimeToFPSUpdate = 0.f;

    float FPS = 0.f;

    /*We are showing ping every Frequency and after that waiting Duration*/
    UPROPERTY(EditDefaultsOnly, Category = "WTR | Ping")
    float ShowPingFrequency = 5.f;

    UPROPERTY(EditDefaultsOnly, Category = "WTR | Ping")
    float ShowPingDuration = 7.f;

    UPROPERTY(EditDefaultsOnly, Category = "WTR | Ping")
    float PingThreshold = 50.f;

    float ShowPingFrequencyRuntime = 0.f;
    float ShowPingDurationRuntime = 0.f;

    UFUNCTION(Server, Reliable)
    void Server_ReportHighPingStatus(bool bHighPing);

    /*
     * Functions
     */
    UFUNCTION(Client, Reliable)
    void Client_OnPossess();

    UFUNCTION(Client, Reliable)
    void Client_ElimAnnouncement(const APlayerState* AttackerState, const APlayerState* VictimState);

    void SetHUDTime();
    void Debug_ShowHUDTime();
    void ShowFPS(float DeltaTime);
    void PingTick(float DeltaTime);
    void ShowPing();
    void HidePing();
    void OnQuitButtonPressed();
    void OnChatButtonPressed();
    FString GetCooldownDeathMatchText();
    FString GetCooldownTeamsMatchText();
};
