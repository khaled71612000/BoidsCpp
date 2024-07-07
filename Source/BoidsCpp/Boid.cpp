#include "Boid.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "FlockSubsystem.h"
#include "Components/BoxComponent.h"
#include "BoidVolumeSpawner.h"

ABoid::ABoid()
{
	PrimaryActorTick.bCanEverTick = true;

	BoidSkeletalMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Boid Mesh Component"));
	RootComponent = BoidSkeletalMesh;
	BoidSkeletalMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	BoidSkeletalMesh->SetCollisionResponseToAllChannels(ECR_Ignore);

	BoidStaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Boid Static Mesh"));
	BoidStaticMesh->SetupAttachment(RootComponent);
	BoidStaticMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	BoidStaticMesh->SetCollisionResponseToAllChannels(ECR_Ignore);

	DetectionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("Detection Sphere"));
	DetectionSphere->SetupAttachment(RootComponent);
	DetectionSphere->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	DetectionSphere->SetCollisionResponseToAllChannels(ECR_Block);

	BoidVelocity = FVector::OneVector;
	BoidAcceleration = FVector::ZeroVector;
	MinAlignForce = -1;
	MaxAlignForce = 1;

	bIsInfluencedByRing = false;
}

void ABoid::SetPercipRadius(int32 rad)
{
	DetectionSphere->SetSphereRadius(rad);
}

void ABoid::ResetInfluenceState()
{
	bIsInfluencedByRing = false;
	GetWorldTimerManager().ClearTimer(InfluenceTimerHandle);
}

void ABoid::AddForce(const FVector& Force)
{
	BoidAcceleration += Force;

	// Update the velocity
	BoidVelocity += BoidAcceleration;
	BoidVelocity = BoidVelocity.GetClampedToSize(MinAlignForce, MaxAlignForce);

	// Apply the movement
	SetActorLocation(GetActorLocation() + BoidVelocity);

	// Reset acceleration after applying the force, you dont want them to get faster each time they go through the ring
	BoidAcceleration = FVector::ZeroVector;
}

void ABoid::BeginPlay()
{
	Super::BeginPlay();

	FlockSubsystem = GetWorld()->GetSubsystem<UFlockSubsystem>();
	BoidVelocity = FMath::VRand();
	BoidVelocity *= FMath::RandRange(MaxAlignForce, MinAlignForce);
}

void ABoid::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	Flock(DeltaTime);
	CheckBounds();
	UpdateRotation(DeltaTime);
}

void ABoid::Flock(float DeltaTime)
{
	BoidAcceleration = FVector::ZeroVector;

	TArray<AActor*> OverlappingActors;
	DetectionSphere->GetOverlappingActors(OverlappingActors, ABoid::StaticClass());

	if (!bDisableSeparation)
	{
		BoidAcceleration += Separation(OverlappingActors);
	}
	if (!bDisableAlign)
	{
		BoidAcceleration += Align(OverlappingActors);
	}
	if (!bDisableCohesion)
	{
		BoidAcceleration += Cohesion(OverlappingActors);
	}

	BoidVelocity += BoidAcceleration;
	BoidVelocity = BoidVelocity.GetClampedToSize(MinAlignForce, MaxAlignForce);
	if (bDisableZ)
	{
		BoidVelocity.Z = 0;
	}

	SetActorLocation(GetActorLocation() + BoidVelocity);}

void ABoid::CheckBounds()
{
	if (SpawnVolume)
	{
		// Get the origin (center) and the extents (half-dimensions) of the SpawnVolume
		FVector Location = GetActorLocation();
		FVector VolumeOrigin = SpawnVolume->GetSpawnVolume()->Bounds.Origin;
		FVector VolumeExtent = SpawnVolume->GetSpawnVolume()->Bounds.BoxExtent;

		bool bIsOutside = false;

		// Check if the Boid's location is within the bounds of the SpawnVolume
		if (!FMath::IsWithin(Location.X, VolumeOrigin.X - VolumeExtent.X, VolumeOrigin.X + VolumeExtent.X))
		{
			BoidVelocity.X *= -1;
			bIsOutside = true;
		}

		if (!FMath::IsWithin(Location.Y, VolumeOrigin.Y - VolumeExtent.Y, VolumeOrigin.Y + VolumeExtent.Y))
		{
			BoidVelocity.Y *= -1;
			bIsOutside = true;
		}

		if (!bDisableZ && !FMath::IsWithin(Location.Z, VolumeOrigin.Z - VolumeExtent.Z, VolumeOrigin.Z + VolumeExtent.Z))
		{
			BoidVelocity.Z *= -1;
			bIsOutside = true;
		}

		// If the boid is outside, nudge it slightly towards the center to prevent getting stuck
		if (bIsOutside)
		{
			FVector DirectionToCenter = (VolumeOrigin - Location).GetSafeNormal();
			SetActorLocation(Location + DirectionToCenter * MinAlignForce); // Nudge distance can be adjusted
		}
	}
}

void ABoid::UpdateRotation(float DeltaTime)
{
	if (BoidVelocity.SizeSquared() > KINDA_SMALL_NUMBER)
	{
		FRotator CurrentRotation = GetActorRotation();
		FRotator TargetRotation = BoidVelocity.Rotation();
		float RotationSpeed = 5.0f; // Adjust this value to control the rotation speed
		FRotator NewRotation = FMath::RInterpTo(CurrentRotation, TargetRotation, DeltaTime, RotationSpeed);
		SetActorRotation(NewRotation);
	}
}

FVector ABoid::Separation(TArray<AActor*> boids)
{
	FVector steeringForce = FVector::ZeroVector;
	int32 total = 0;

	for (AActor* actor : boids)
	{
		if (ABoid* boid = Cast<ABoid>(actor))
		{
			if (boid != this)
			{
				FVector difference = GetActorLocation() - boid->GetActorLocation();
				float distance = difference.Size();

				if (distance < KINDA_SMALL_NUMBER) 
				{
					continue;
				}

				// Directly weight by inverse of distance
				FVector weightedDifference = difference / distance;
				steeringForce += weightedDifference;
				total++;
			}
		}
	}

	if (total > 0)
	{
		steeringForce /= total;
		steeringForce.Normalize();
		steeringForce *= MaxAlignForce;
	}

	return steeringForce;
}

FVector ABoid::Align(const TArray<AActor*> boids)
{
	FVector steeringForce = FVector::ZeroVector;
	int32 total = 0;

	for (AActor* actor : boids)
	{
		if (ABoid* boid = Cast<ABoid>(actor))
		{
			if (boid != this)
			{
				steeringForce += boid->GetBoidVelocity();
				total++;
			}
		}
	}

	if (total > 0)
	{
		steeringForce /= total;
		steeringForce.Normalize();
		steeringForce *= MaxAlignForce;
	}

	return steeringForce;

}
FVector ABoid::Cohesion(const TArray<AActor*> boids)
{
	FVector steeringForce = FVector::ZeroVector;
	int32 total = 0;

	for (AActor* actor : boids)
	{
		if (ABoid* boid = Cast<ABoid>(actor))
		{
			if (boid != this)
			{
				steeringForce += boid->GetActorLocation();
				total++;
			}
		}
	}

	if (total > 0)
	{
		steeringForce /= total;
		steeringForce -= GetActorLocation();
		steeringForce.Normalize();
		steeringForce *= MaxAlignForce;
	}

	return steeringForce;
}
