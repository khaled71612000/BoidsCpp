#include "BoidRing.h"
#include "Boid.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"

ABoidRing::ABoidRing()
{
	PrimaryActorTick.bCanEverTick = true;

	DetectionSphere = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Detection Sphere"));
	RootComponent = DetectionSphere;
	DetectionSphere->SetCollisionProfileName(TEXT("Trigger"));
	DetectionSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	DetectionSphere->SetCollisionResponseToAllChannels(ECR_Overlap);
}

void ABoidRing::BeginPlay()
{
	Super::BeginPlay();
}

void ABoidRing::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	InfluenceBoids();
}

void ABoidRing::InfluenceBoids()
{
    TArray<AActor*> overlappingActors;
    DetectionSphere->GetOverlappingActors(overlappingActors, ABoid::StaticClass());

    if (overlappingActors.Num() == 0)
    {
        return;
    }

    const FVector ringLocation = GetActorLocation();
    const float radius = DetectionSphere->Bounds.SphereRadius;
    if (radius <= KINDA_SMALL_NUMBER)
    {
        return;
    }

    const float clampedInnerFraction = FMath::Clamp(InnerRadiusFrac, 0.f, 1.f);
    const float innerRadius = radius * clampedInnerFraction;
    const FVector ringForward = GetActorForwardVector().GetSafeNormal();

    for (AActor* actor : overlappingActors)
    {
        ABoid* boid = Cast<ABoid>(actor);
        if (!boid)
        {
            continue;
        }

        const FVector boidLocation = boid->GetActorLocation();
        const FVector toRing = ringLocation - boidLocation;
        const float   dist = toRing.Size();

        if (dist <= KINDA_SMALL_NUMBER)
        {
            continue;
        }

        const FVector dirToRing = toRing / dist;
        // weight - normalized dist
        const float weight = 1.0f - FMath::Clamp(dist / radius, 0.0f, 1.0f);
        if (bGoThroughRing && dist < innerRadius)
        {
            FVector launchDir = ringForward;
            if (launchDir.IsNearlyZero())
            {
                launchDir = -dirToRing;
            }

            const FVector launchVelocity = launchDir.GetSafeNormal() * LaunchSpeed;
            boid->RequestLaunch(launchVelocity);
            continue;
        }

        FVector influenceForce = FVector::ZeroVector;
        if (bGoThroughRing)
        {
            influenceForce = dirToRing * boid->GetMaxAlign() * weight;
        }
        else
        {
            FVector tangentRound = FVector::CrossProduct(dirToRing, FVector::UpVector);
            if (tangentRound.IsNearlyZero())
            {
                tangentRound = FVector::CrossProduct(dirToRing, FVector::RightVector);
            }
            tangentRound.Normalize();

            influenceForce = tangentRound * boid->GetMaxAlign() * weight;
        }

        boid->AddForce(influenceForce);
    }
}
