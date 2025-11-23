#pragma once

#include "CoreMinimal.h"
#include "Components/BoxComponent.h"
#include "Subsystems/WorldSubsystem.h"
#include "FlockDeveloperSettings.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "FlockSubsystem.generated.h"

class ABoid;

USTRUCT()
struct FBoidData
{
    GENERATED_BODY();

    FVector Position = FVector::ZeroVector;
    FVector Velocity = FVector::ZeroVector;
    FVector Acceleration = FVector::ZeroVector;

    float MinSpeed = 0.f;
    float MaxSpeed = 600.f;
    float PerceptionRadius = 200.f;
    float RotationInterpSpeed = 5.f;

    bool bDisableZ = false;
    bool bDisableAlign = false;
    bool bDisableSeparation = false;
    bool bDisableCohesion = false;
};

USTRUCT()
struct FBoidSpatialNode
{
    GENERATED_BODY()

    FVector Center = FVector::ZeroVector;
    FVector HalfSize = FVector::ZeroVector;

    TArray<ABoid*> Boids;

    int32 Children[8] =
    {
        INDEX_NONE, INDEX_NONE, INDEX_NONE, INDEX_NONE,
        INDEX_NONE, INDEX_NONE, INDEX_NONE, INDEX_NONE
    };

    int32 Depth = 0;

    bool IsLeaf() const { return Children[0] == INDEX_NONE; }
};

USTRUCT()
struct FBoidDataSpatialNode
{
    GENERATED_BODY();

    FVector Center = FVector::ZeroVector;
    FVector HalfSize = FVector::ZeroVector;

    TArray<int32> BoidIndices;

    int32 Children[8] =
    {
        INDEX_NONE, INDEX_NONE, INDEX_NONE, INDEX_NONE,
        INDEX_NONE, INDEX_NONE, INDEX_NONE, INDEX_NONE
    };

    int32 Depth = 0;

    bool IsLeaf() const { return Children[0] == INDEX_NONE; }
};

UCLASS()
class BOIDSCPP_API UFlockSubsystem : public UTickableWorldSubsystem
{
    GENERATED_BODY()

public:
    UFlockSubsystem();

    virtual void Tick(float DeltaTime) override;
    void TickActorBoids(float DeltaTime);
    void TickDataBoids(float DeltaTime);

    void InitDataBoids(
        int32 NumBoids,
        const FBox& SpawnBounds,
        UInstancedStaticMeshComponent* InISM,
        float InPerceptionRadius,
        float InMinAlignForce,
        float InMaxAlignForce,
        bool bInDisableZ,
        bool bInDisableAlign,
        bool bInDisableCohesion,
        bool bInDisableSeparation,
        float InRotationInterpSpeed);

    virtual TStatId GetStatId() const override
    {
        RETURN_QUICK_DECLARE_CYCLE_STAT(UFlockSubsystem, STATGROUP_Tickables);
    }

    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    void AddToBoidList(ABoid* Boid);
    void RemoveFromBoidList(ABoid* Boid);

    void GetNeighbors(ABoid* Query, float Radius, TArray<ABoid*>& OutNeighbors) const;
    void SetWorldBoundsFromVolume(UBoxComponent* Volume);
    void DrawDebugTree(bool bDraw) const;

    void RebuildTree();

    FVector ComputeSeparationData(int32 Index, const TArray<int32>& Neighbors);
    FVector ComputeAlignmentData(int32 Index, const TArray<int32>& Neighbors);
    FVector ComputeCohesionData(int32 Index, const TArray<int32>& Neighbors);

private:
    //Actor Functions
    void InsertBoidIntoNode(int32 NodeIndex, ABoid* Boid);
    void SubdivideNode(int32 NodeIndex);
    void QueryNode(
        int32 NodeIndex,
        const FVector& QueryPos,
        float RadiusSq,
        ABoid* QueryBoid,
        TArray<ABoid*>& OutNeighbors
    ) const;
    bool SphereIntersectsNode(
        const FVector& QueryPos,
        float Radius,
        const FBoidSpatialNode& Node
    ) const;

    //Data Functions
    void RebuildDataTree();
    void InsertDataBoidIntoNode(int32 NodeIndex, int32 BoidIndex);
    void SubdivideDataNode(int32 NodeIndex);
    void QueryDataNode(
        int32 NodeIndex,
        const FVector& QueryPos,
        float RadiusSq,
        int32 QueryBoidIndex,
        TArray<int32>& OutNeighbors
    ) const;
    bool SphereIntersectsDataNode(
        const FVector& QueryPos,
        float RadiusSq,
        const FBoidDataSpatialNode& Node
    ) const;
    void GetNeighborsData(int32 QueryBoidIndex, float Radius, TArray<int32>& OutNeighbors) const;


    UPROPERTY()
    EFlockSimulationMode SimulationMode = EFlockSimulationMode::Actors;
    
    int32 MaxBoidsPerNode = 8;
    int32 MaxTreeDepth = 7;
    float MinNodeHalfSize = 1.f;

    TArray<ABoid*> Boids;
    TArray<FBoidSpatialNode> OctNodes;
    int32 RootNodeIndex = INDEX_NONE;

    FVector WorldMin = FVector::ZeroVector;
    FVector WorldMax = FVector::ZeroVector;

    TArray<FBoidData> DataBoids;
    TArray<FBoidDataSpatialNode> DataOctNodes;
    int32 DataRootNodeIndex = INDEX_NONE;

    UPROPERTY()
    UInstancedStaticMeshComponent* DataBoidISM = nullptr;
};
