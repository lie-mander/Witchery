// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "OnlineSessionSettings.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "MenuWidget.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMenuButtonsClicked, bool, bIsClicked);

UCLASS()
class MULTIPLAYERSESSIONS_API UMenuWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable)
    void SetupMenu(int32 NumberPublicConnection_ = 4, const FString& MatchType_ = FString(TEXT("DIR")),
        const FString& LobbyPath_ = FString(TEXT("/Game/ThirdPerson/Maps/Lobby")), bool NeedDebug_ = false);

    UPROPERTY(BlueprintAssignable)
    FOnMenuButtonsClicked OnMenuButtonsClicked;

protected:
    virtual bool Initialize() override;
    virtual void NativeDestruct() override;

    //////////
    // Callback functions for our custom delegates
    //

    UFUNCTION()
    void OnCreateSession(bool bWasSuccessful);

    void OnFindSessions(const TArray<FOnlineSessionSearchResult>& Results, bool bWasSuccessful);

    void OnJoinSession(EOnJoinSessionCompleteResult::Type Result);

    UFUNCTION()
    void OnDestroySession(bool bWasSuccessful);

    UFUNCTION()
    void OnStartSession(bool bWasSuccessful);

private:
    int32 NumberPublicConnection = 4;
    FString MatchType = FString(TEXT("DIR"));
    FString LobbyPath = FString(TEXT(""));
    bool NeedDebug = false;

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Menu", meta = (AllowPrivateAccess = "true"))
    int32 NumberPlayersToStart = 3;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Menu", meta = (AllowPrivateAccess = "true"))
    int32 DefaultNumberPlayersToStart = 3;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Menu", meta = (AllowPrivateAccess = "true"))
    int32 MaxNumberPlayersToStart = 50;

    UPROPERTY(meta = (BindWidget))
    class UButton* HostButton;

    UPROPERTY(meta = (BindWidget))
    UButton* JoinButton;

    UFUNCTION()
    void HostButtonClicked();

    UFUNCTION()
    void JoinButtonClicked();

    void MenuTearDown();

    // Our multiplayer sessions subsystem for connect menu with it.
    class UMultiplayerSessionsSubsystem* MultiplayerSessionsSubsystem;
};
