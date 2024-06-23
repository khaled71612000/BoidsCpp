#include "Boid.h"
#include "FlockSubsystem.h"

ABoid::ABoid()
{
	PrimaryActorTick.bCanEverTick = true;

	BoidMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Boid Mesh Component"));
	BoidMesh->SetupAttachment(RootComponent);
	BoidMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	BoidMesh->SetCollisionResponseToAllChannels(ECR_Ignore);

	BoidVelocity = FVector::ZeroVector;
}

void ABoid::BeginPlay()
{
	Super::BeginPlay();

	BoidVelocity = GetActorForwardVector();
	BoidVelocity.Normalize();
	BoidVelocity *= FMath::RandRange(MinAlignForce, MaxAlignForce);

	FlockSubsystem = GetWorld()->GetSubsystem<UFlockSubsystem>();

	CurrentRotation = GetActorRotation();
}

void ABoid::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	Flock(DeltaTime);
	UpdateRotation(DeltaTime);
}

void ABoid::Flock(float DeltaTime)
{
	FVector Acceleration = FVector::ZeroVector;

	SetActorLocation(GetActorLocation() + (BoidVelocity * DeltaTime));
	Acceleration += Align(FlockSubsystem->GetBoidList());

	BoidVelocity += (Acceleration * DeltaTime);
	BoidVelocity = BoidVelocity.GetClampedToSize(MinAlignForce, MaxAlignForce);
	SetActorRotation(BoidVelocity.ToOrientationQuat());
}

void ABoid::UpdateRotation(float DeltaTime)
{
	CurrentRotation = FMath::RInterpTo(CurrentRotation, GetActorRotation(), DeltaTime, 7.0f);
	BoidMesh->SetWorldRotation(CurrentRotation);
}

FVector ABoid::Align(const TSet<ABoid*> Boids)
{
	FVector SteeringForce = FVector::ZeroVector;
	int32 Total = 0;

	for (ABoid* Boid : Boids)
	{
		if (Boid != this && FVector::Dist(GetActorLocation(), Boid->GetActorLocation()) < PerceptionRadius)
		{
			SteeringForce += Boid->GetBoidVelocity();
			Total++;
		}
	}

	if (Total > 0)
	{
		SteeringForce /= Total;
		SteeringForce = SteeringForce.GetSafeNormal() * BoidVelocity.Size();
		SteeringForce -= BoidVelocity;
		SteeringForce *= MaxSpeed;
		SteeringForce = SteeringForce.GetClampedToSize(MinAlignForce, MaxAlignForce);
	}

	return SteeringForce;
}
