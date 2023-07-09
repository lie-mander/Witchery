// Witchery. Copyright Liemander. All Rights Reserved.

#include "Components/WTRLagCompensationComponent.h"

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
}
