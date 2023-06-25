// Witchery. Copyright Liemander. All Rights Reserved.

#include "Components/WTRBuffComponent.h"

UWTRBuffComponent::UWTRBuffComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UWTRBuffComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UWTRBuffComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

