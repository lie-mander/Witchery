// Witchery. Copyright Liemander. All Rights Reserved.

#include "Weapons/WTRProjectileBullet.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "GameFramework/ProjectileMovementComponent.h"

AWTRProjectileBullet::AWTRProjectileBullet() 
{
    ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovementComponent"));
    ProjectileMovementComponent->bRotationFollowsVelocity = true;
    ProjectileMovementComponent->SetIsReplicated(true);
    ProjectileMovementComponent->InitialSpeed = InitialSpeed;
    ProjectileMovementComponent->MaxSpeed = InitialSpeed;
}

void AWTRProjectileBullet::OnHit(
    UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
    const ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner());
    if (OwnerCharacter)
    {
        AController* OwnerController = Cast<AController>(OwnerCharacter->Controller);
        if (OwnerController)
        {
            UGameplayStatics::ApplyDamage(OtherActor, Damage, OwnerController, this, UDamageType::StaticClass());
        }
    }

    Super::OnHit(HitComponent, OtherActor, OtherComp, NormalImpulse, Hit);
}

void AWTRProjectileBullet::BeginPlay() 
{
    Super::BeginPlay();

    FPredictProjectilePathParams PathParams;
    FPredictProjectilePathResult PathResult;

    PathParams.ActorsToIgnore.Add(this);
    PathParams.bTraceWithChannel = true;
    PathParams.bTraceWithCollision = true;
    PathParams.DrawDebugTime = 5.f;
    PathParams.DrawDebugType = EDrawDebugTrace::ForDuration;
    PathParams.LaunchVelocity = GetActorForwardVector() * InitialSpeed;
    PathParams.MaxSimTime = 5.f;
    PathParams.ProjectileRadius = 5.f;
    PathParams.SimFrequency = 30.f;
    PathParams.StartLocation = GetActorLocation();
    PathParams.TraceChannel = ECollisionChannel::ECC_Visibility;

    UGameplayStatics::PredictProjectilePath(this, PathParams, PathResult);
}
