#include "BoidPointSpawner.h"
#include "Components/BillboardComponent.h"
#include "Boid.h"
#include "FlockSubsystem.h"

ABoidPointSpawner::ABoidPointSpawner()
{
	PrimaryActorTick.bCanEverTick = true;

	SpawnPointBillboard = CreateDefaultSubobject<UBillboardComponent>(TEXT("Spawn Billboard"));
	RootComponent = SpawnPointBillboard;

	BoidSpawnNumber = 0;
}

void ABoidPointSpawner::BeginPlay()
{
	Super::BeginPlay();

	FlockSubsystem = GetWorld()->GetSubsystem<UFlockSubsystem>();

	SpawnBoids(BoidSpawnNumber);
}

void ABoidPointSpawner::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ABoidPointSpawner::SpawnBoids(int32 NumBoids)
{
	if (!ensure(FlockSubsystem))
	{
		return;
	}

	FVector SpawnLocation = this->GetActorLocation();
	FRotator SpawnRotation = FRotator::ZeroRotator;
	FActorSpawnParameters BoidSpawnParams;
	BoidSpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	for (int32 i = 0; i < NumBoids; ++i)
	{
		SpawnRotation = FMath::VRand().ToOrientationRotator();
		FlockSubsystem->AddToBoidList(GetWorld()->SpawnActor<ABoid>(BoidBp, SpawnLocation, SpawnRotation, BoidSpawnParams));
	}
}
