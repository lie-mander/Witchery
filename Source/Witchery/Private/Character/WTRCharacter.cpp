// Witchery. Copyright Liemander. All Rights Reserved.

#include "Character/WTRCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/WidgetComponent.h"

AWTRCharacter::AWTRCharacter()
{
    PrimaryActorTick.bCanEverTick = true;

    SpringArmComponent = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArmComponent"));
    SpringArmComponent->SetupAttachment(GetMesh());
    SpringArmComponent->TargetArmLength = 600.f;
    SpringArmComponent->bUsePawnControlRotation = true;

    CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComponent"));
    CameraComponent->SetupAttachment(SpringArmComponent, USpringArmComponent::SocketName);
    CameraComponent->bUsePawnControlRotation = true;

    bUseControllerRotationYaw = false;
    GetCharacterMovement()->bOrientRotationToMovement = true;

    OverheadWidget = CreateDefaultSubobject<UWidgetComponent>("OverheadWidget");
    OverheadWidget->SetupAttachment(RootComponent);
}

void AWTRCharacter::BeginPlay()
{
    Super::BeginPlay();
}

void AWTRCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void AWTRCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    PlayerInputComponent->BindAction("Jump", EInputEvent::IE_Pressed, this, &ACharacter::Jump);

    PlayerInputComponent->BindAxis(FName("MoveForward"), this, &ThisClass::MoveForward);
    PlayerInputComponent->BindAxis(FName("MoveRight"), this, &ThisClass::MoveRight);
    PlayerInputComponent->BindAxis(FName("Turn"), this, &ThisClass::Turn);
    PlayerInputComponent->BindAxis(FName("LookUp"), this, &ThisClass::LookUp);
}

void AWTRCharacter::MoveForward(float Amount) 
{
    if (Controller && Amount != 0.f)
    {
        const FRotator YawRotation(0.f, Controller->GetControlRotation().Yaw, 0.f);
        const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
        AddMovementInput(Direction, Amount);
    }
}

void AWTRCharacter::MoveRight(float Amount) 
{
    if (Controller && Amount != 0.f)
    {
        const FRotator YawRotation(0.f, Controller->GetControlRotation().Yaw, 0.f);
        const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
        AddMovementInput(Direction, Amount);
    }
}

void AWTRCharacter::Turn(float Amount) 
{
    AddControllerYawInput(Amount);
}

void AWTRCharacter::LookUp(float Amount) 
{
    AddControllerPitchInput(Amount);
}
