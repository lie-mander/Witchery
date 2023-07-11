// Witchery. Copyright Liemander. All Rights Reserved.

#include "Components/WTRLagCompensationComponent.h"
#include "Components/BoxComponent.h"
#include "Character/WTRCharacter.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "Weapons/WTRWeapon.h"

UWTRLagCompensationComponent::UWTRLagCompensationComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
}

void UWTRLagCompensationComponent::BeginPlay()
{
    Super::BeginPlay();
}

void UWTRLagCompensationComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    RecordFrameHistory();
}

void UWTRLagCompensationComponent::Server_ScoreRequest_Implementation(AWTRCharacter* HitCharacter, const FVector_NetQuantize& TraceStart,
    const FVector_NetQuantize& HitLocation, float HitTime, AWTRWeapon* DamageCauser)
{
    FServerSideRewindResult Confrim = ServerSideRewind(HitCharacter, TraceStart, HitLocation, HitTime);

    if (Confrim.bConfrimHit && DamageCauser && Character)
    {
        UGameplayStatics::ApplyDamage(  //
            HitCharacter,               //
            DamageCauser->GetDamage(),  //
            Character->Controller,      //
            DamageCauser,               //
            UDamageType::StaticClass()  //
        );
    }
}

FServerSideRewindResult UWTRLagCompensationComponent::ServerSideRewind(
    AWTRCharacter* HitCharacter, const FVector_NetQuantize& TraceStart, const FVector_NetQuantize& HitLocation, float HitTime)
{
    FFramePackage FrameToCheck = GetFrameToCheck(HitCharacter, HitTime);

    return ConfrimHit(FrameToCheck, HitCharacter, TraceStart, HitLocation);
}

FShotgunServerSideRewindResult UWTRLagCompensationComponent::ShotgunServerSideRewind(const TArray<AWTRCharacter*>& HitCharacters,
    const FVector_NetQuantize& TraceStart, const TArray<FVector_NetQuantize>& HitLocations, float HitTime)
{
    TArray<FFramePackage> FrameToChecks;

    for (AWTRCharacter* HitCharacter : HitCharacters)
    {
        FrameToChecks.Add(GetFrameToCheck(HitCharacter, HitTime));
    }

    return FShotgunServerSideRewindResult();
}

FServerSideRewindResult UWTRLagCompensationComponent::ConfrimHit(const FFramePackage& Package, AWTRCharacter* HitCharacter,
    const FVector_NetQuantize& TraceStart, const FVector_NetQuantize& HitLocation)
{
    if (!HitCharacter || !GetWorld()) return FServerSideRewindResult();

    /*
     * Cache current frame, cause after confirmed hit we need to return boxes to main position
     * Move boxes to interp package position
     * Disabled character mesh collision, cause it can block line trace and we want check only line trace with boxes
     */
    FFramePackage CurrentFrame;
    CacheFrame(CurrentFrame, HitCharacter);
    MoveBoxes(Package, HitCharacter);
    EnableCharacterMeshCollision(HitCharacter, ECollisionEnabled::NoCollision);

    // Default result
    FServerSideRewindResult ServerSideRewindResult;
    ServerSideRewindResult.bConfrimHit = false;
    ServerSideRewindResult.bHeadshot = false;

    // Firsty we will check headshot
    UBoxComponent* HeadBox = HitCharacter->HitBoxesMap[FName("head")];
    HeadBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    HeadBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);

    // HitResult for check blocking hit with boxes
    FHitResult RewindHitResult;

    // TraceEnd but we did it longer on 25% that line trace will go always through HitCharacter
    const FVector TraceEnd = TraceStart + (HitLocation - TraceStart) * 1.25f;

    GetWorld()->LineTraceSingleByChannel(  //
        RewindHitResult,                   //
        TraceStart,                        //
        TraceEnd,                          //
        ECollisionChannel::ECC_Visibility  //
    );

    if (RewindHitResult.bBlockingHit)
    {
        // We have headshot, can return early
        EnableCharacterMeshCollision(HitCharacter, ECollisionEnabled::QueryAndPhysics);
        ReturnBoxes(CurrentFrame, HitCharacter);

        ServerSideRewindResult.bConfrimHit = true;
        ServerSideRewindResult.bHeadshot = true;
        return ServerSideRewindResult;
    }
    else
    {
        // Do line trace for other boxes
        for (auto& HitBoxPair : HitCharacter->HitBoxesMap)
        {
            if (HitBoxPair.Value)
            {
                HitBoxPair.Value->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
                HitBoxPair.Value->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
            }
        }

        GetWorld()->LineTraceSingleByChannel(  //
            RewindHitResult,                   //
            TraceStart,                        //
            TraceEnd,                          //
            ECollisionChannel::ECC_Visibility  //
        );

        if (RewindHitResult.bBlockingHit)
        {
            // We have confirmed hit, but not headshot. Can return
            EnableCharacterMeshCollision(HitCharacter, ECollisionEnabled::QueryAndPhysics);
            ReturnBoxes(CurrentFrame, HitCharacter);

            ServerSideRewindResult.bConfrimHit = true;
            return ServerSideRewindResult;
        }
    }

    return ServerSideRewindResult;
}

