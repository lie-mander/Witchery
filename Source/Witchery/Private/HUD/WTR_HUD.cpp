// Witchery. Copyright Liemander. All Rights Reserved.

#include "HUD/WTR_HUD.h"
#include "HUD/WTRCharacterOverlayWidget.h"
#include "HUD/WTRAnnouncementWidget.h"
#include "HUD/WTRElimAnnouncementWidget.h"
#include "GameFramework/PlayerController.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Components/HorizontalBox.h"
#include "Components/CanvasPanelSlot.h"

void AWTR_HUD::BeginPlay()
{
    Super::BeginPlay();
}

void AWTR_HUD::AddCharacterOverlay()
{
    OwnerController = (OwnerController == nullptr) ? GetOwningPlayerController() : OwnerController;

    if (OwnerController && CharacterOverlayWidgetClass)
    {
        CharacterOverlayWidget = CreateWidget<UWTRCharacterOverlayWidget>(OwnerController, CharacterOverlayWidgetClass);
        CharacterOverlayWidget->AddToViewport();
    }
}

void AWTR_HUD::AddAnnouncement()
{
    OwnerController = (OwnerController == nullptr) ? GetOwningPlayerController() : OwnerController;

    if (OwnerController && AnnouncementWidgetClass)
    {
        AnnouncementWidget = CreateWidget<UWTRAnnouncementWidget>(OwnerController, AnnouncementWidgetClass);
        AnnouncementWidget->AddToViewport();
    }
}

void AWTR_HUD::AddElimAnnouncement(const FString& AttackerName, const FString& VictimName)
{
    OwnerController = (OwnerController == nullptr) ? GetOwningPlayerController() : OwnerController;
    if (OwnerController && ElimAnnouncementWidgetClass)
    {
        ElimAnnouncementWidget = CreateWidget<UWTRElimAnnouncementWidget>(OwnerController, ElimAnnouncementWidgetClass);
        ElimAnnouncementWidget->SetElimAnnouncementText(AttackerName, VictimName);
        ElimAnnouncementWidget->AddToViewport();

        if (!ElimMessages.IsEmpty())
        {
            for (UWTRElimAnnouncementWidget* ElimMessage : ElimMessages)
            {
                if (ElimMessage && ElimMessage->ElimHorizontalBox)
                {
                    UCanvasPanelSlot* CanvasPanelSlot = UWidgetLayoutLibrary::SlotAsCanvasSlot(ElimMessage->ElimHorizontalBox);
                    if (CanvasPanelSlot)
                    {
                        const FVector2D Position = CanvasPanelSlot->GetPosition();
                        const FVector2D NewPosition = FVector2D(                             //
                            CanvasPanelSlot->GetPosition().X,                                //
                            CanvasPanelSlot->GetPosition().Y + CanvasPanelSlot->GetSize().Y  //
                        );
                        CanvasPanelSlot->SetPosition(NewPosition);
                    }
                }
            }
        }

        ElimMessages.Add(ElimAnnouncementWidget);

        if (ElimAnnouncementWidget)
        {
            // Timer for remove from viewport
            FTimerHandle ElimMessageHandle;
            FTimerDelegate ElimMessageDelegate;
            ElimMessageDelegate.BindUFunction(this, FName("OnElimMessageTimerFinished"), ElimAnnouncementWidget);
            GetWorldTimerManager().SetTimer(  //
                ElimMessageHandle,            //
                ElimMessageDelegate,          //
                ElimMessageTime,              //
                false                         //
            );
        }
    }
}

void AWTR_HUD::OnElimMessageTimerFinished(UWTRElimAnnouncementWidget* MessageToDelete)
{
    if (MessageToDelete)
    {
        ElimMessages.Remove(MessageToDelete);
        MessageToDelete->RemoveFromParent();
    }
}

void AWTR_HUD::DrawHUD()
{
    Super::DrawHUD();

    if (GEngine)
    {
        FVector2D ViewportSize;
        GEngine->GameViewport.Get()->GetViewportSize(ViewportSize);
        const FVector2D ViewportCenter(ViewportSize.X / 2.f, ViewportSize.Y / 2.f);

        float SpreadMultiplier = CrosshairSpreadMultiplier * CrosshairHUDPackage.CrosshairSpread;

        if (CrosshairHUDPackage.CrosshairsCenter)
        {
            const FVector2D Spread(FVector2D::Zero());
            DrawCrosshair(CrosshairHUDPackage.CrosshairsCenter, ViewportCenter, Spread, CrosshairHUDPackage.CrosshairColor);
        }

        if (CrosshairHUDPackage.CrosshairsLeft)
        {
            const FVector2D Spread(-SpreadMultiplier, 0.f);
            DrawCrosshair(CrosshairHUDPackage.CrosshairsLeft, ViewportCenter, Spread, CrosshairHUDPackage.CrosshairColor);
        }

        if (CrosshairHUDPackage.CrosshairsRight)
        {
            const FVector2D Spread(SpreadMultiplier, 0.f);
            DrawCrosshair(CrosshairHUDPackage.CrosshairsRight, ViewportCenter, Spread, CrosshairHUDPackage.CrosshairColor);
        }

        if (CrosshairHUDPackage.CrosshairsTop)
        {
            const FVector2D Spread(0.f, -SpreadMultiplier);
            DrawCrosshair(CrosshairHUDPackage.CrosshairsTop, ViewportCenter, Spread, CrosshairHUDPackage.CrosshairColor);
        }

        if (CrosshairHUDPackage.CrosshairsBottom)
        {
            const FVector2D Spread(0.f, SpreadMultiplier);
            DrawCrosshair(CrosshairHUDPackage.CrosshairsBottom, ViewportCenter, Spread, CrosshairHUDPackage.CrosshairColor);
        }
    }
}

void AWTR_HUD::DrawCrosshair(
    UTexture2D* Texture, const FVector2D& ViewportCenter, const FVector2D& Spread, const FLinearColor& CrosshairColor)
{
    if (!Texture) return;

    const float TextureWidht = Texture->GetSizeX();
    const float TextureHeight = Texture->GetSizeY();

    const FVector2D DrawPoint(                               //
        ViewportCenter.X - (TextureWidht / 2.f) + Spread.X,  //
        ViewportCenter.Y - (TextureHeight / 2.f) + Spread.Y  //
    );

    DrawTexture(        //
        Texture,        //
        DrawPoint.X,    //
        DrawPoint.Y,    //
        TextureWidht,   //
        TextureHeight,  //
        0.f,            //
        0.f,            //
        1.f,            //
        1.f,            //
        CrosshairColor  //
    );
}
