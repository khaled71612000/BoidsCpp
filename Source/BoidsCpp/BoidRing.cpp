#include "BoidRing.h"
#include "Components/SphereComponent.h"
#include "Boid.h"
#include "Engine/World.h"

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

    for (AActor* Actor : overlappingActors)
    {
        ABoid* boid = Cast<ABoid>(Actor);
        if (!boid)
        {
            continue;
        }

        const FVector boidLocation = boid->GetActorLocation();
        FVector toRing = ringLocation - boidLocation;
        const float dist = toRing.Size();

        if (dist <= KINDA_SMALL_NUMBER || radius <= KINDA_SMALL_NUMBER)
        {
            continue;
        }

        const FVector DirToRing = toRing / dist;
        const float weight = 1.0f - FMath::Clamp(dist / radius, 0.0f, 1.0f);
        FVector influenceForce = FVector::ZeroVector;

        if (bGoThroughRing)
        {
            influenceForce = DirToRing * boid->GetMaxAlign() * weight;
        }
        else
        {
            FVector tangentRound = FVector::CrossProduct(DirToRing, FVector::UpVector);
            if (tangentRound.IsNearlyZero())
            {
                tangentRound = FVector::CrossProduct(DirToRing, FVector::RightVector);
            }
            tangentRound.Normalize();

            influenceForce = tangentRound * boid->GetMaxAlign() * weight;
        }

        boid->AddForce(influenceForce);
    }
}