FShotgunServerSideRewindResult UWTRLagCompensationComponent::ShorgunConfirmHits(
    const TArray<FFramePackage>& FramePackages, const FVector_NetQuantize& TraceStart, const TArray<FVector_NetQuantize>& HitLocations)
{
    return FShotgunServerSideRewindResult();
}

FFramePackage UWTRLagCompensationComponent::GetFrameToCheck(AWTRCharacter* HitCharacter, float HitTime) 
{
    // Firstly need to check pointest to things that we will use
    const bool bNeedReturn = !HitCharacter || !HitCharacter->GetLagCompensation() ||
                             !HitCharacter->GetLagCompensation()->FrameHistory.GetHead() ||
                             !HitCharacter->GetLagCompensation()->FrameHistory.GetTail();
    if (bNeedReturn) return FFramePackage();

    /*
     * 1. For shortly-using
     * 2. Variable to save final frame package
     * 3. In mostly situations - we need to use interpolate between 2 frame packages, but we have exceptions
     */
    const TDoubleLinkedList<FFramePackage>& History = HitCharacter->GetLagCompensation()->FrameHistory;
    FFramePackage FrameToCheck;
    bool bNeedInterpolate = true;

    // If true - character too lagging! Hit time so far to oldest record frame!
    // Need to break SSR (Server-side rewind)
    if (HitTime < History.GetTail()->GetValue().Time)
    {
        return FFramePackage();
    }

    // If true - character has the baddest allowable ping, but we can use package from history in this time
    if (HitTime == History.GetTail()->GetValue().Time)
    {
        bNeedInterpolate = false;
        FrameToCheck = History.GetTail()->GetValue();
    }

    // If true - character has perfect ping, we need to use newest frame package
    if (HitTime >= History.GetHead()->GetValue().Time)
    {
        bNeedInterpolate = false;
        FrameToCheck = History.GetHead()->GetValue();
    }

    /*
     * Two pointers for realise algorithm of finding two Frame Packages which are on two sides of HitTime
     *
     * Older < HitTime < Younger
     *
     * Firstly we need to init them with Head pointer and moving from Head to Tail
     * After complete finding we will use interpolation between Older/Younger values
     */
    TDoubleLinkedList<FFramePackage>::TDoubleLinkedListNode* Younger = History.GetHead();
    TDoubleLinkedList<FFramePackage>::TDoubleLinkedListNode* Older = Younger;

    while (Older->GetValue().Time > HitTime)
    {
        if (Older->GetNextNode())
        {
            // Move to tail if can (if GetNextNode() == nullptr - will be error)
            Older = Older->GetNextNode();
        }
        else
        {
            // We want to stop loop if we already on the Tail
            break;
        }

        if (Older->GetValue().Time > HitTime)
        {
            // If still greater than HitTime - we can move Younger pointer with Older
            Younger = Older;
        }
    }

    // Highly unlikely, but we found our frame to check
    if (Older->GetValue().Time == HitTime)
    {
        bNeedInterpolate = false;
        FrameToCheck = Older->GetValue();
    }

    // Interpolate between Older and Younger packages
    if (bNeedInterpolate)
    {
        FrameToCheck = InterpBetweenPackages(Older->GetValue(), Younger->GetValue(), HitTime);
    }

    return FrameToCheck;
}

void UWTRLagCompensationComponent::RecordFrameHistory()
{
    if (!Character || !Character->HasAuthority()) return;

    if (FrameHistory.Num() <= 1)
    {
        SaveThisFrame();
    }
    else
    {
        float HistoryLenghtInSec = TimeBetweenHeadAndTail();

        while (HistoryLenghtInSec > MaxRecordTime)
        {
            FrameHistory.RemoveNode(FrameHistory.GetTail());
            HistoryLenghtInSec = TimeBetweenHeadAndTail();
        }

        SaveThisFrame();
        // ShowFramePackage(FrameHistory.GetHead()->GetValue(), FColor::Orange);
    }
}

FFramePackage UWTRLagCompensationComponent::InterpBetweenPackages(
    const FFramePackage& OlderPackage, const FFramePackage& YoungerPackage, float HitTime)
{
    // Time between younger and older packages will be our distance to interpolation
    const float DistanceInSec = YoungerPackage.Time - OlderPackage.Time;
    // Part of the distance which we need to interp to
    const float InterpFraction = (HitTime - OlderPackage.Time) / DistanceInSec;

    // Result frame package
    FFramePackage InterpFramePackage;
    InterpFramePackage.Time = HitTime;

    /*
     * We`re going through Younger (or Older) pair to interp all boxes between OlderPackage and YoungerPackage
     * And save result in InterpFramePackage
     */
    for (auto& YoungerPair : YoungerPackage.FrameInfo)
    {
        const FName& BoxName = YoungerPair.Key;

        const FBoxInformation& YoungerBoxInfo = YoungerPackage.FrameInfo[BoxName];
        const FBoxInformation& OlderBoxInfo = OlderPackage.FrameInfo[BoxName];

        FBoxInformation InterpBoxInfo;

        // We don`t want to use DeltaTime, we want to do one interp in one time
        InterpBoxInfo.Location = FMath::VInterpTo(OlderBoxInfo.Location, YoungerBoxInfo.Location, 1.f, InterpFraction);
        InterpBoxInfo.Rotation = FMath::RInterpTo(OlderBoxInfo.Rotation, YoungerBoxInfo.Rotation, 1.f, InterpFraction);

        // BoxExtent for every box is unchanged, we can simple copy it from Younger (or Older) pair
        InterpBoxInfo.BoxExtent = YoungerBoxInfo.BoxExtent;

        // And save every interp box in resutl frame package
        InterpFramePackage.FrameInfo.Add(BoxName, InterpBoxInfo);
    }

    return InterpFramePackage;
}

