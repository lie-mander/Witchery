// Witchery. Copyright Liemander. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "WTRTypes.h"
#include "GameFramework/HUD.h"
#include "WTR_HUD.generated.h"

class UTexture2D;
class UWTRCharacterOverlayWidget;
class UUserWidget;

UCLASS()
class WITCHERY_API AWTR_HUD : public AHUD
{
    GENERATED_BODY()

public:
    virtual void DrawHUD() override;

    void AddCharacterOverlay();

    FORCEINLINE void SetCrosshairHUDPackage(const FCrosshairHUDPackage& Package) { CrosshairHUDPackage = Package; }

    UPROPERTY(EditAnywhere)
    TSubclassOf<UUserWidget> CharacterOverlayWidgetClass;

    UPROPERTY()
    UWTRCharacterOverlayWidget* CharacterOverlayWidget;

protected:
    virtual void BeginPlay() override;

private:
    UPROPERTY(EditDefaultsOnly, Category = "Crosshair")
    float CrosshairSpreadMultiplier = 16.f;

    FCrosshairHUDPackage CrosshairHUDPackage;

    void DrawCrosshair(UTexture2D* Texture, const FVector2D& ViewportCenter, const FVector2D& Spread, const FLinearColor& CrosshairColor);
};
