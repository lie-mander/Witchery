// Witchery. Copyright Liemander. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "WTRTypes.h"
#include "GameFramework/HUD.h"
#include "WTR_HUD.generated.h"

class UTexture2D;
class UWTRCharacterOverlayWidget;
class UWTRAnnouncementWidget;
class UWTRElimAnnouncementWidget;
class UUserWidget;

UCLASS()
class WITCHERY_API AWTR_HUD : public AHUD
{
    GENERATED_BODY()

public:
    virtual void DrawHUD() override;

    void AddCharacterOverlay();
    void AddAnnouncement();
    void AddElimAnnouncement(const FString& AttackerName, const FString& VictimName);

    FORCEINLINE void SetCrosshairHUDPackage(const FCrosshairHUDPackage& Package) { CrosshairHUDPackage = Package; }

    UPROPERTY(EditAnywhere, Category = "WTR | Widgets")
    TSubclassOf<UUserWidget> CharacterOverlayWidgetClass;

    UPROPERTY(EditAnywhere, Category = "WTR | Widgets")
    TSubclassOf<UUserWidget> AnnouncementWidgetClass;

    UPROPERTY(EditAnywhere, Category = "WTR | Widgets")
    TSubclassOf<UUserWidget> ElimAnnouncementWidgetClass;

    UPROPERTY()
    UWTRCharacterOverlayWidget* CharacterOverlayWidget;

    UPROPERTY()
    UWTRAnnouncementWidget* AnnouncementWidget;

    UPROPERTY()
    UWTRElimAnnouncementWidget* ElimAnnouncementWidget;

protected:
    virtual void BeginPlay() override;

private:
    UPROPERTY(EditDefaultsOnly, Category = "WTR | Crosshair")
    float CrosshairSpreadMultiplier = 16.f;

    UPROPERTY()
    APlayerController* OwnerController;

    FCrosshairHUDPackage CrosshairHUDPackage;

    void DrawCrosshair(UTexture2D* Texture, const FVector2D& ViewportCenter, const FVector2D& Spread, const FLinearColor& CrosshairColor);
};
