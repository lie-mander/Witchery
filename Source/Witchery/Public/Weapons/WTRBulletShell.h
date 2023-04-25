// Witchery. Copyright Liemander. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WTRBulletShell.generated.h"

UCLASS()
class WITCHERY_API AWTRBulletShell : public AActor
{
    GENERATED_BODY()

public:
    AWTRBulletShell();

protected:
    UPROPERTY(VisibleAnywhere, Category = "Components")
    class UStaticMeshComponent* BulletMesh;

    virtual void BeginPlay() override;
};
