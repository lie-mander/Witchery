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

        if (CrosshairHUDPackage.CrosshairsCenter)
        {
            DrawCrosshair(CrosshairHUDPackage.CrosshairsCenter, ViewportCenter);
        }

        if (CrosshairHUDPackage.CrosshairsLeft)
        {
            DrawCrosshair(CrosshairHUDPackage.CrosshairsLeft, ViewportCenter);
        }

        if (CrosshairHUDPackage.CrosshairsRight)
        {
            DrawCrosshair(CrosshairHUDPackage.CrosshairsRight, ViewportCenter);
        }

        if (CrosshairHUDPackage.CrosshairsTop)
        {
            DrawCrosshair(CrosshairHUDPackage.CrosshairsTop, ViewportCenter);
        }

        if (CrosshairHUDPackage.CrosshairsBottom)
        {
            DrawCrosshair(CrosshairHUDPackage.CrosshairsBottom, ViewportCenter);
        }
    }
}

void AWTR_HUD::DrawCrosshair(UTexture2D* Texture, const FVector2D& ViewportCenter)
{
    if (!Texture) return;
    UE_LOG(LogTemp, Warning, TEXT("DrawCrosshair"));
    const float TextureWidht = Texture->GetSizeX();
    const float TextureHeight = Texture->GetSizeY();

    const FVector2D DrawPoint(                    //
        ViewportCenter.X - (TextureWidht / 2.f),  //
        ViewportCenter.Y - (TextureHeight / 2.f)  //
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
        1.f             //
    );
}
