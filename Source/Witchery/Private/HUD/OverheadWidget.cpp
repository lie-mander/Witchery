// Witchery. Copyright Liemander. All Rights Reserved.

#include "HUD/OverheadWidget.h"
#include "Components/TextBlock.h"

void UOverheadWidget::ShowPlayerNetRole(APawn* InPawn)
{
    ENetRole RemoteRole = InPawn->GetRemoteRole();
    FString Role;
    switch (RemoteRole)
    {
        case ROLE_None: Role = FString("None"); break;
        case ROLE_SimulatedProxy: Role = FString("Simulated Proxy"); break;
        case ROLE_AutonomousProxy: Role = FString("Autonomous Proxy"); break;
        case ROLE_Authority: Role = FString("Authority"); break;
    }

    FString RemoteRoleString = FString::Printf(TEXT("Remote role -> %s"), *Role);
    SetDisplayText(RemoteRoleString);
}

void UOverheadWidget::SetDisplayText(FString Text)
{
    if (OverheadText)
    {
        OverheadText->SetText(FText::FromString(Text));
    }
}

void UOverheadWidget::OnLevelRemovedFromWorld(ULevel* InLevel, UWorld* InWorld)
{
    RemoveFromParent();
    Super::OnLevelRemovedFromWorld(InLevel, InWorld);
}
