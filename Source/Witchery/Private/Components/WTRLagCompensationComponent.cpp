// Witchery. Copyright Liemander. All Rights Reserved.

#include "Components/WTRLagCompensationComponent.h"
#include "Components/BoxComponent.h"
#include "Character/WTRCharacter.h"
#include "DrawDebugHelpers.h"

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

void UWTRLagCompensationComponent::ServerSideRewind(
    AWTRCharacter* HitCharacter, const FVector& TraceStart, const FVector& HitLocation, float HitTime)
{
    // Firstly need to check pointest to things that we will use
    const bool bNeedReturn = !HitCharacter || !HitCharacter->GetLagCompensation() ||
                             !HitCharacter->GetLagCompensation()->FrameHistory.GetHead() ||
                             !HitCharacter->GetLagCompensation()->FrameHistory.GetTail();
    if (bNeedReturn) return;

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
        return;
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

    }
}

void UWTRLagCompensationComponent::RecordFrameHistory()
{
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
        ShowFramePackage(FrameHistory.GetHead()->GetValue(), FColor::Orange);
    }
}

void UWTRLagCompensationComponent::SaveFramePackage(FFramePackage& Package)
{
    Character = (Character == nullptr) ? Cast<AWTRCharacter>(GetOwner()) : Character;
    if (Character && GetWorld())
    {
        Package.Time = GetWorld()->GetTimeSeconds();

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
