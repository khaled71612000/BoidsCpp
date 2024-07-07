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

	ProbabilityToGoThrough = 0.5f;
	InfluenceDuration = 2.0f;
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
	TArray<AActor*> OverlappingActors;
	DetectionSphere->GetOverlappingActors(OverlappingActors, ABoid::StaticClass());

	for (AActor* Actor : OverlappingActors)
	{
		ABoid* Boid = Cast<ABoid>(Actor);
		if (Boid)
		{
			// Determine if the boid should go through the ring or around it
			float RandomValue = FMath::FRand();
			if (RandomValue <= ProbabilityToGoThrough && !Boid->bIsInfluencedByRing)
			{
				// Go through the ring
				FVector DirectionToRing = GetActorLocation() - Boid->GetActorLocation();
				DirectionToRing.Normalize();

				FVector InfluenceForce = DirectionToRing * Boid->GetMaxAlign();
				Boid->AddForce(InfluenceForce);
			}
			else
			{
				// Go around the ring
				FVector DirectionToRing = GetActorLocation() - Boid->GetActorLocation();
				DirectionToRing.Normalize();

				FVector TangentialDirection = FVector::CrossProduct(DirectionToRing, FVector::UpVector);
				TangentialDirection.Normalize();

				FVector InfluenceForce = TangentialDirection * Boid->GetMaxAlign();
				Boid->AddForce(InfluenceForce);
			}

			Boid->bIsInfluencedByRing = true;
			Boid->GetWorldTimerManager().SetTimer(Boid->InfluenceTimerHandle, Boid, &ABoid::ResetInfluenceState, InfluenceDuration, false);
		}
	}
}
