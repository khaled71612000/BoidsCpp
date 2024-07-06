#include "BoidVolumeSpawner.h"
#include "Components/BillboardComponent.h"
#include "Components/BoxComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Boid.h"
#include "FlockSubsystem.h"

ABoidVolumeSpawner::ABoidVolumeSpawner()
{
	PrimaryActorTick.bCanEverTick = true;

	SpawnPointBillboard = CreateDefaultSubobject<UBillboardComponent>(TEXT("Spawn Billboard"));
	RootComponent = SpawnPointBillboard;

	SpawnVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("Spawn Volume"));
	SpawnVolume->SetupAttachment(RootComponent);

	BoidSpawnNumber = 0;
}

void ABoidVolumeSpawner::BeginPlay()
{
	Super::BeginPlay();

	FlockSubsystem = GetWorld()->GetSubsystem<UFlockSubsystem>();

	SpawnBoids(BoidSpawnNumber);
}

void ABoidVolumeSpawner::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ABoidVolumeSpawner::SpawnBoids(int32 NumBoids)
{
	if (!ensure(FlockSubsystem))
	{
		return;
	}

	FVector SpawnOrigin = SpawnVolume->Bounds.Origin;
	FVector SpawnExtent = SpawnVolume->Bounds.BoxExtent;

	FRotator SpawnRotation = FRotator::ZeroRotator;
	FActorSpawnParameters BoidSpawnParams;
	BoidSpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	for (int32 i = 0; i < NumBoids; ++i)
	{
		//you could get actor location here and they will spawn in a perfect sphere since the VRand returns -1 to 1 in all directions
		FVector SpawnLocation = UKismetMathLibrary::RandomPointInBoundingBox(SpawnOrigin, SpawnExtent);
		SpawnRotation = FMath::VRand().ToOrientationRotator();
		ABoid* spawnActor = GetWorld()->SpawnActor<ABoid>(BoidBp, SpawnLocation, SpawnRotation, BoidSpawnParams);
		FlockSubsystem->AddToBoidList(spawnActor);
		spawnActor->SetSpawnVolume(this);
		spawnActor->SetMinMaxForce(MinAlignForce, MaxAlignForce);
		spawnActor->SetPercipRadius(PerceptionRadius);
		spawnActor->SetZToggle(bDisableZ);
		spawnActor->SetRules(bDisableAlign, bDisableCohesion, bDisableSeparation);
	}
}
