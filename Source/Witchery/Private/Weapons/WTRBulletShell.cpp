// Witchery. Copyright Liemander. All Rights Reserved.

#include "Weapons/WTRBulletShell.h"
#include "Components/StaticMeshComponent.h"

AWTRBulletShell::AWTRBulletShell()
{
	PrimaryActorTick.bCanEverTick = false;

	BulletMesh = CreateDefaultSubobject<UStaticMeshComponent>("BulletShellMesh");
}

void AWTRBulletShell::BeginPlay()
{
	Super::BeginPlay();
}
