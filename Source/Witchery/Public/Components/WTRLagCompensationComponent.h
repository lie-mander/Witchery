// Witchery. Copyright Liemander. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "WTRTypes.h"
#include "WTRLagCompensationComponent.generated.h"

class AWTRCharacter;
class AWTRPlayerController;
class AWTRWeapon;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class WITCHERY_API UWTRLagCompensationComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    friend class AWTRCharacter;

    UWTRLagCompensationComponent();
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UFUNCTION(Server, Reliable)
    void Server_ScoreRequest(AWTRCharacter* HitCharacter, const FVector_NetQuantize& TraceStart, const FVector_NetQuantize& HitLocation,
        float HitTime, AWTRWeapon* DamageCauser);

protected:
    virtual void BeginPlay() override;

    void SaveFramePackage(FFramePackage& Package);
    void ShowFramePackage(const FFramePackage& Package, const FColor& Color);
    FFramePackage InterpBetweenPackages(const FFramePackage& OlderPackage, const FFramePackage& YoungerPackage, float HitTime);
    FServerSideRewindResult ConfrimHit(const FFramePackage& Package, AWTRCharacter* HitCharacter, const FVector_NetQuantize& TraceStart,
        const FVector_NetQuantize& HitLocation);
    void CacheFrame(FFramePackage& Package, AWTRCharacter* HitCharacter);
    void MoveBoxes(const FFramePackage& Package, AWTRCharacter* HitCharacter);
    void ReturnBoxes(const FFramePackage& Package, AWTRCharacter* HitCharacter);
    void EnableCharacterMeshCollision(AWTRCharacter* HitCharacter, ECollisionEnabled::Type CollisionEnabled);

    // Main function for server-side rewind
    FServerSideRewindResult ServerSideRewind(
        AWTRCharacter* HitCharacter, const FVector_NetQuantize& TraceStart, const FVector_NetQuantize& HitLocation, float HitTime);

private:
    UPROPERTY()
    AWTRCharacter* Character;

    UPROPERTY()
    AWTRPlayerController* Controller;

    UPROPERTY(EditDefaultsOnly, Category = "WTR | RewindTime")
    float MaxRecordTime = 4.f;

    TDoubleLinkedList<FFramePackage> FrameHistory;

    void RecordFrameHistory();
    void SaveThisFrame();
    float TimeBetweenHeadAndTail();
};
