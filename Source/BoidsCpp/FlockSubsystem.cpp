#include "FlockSubsystem.h"
#include "Boid.h"

UFlockSubsystem::UFlockSubsystem()
{
}

void UFlockSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

void UFlockSubsystem::Deinitialize()
{
	Super::Deinitialize();
}

void UFlockSubsystem::AddToBoidList(ABoid* boid)
{
	Boids.Add(boid);
}

void UFlockSubsystem::RemoveFromBoidList(ABoid* boid)
{
	Boids.Remove(boid);
}
