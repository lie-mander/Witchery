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
    float HeadTime = FrameHistory.GetHead()->GetValue().Time;
    float TailTime = FrameHistory.GetTail()->GetValue().Time;
    return HeadTime - TailTime;
}

void UWTRLagCompensationComponent::SaveThisFrame()
{
    FFramePackage ThisFrame;
    SaveFramePackage(ThisFrame);
    FrameHistory.AddHead(ThisFrame);
}
