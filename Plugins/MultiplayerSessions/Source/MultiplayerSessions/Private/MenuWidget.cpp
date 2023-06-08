// Fill out your copyright notice in the Description page of Project Settings.

#include "MenuWidget.h"
#include "Components/Button.h"
#include "Components/EditableTextBox.h"
#include "MultiplayerSessionsSubsystem.h"
#include "OnlineSubsystem.h"

void UMenuWidget::SetupMenu(int32 NumberPublicConnection_, const FString& MatchType_, const FString& LobbyPath_, bool NeedDebug_)
{
    NumberPublicConnection = NumberPublicConnection_;
    MatchType = MatchType_;
    LobbyPath = FString::Printf(TEXT("%s?listen"), *LobbyPath_);
    NeedDebug = NeedDebug_;

    AddToViewport();
    SetVisibility(ESlateVisibility::Visible);
    SetIsFocusable(true);

    const auto World = GetWorld();
    if (World)
    {
        auto PlayerController = World->GetFirstPlayerController();
        if (PlayerController)
        {
            FInputModeUIOnly InputModeData;
            InputModeData.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
            InputModeData.SetWidgetToFocus(TakeWidget());
            PlayerController->SetInputMode(InputModeData);
            PlayerController->SetShowMouseCursor(true);
        }
    }

    const auto GameInstance = GetGameInstance();
    if (GameInstance)
    {
        MultiplayerSessionsSubsystem = GameInstance->GetSubsystem<UMultiplayerSessionsSubsystem>();
    }

    if (MultiplayerSessionsSubsystem)
    {
        MultiplayerSessionsSubsystem->MultiplayerOnCreateSessionComplete.AddDynamic(this, &ThisClass::OnCreateSession);
        MultiplayerSessionsSubsystem->MultiplayerOnFindSessionsComplete.AddUObject(this, &ThisClass::OnFindSessions);
        MultiplayerSessionsSubsystem->MultiplayerOnJoinSessionComplete.AddUObject(this, &ThisClass::OnJoinSession);
        MultiplayerSessionsSubsystem->MultiplayerOnDestroySessionComplete.AddDynamic(this, &ThisClass::OnDestroySession);
        MultiplayerSessionsSubsystem->MultiplayerOnStartSessionComplete.AddDynamic(this, &ThisClass::OnStartSession);
    }
}

bool UMenuWidget::Initialize()
{
    if (!Super::Initialize()) return false;

    if (!HostButton || !JoinButton) return false;

    HostButton->OnClicked.AddDynamic(this, &ThisClass::HostButtonClicked);
    JoinButton->OnClicked.AddDynamic(this, &ThisClass::JoinButtonClicked);

    if (NeedDebug && GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 20.f, FColor::Orange, "Successful initialize menu");
    }

    return true;
}

void UMenuWidget::NativeDestruct() 
{
    MenuTearDown();
    Super::NativeDestruct();
}

void UMenuWidget::OnCreateSession(bool bWasSuccessful)
{
    if (bWasSuccessful)
    {
        if (NeedDebug && GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 20.f, FColor::Orange, "Successful create session");
        }
        const auto World = GetWorld();
        if (World)
        {
            if (NeedDebug && GEngine)
            {
                GEngine->AddOnScreenDebugMessage(-1, 20.f, FColor::Orange, "Starting server travel..");
            }
            World->ServerTravel(LobbyPath);
        }
    }
    else
    {
        if (NeedDebug && GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 20.f, FColor::Red, "Failed create session");
        }
        HostButton->SetIsEnabled(true);
        JoinButton->SetIsEnabled(true);

        OnMenuButtonsClicked.Broadcast(false);
    }
}

void UMenuWidget::OnFindSessions(const TArray<FOnlineSessionSearchResult>& Results, bool bWasSuccessful)
{
    if (!MultiplayerSessionsSubsystem) return;

    if (NeedDebug && GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 20.f, FColor::Orange, "Sessions found successful");
    }

    for (auto Result : Results)
    {
        FString SettingsValue;
        Result.Session.SessionSettings.Get(FName("MatchType"), SettingsValue);
        if (SettingsValue == MatchType)
        {
            if (NeedDebug && GEngine)
            {
                GEngine->AddOnScreenDebugMessage(-1, 20.f, FColor::Orange, "A suitable session is found successfully");
            }
            MultiplayerSessionsSubsystem->JoinSession(Result);
            return;
        }
    }

    if (NeedDebug && GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 20.f, FColor::Red, "A suitable session is found failed");
    }

    HostButton->SetIsEnabled(true);
    JoinButton->SetIsEnabled(true);

    OnMenuButtonsClicked.Broadcast(false);
}

void UMenuWidget::OnJoinSession(EOnJoinSessionCompleteResult::Type Result)
{
    IOnlineSubsystem* OnlineSubSystem = IOnlineSubsystem::Get();
    if (OnlineSubSystem)
    {
        if (NeedDebug && GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 20.f, FColor::Orange, "OnlineSubSystem is valid");
        }
        IOnlineSessionPtr SessionInterface = OnlineSubSystem->GetSessionInterface();
        if (SessionInterface.IsValid())
        {
            if (NeedDebug && GEngine)
            {
                GEngine->AddOnScreenDebugMessage(-1, 20.f, FColor::Orange, "SessionInterface is valid");
            }
            FString Address;
            if (SessionInterface->GetResolvedConnectString(NAME_GameSession, Address))
            {
                if (NeedDebug && GEngine)
                {
                    GEngine->AddOnScreenDebugMessage(-1, 20.f, FColor::Orange, FString::Printf(TEXT("Address: %s"), *Address));
                }
                APlayerController* PlayerController = GetGameInstance()->GetFirstLocalPlayerController();
                if (PlayerController)
                {
                    if (NeedDebug && GEngine)
                    {
                        GEngine->AddOnScreenDebugMessage(-1, 20.f, FColor::Orange, "Starting client travel..");
                    }
                    PlayerController->ClientTravel(Address, TRAVEL_Absolute);
                }
            }
        }
    }

    if (Result != EOnJoinSessionCompleteResult::Success)
    {
        if (NeedDebug && GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 20.f, FColor::Red, "Join session result is`t success");
        }
        HostButton->SetIsEnabled(true);
        JoinButton->SetIsEnabled(true);

        OnMenuButtonsClicked.Broadcast(false);
    }
}

void UMenuWidget::OnDestroySession(bool bWasSuccessful) {}

void UMenuWidget::OnStartSession(bool bWasSuccessful) {}

void UMenuWidget::HostButtonClicked()
{
    HostButton->SetIsEnabled(false);
    JoinButton->SetIsEnabled(false);

    OnMenuButtonsClicked.Broadcast(true);

    if (MultiplayerSessionsSubsystem)
    {
        MultiplayerSessionsSubsystem->CreateSession(NumberPublicConnection, MatchType, NeedDebug);
    }
}

void UMenuWidget::JoinButtonClicked()
{
    HostButton->SetIsEnabled(false);
    JoinButton->SetIsEnabled(false);

    OnMenuButtonsClicked.Broadcast(true);

    if (MultiplayerSessionsSubsystem)
    {
        MultiplayerSessionsSubsystem->FindSessions(10000, NeedDebug);
    }
}

void UMenuWidget::MenuTearDown()
{
    RemoveFromParent();

    const auto World = GetWorld();
    if (World)
    {
        auto PlayerController = World->GetFirstPlayerController();
        if (PlayerController)
        {
            FInputModeGameOnly InputModeData;
            PlayerController->SetInputMode(InputModeData);
            PlayerController->SetShowMouseCursor(false);
        }
    }
}
