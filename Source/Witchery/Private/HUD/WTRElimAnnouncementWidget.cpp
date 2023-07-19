// Witchery. Copyright Liemander. All Rights Reserved.

#include "HUD/WTRElimAnnouncementWidget.h"
#include "Components/TextBlock.h"

void UWTRElimAnnouncementWidget::SetElimAnnouncementText(const FString& AttackerName, const FString& VictimName) 
{
    const FString Text = FString::Printf(TEXT("%s elimmed %s"), *AttackerName, *VictimName);
    ElimText->SetText(FText::FromString(Text));
}
