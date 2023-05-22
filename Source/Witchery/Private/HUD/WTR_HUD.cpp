// Witchery. Copyright Liemander. All Rights Reserved.

#include "HUD/WTR_HUD.h"

void AWTR_HUD::DrawHUD()
{
    Super::DrawHUD();

    if (GEngine)
    {
        FVector2D ViewportSize;
        GEngine->GameViewport.Get()->GetViewportSize(ViewportSize);
        FVector2D ViewportCenter(ViewportSize.X / 2.f, ViewportSize.Y / 2.f);

        float SpreadMultiplier = CrosshairSpreadMultiplier * CrosshairHUDPackage.CrosshairSpread;

        if (CrosshairHUDPackage.CrosshairsCenter)
        {
            FVector2D Spread(FVector2D::Zero());
            DrawCrosshair(CrosshairHUDPackage.CrosshairsCenter, ViewportCenter, Spread, CrosshairHUDPackage.CrosshairColor);
        }

        if (CrosshairHUDPackage.CrosshairsLeft)
        {
            FVector2D Spread(-SpreadMultiplier, 0.f);
            DrawCrosshair(CrosshairHUDPackage.CrosshairsLeft, ViewportCenter, Spread, CrosshairHUDPackage.CrosshairColor);
        }

        if (CrosshairHUDPackage.CrosshairsRight)
        {
            FVector2D Spread(SpreadMultiplier, 0.f);
            DrawCrosshair(CrosshairHUDPackage.CrosshairsRight, ViewportCenter, Spread, CrosshairHUDPackage.CrosshairColor);
        }

        if (CrosshairHUDPackage.CrosshairsTop)
        {
            FVector2D Spread(0.f, -SpreadMultiplier);
            DrawCrosshair(CrosshairHUDPackage.CrosshairsTop, ViewportCenter, Spread, CrosshairHUDPackage.CrosshairColor);
        }

        if (CrosshairHUDPackage.CrosshairsBottom)
        {
            FVector2D Spread(0.f, SpreadMultiplier);
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