void UWTRLagCompensationComponent::SaveFramePackage(FFramePackage& Package)
{
    Character = (Character == nullptr) ? Cast<AWTRCharacter>(GetOwner()) : Character;
    if (Character && GetWorld())
    {
        Package.Time = GetWorld()->GetTimeSeconds();
        Package.OwnerCharacter = Character;

        for (auto& HitBoxPair : Character->HitBoxesMap)
        {
            FBoxInformation BoxInfo;
            BoxInfo.Location = HitBoxPair.Value->GetComponentLocation();
            BoxInfo.Rotation = HitBoxPair.Value->GetComponentRotation();
            BoxInfo.BoxExtent = HitBoxPair.Value->GetScaledBoxExtent();

            Package.FrameInfo.Add(HitBoxPair.Key, BoxInfo);
        }
    }
}

void UWTRLagCompensationComponent::CacheFrame(FFramePackage& Package, AWTRCharacter* HitCharacter)
{
    if (!HitCharacter) return;

    for (auto& HitBoxPair : HitCharacter->HitBoxesMap)
    {
        if (HitBoxPair.Value)
        {
            FBoxInformation BoxInfo;
            BoxInfo.Location = HitBoxPair.Value->GetComponentLocation();
            BoxInfo.Rotation = HitBoxPair.Value->GetComponentRotation();
            BoxInfo.BoxExtent = HitBoxPair.Value->GetScaledBoxExtent();

            Package.FrameInfo.Add(HitBoxPair.Key, BoxInfo);
        }
    }
}

void UWTRLagCompensationComponent::MoveBoxes(const FFramePackage& Package, AWTRCharacter* HitCharacter)
{
    if (!HitCharacter) return;

    for (auto& HitBoxPair : HitCharacter->HitBoxesMap)
    {
        if (HitBoxPair.Value)
        {
            const FBoxInformation& BoxInfo = Package.FrameInfo[HitBoxPair.Key];

            HitBoxPair.Value->SetWorldLocation(BoxInfo.Location);
            HitBoxPair.Value->SetWorldRotation(BoxInfo.Rotation);
            HitBoxPair.Value->SetBoxExtent(BoxInfo.BoxExtent);
        }
    }
}

void UWTRLagCompensationComponent::ReturnBoxes(const FFramePackage& Package, AWTRCharacter* HitCharacter)
{
    if (!HitCharacter) return;

    for (auto& HitBoxPair : HitCharacter->HitBoxesMap)
    {
        if (HitBoxPair.Value)
        {
            const FBoxInformation& BoxInfo = Package.FrameInfo[HitBoxPair.Key];

            HitBoxPair.Value->SetWorldLocation(BoxInfo.Location);
            HitBoxPair.Value->SetWorldRotation(BoxInfo.Rotation);
            HitBoxPair.Value->SetBoxExtent(BoxInfo.BoxExtent);
            HitBoxPair.Value->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        }
    }
}

void UWTRLagCompensationComponent::EnableCharacterMeshCollision(AWTRCharacter* HitCharacter, ECollisionEnabled::Type CollisionEnabled)
{
    if (!HitCharacter || !HitCharacter->GetMesh()) return;

    HitCharacter->GetMesh()->SetCollisionEnabled(CollisionEnabled);
}

float UWTRLagCompensationComponent::TimeBetweenHeadAndTail()
{
    const float HeadTime = FrameHistory.GetHead()->GetValue().Time;
    const float TailTime = FrameHistory.GetTail()->GetValue().Time;
    return HeadTime - TailTime;
}

void UWTRLagCompensationComponent::SaveThisFrame()
{
    FFramePackage ThisFrame;
    SaveFramePackage(ThisFrame);
    FrameHistory.AddHead(ThisFrame);
}

void UWTRLagCompensationComponent::ShowFramePackage(const FFramePackage& Package, const FColor& Color)
{
    if (!GetWorld()) return;

    for (auto& FrameInfoPair : Package.FrameInfo)
    {
        DrawDebugBox(                             //
            GetWorld(),                           //
            FrameInfoPair.Value.Location,         //
            FrameInfoPair.Value.BoxExtent,        //
            FQuat(FrameInfoPair.Value.Rotation),  //
            Color,                                //
            false,                                //
            MaxRecordTime                         //
        );
    }
}
