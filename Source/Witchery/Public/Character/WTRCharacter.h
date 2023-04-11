// Witchery. Copyright Liemander. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "WTRCharacter.generated.h"

UCLASS()
class WITCHERY_API AWTRCharacter : public ACharacter
{
    GENERATED_BODY()

public:
    AWTRCharacter();
    virtual void Tick(float DeltaTime) override;
    virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

protected:
    void MoveForward(float Amount);
    void MoveRight(float Amount);
    void Turn(float Amount);
    void LookUp(float Amount);

    virtual void BeginPlay() override;

private:
    UPROPERTY(VisibleAnywhere, Category = "Camera")
    class USpringArmComponent* SpringArmComponent;

    UPROPERTY(VisibleAnywhere, Category = "Camera")
    class UCameraComponent* CameraComponent;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
    class UWidgetComponent* OverheadWidget;
};
