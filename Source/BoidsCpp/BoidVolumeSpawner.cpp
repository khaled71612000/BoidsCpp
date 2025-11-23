#include "BoidVolumeSpawner.h"
#include "Components/BillboardComponent.h"
#include "Components/BoxComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Boid.h"
#include "FlockSubsystem.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "FlockDeveloperSettings.h"  

ABoidVolumeSpawner::ABoidVolumeSpawner()
{
    PrimaryActorTick.bCanEverTick = true;

    SpawnPointBillboard = CreateDefaultSubobject<UBillboardComponent>(TEXT("Spawn Billboard"));
    RootComponent = SpawnPointBillboard;

    SpawnVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("Spawn Volume"));
    SpawnVolume->SetupAttachment(RootComponent);
}

void ABoidVolumeSpawner::BeginPlay()
{
    Super::BeginPlay();

    FlockSubsystem = GetWorld()->GetSubsystem<UFlockSubsystem>();

    if (FlockSubsystem && SpawnVolume)
    {
        FlockSubsystem->SetWorldBoundsFromVolume(SpawnVolume);
    }

    const UFlockDeveloperSettings* Settings = UFlockDeveloperSettings::Get();
    const EFlockSimulationMode Mode = Settings ? Settings->SimulationMode : EFlockSimulationMode::Actors;

    if (Mode == EFlockSimulationMode::Actors)
    {
        SpawnBoids(BoidSpawnNumber);
    }
    else
    {
        SpawnDataBoids(BoidSpawnNumber);
    }
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

void ABoidVolumeSpawner::SpawnDataBoids(int32 NumBoids)
{
    if (!ensure(FlockSubsystem) || !SpawnVolume)
    {
        return;
    }

    if (!DataBoidMesh)
    {
        return;
    }

    if (!DataBoidISM)
    {
        DataBoidISM = NewObject<UInstancedStaticMeshComponent>(this, TEXT("BoidISM"));
        DataBoidISM->SetupAttachment(RootComponent);
        DataBoidISM->SetStaticMesh(DataBoidMesh);
        DataBoidISM->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        DataBoidISM->bCastDynamicShadow = false;
        DataBoidISM->RegisterComponent();
    }

    const FVector origin = SpawnVolume->Bounds.Origin;
    const FVector extent = SpawnVolume->Bounds.BoxExtent;
    const FBox spawnBounds(origin - extent, origin + extent);

    FlockSubsystem->InitDataBoids(
        NumBoids,
        spawnBounds,
        DataBoidISM,
        PerceptionRadius,
        MinAlignForce,
        MaxAlignForce,
        bDisableZ,
        bDisableAlign,
        bDisableCohesion,
        bDisableSeparation,
        RotationInterpSpeed);
}
