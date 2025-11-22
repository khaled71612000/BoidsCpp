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

    if (FlockSubsystem && SpawnVolume)
    {
        FlockSubsystem->SetWorldBoundsFromVolume(SpawnVolume);
    }

    SpawnBoids(BoidSpawnNumber);
}

void ABoidVolumeSpawner::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

	if (FlockSubsystem)
	{
        FlockSubsystem->DrawDebugTree(bDrawQuadTree);
	}
}

void ABoidVolumeSpawner::SpawnBoids(int32 numBoids)
{
    if (!ensure(FlockSubsystem))
    {
        return;
    }

    FVector origin = SpawnVolume->Bounds.Origin;
    FVector extent = SpawnVolume->Bounds.BoxExtent;

    FRotator rotation = FRotator::ZeroRotator;
    FActorSpawnParameters params;
    params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    for (int32 i = 0; i < numBoids; ++i)
    {
        FVector spawnLoc = UKismetMathLibrary::RandomPointInBoundingBox(origin, extent);
        rotation = FMath::VRand().ToOrientationRotator();

        ABoid* boid = GetWorld()->SpawnActor<ABoid>(BoidBp, spawnLoc, rotation, params);

        FlockSubsystem->AddToBoidList(boid);

        boid->SetSpawnVolume(this);
        boid->SetMinMaxForce(MinAlignForce, MaxAlignForce);
        boid->SetPercipRadius(PerceptionRadius);
        boid->SetDisableZ(bDisableZ);
        boid->SetRules(bDisableAlign, bDisableCohesion, bDisableSeparation);
    }

    //FlockSubsystem->RebuildTree();
}
