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

    UFUNCTION(Server, Reliable)
    void Server_ProjectileScoreRequest(
        AWTRCharacter* HitCharacter, const FVector_NetQuantize& TraceStart, const FVector_NetQuantize100& LaunchVelocity, float HitTime);

    UFUNCTION(Server, Reliable)
    void Server_ShotgunScoreRequest(const TArray<AWTRCharacter*>& HitCharacters, const FVector_NetQuantize& TraceStart,
        const TArray<FVector_NetQuantize>& HitLocations, float HitTime);

protected:
    virtual void BeginPlay() override;

    void SaveFramePackage(FFramePackage& Package);
    void ShowFramePackage(const FFramePackage& Package, const FColor& Color);
    FFramePackage InterpBetweenPackages(const FFramePackage& OlderPackage, const FFramePackage& YoungerPackage, float HitTime);
    void CacheFrame(FFramePackage& Package, AWTRCharacter* HitCharacter);
    void MoveBoxes(const FFramePackage& Package, AWTRCharacter* HitCharacter);
    void ReturnBoxes(const FFramePackage& Package, AWTRCharacter* HitCharacter);
    void EnableCharacterMeshCollision(AWTRCharacter* HitCharacter, ECollisionEnabled::Type CollisionEnabled);
    FFramePackage GetFrameToCheck(AWTRCharacter* HitCharacter, float HitTime);

    /*
     * HitScan
     */
    FServerSideRewindResult ServerSideRewind(
        AWTRCharacter* HitCharacter, const FVector_NetQuantize& TraceStart, const FVector_NetQuantize& HitLocation, float HitTime);

    FServerSideRewindResult ConfrimHit(const FFramePackage& Package, AWTRCharacter* HitCharacter, const FVector_NetQuantize& TraceStart,
        const FVector_NetQuantize& HitLocation);

    /*
     * Projectile
     */
    FServerSideRewindResult ProjectileServerSideRewind(
        AWTRCharacter* HitCharacter, const FVector_NetQuantize& TraceStart, const FVector_NetQuantize100& LaunchVelocity, float HitTime);

    FServerSideRewindResult ProjectileConfirmHit(const FFramePackage& Package, AWTRCharacter* HitCharacter,
        const FVector_NetQuantize& TraceStart, const FVector_NetQuantize100& LaunchVelocity);

    /*
     * Shotgun
     */
    FShotgunServerSideRewindResult ShotgunServerSideRewind(const TArray<AWTRCharacter*>& HitCharacters,
        const FVector_NetQuantize& TraceStart, const TArray<FVector_NetQuantize>& HitLocations, float HitTime);

    FShotgunServerSideRewindResult ShorgunConfirmHits(
        const TArray<FFramePackage>& FramePackages, const FVector_NetQuantize& TraceStart, const TArray<FVector_NetQuantize>& HitLocations);

private:
    UPROPERTY()
    AWTRCharacter* Character;

    UPROPERTY()
    AWTRPlayerController* Controller;

    UPROPERTY(EditDefaultsOnly, Category = "WTR | RewindTime")
    float MaxRecordTime = 4.f;

    UPROPERTY(EditDefaultsOnly, Category = "WTR | RewindTime")
    bool bDebugLagCompensation = false;

    UPROPERTY(EditDefaultsOnly, Category = "WTR | RewindTime")
    bool bDebugRecord = false;

    TDoubleLinkedList<FFramePackage> FrameHistory;

    void RecordFrameHistory();
    void SaveThisFrame();
    float TimeBetweenHeadAndTail();
    void DrawHitBoxComponent(const FHitResult& HitResult, FColor Color);
};
