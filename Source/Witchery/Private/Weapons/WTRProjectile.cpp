// Witchery. Copyright Liemander. All Rights Reserved.

#include "Weapons/WTRProjectile.h"
#include "Components/BoxComponent.h"

AWTRProjectile::AWTRProjectile()
{
	PrimaryActorTick.bCanEverTick = true;

	BoxCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxCollision"));
    SetRootComponent(BoxCollision);
    BoxCollision->SetCollisionObjectType(ECC_WorldDynamic);
    BoxCollision->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    BoxCollision->SetCollisionResponseToAllChannels(ECR_Ignore);
    BoxCollision->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
    BoxCollision->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
}

void AWTRProjectile::BeginPlay()
{
	Super::BeginPlay();
}

void AWTRProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

