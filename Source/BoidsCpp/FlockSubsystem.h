#pragma once

#include "CoreMinimal.h"
#include "Components/BoxComponent.h"
#include "Subsystems/WorldSubsystem.h"
#include "FlockDeveloperSettings.h"
#include "FlockSubsystem.generated.h"

class ABoid;

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

UCLASS()
class BOIDSCPP_API UFlockSubsystem : public UTickableWorldSubsystem
{
    GENERATED_BODY()

public:
    UFlockSubsystem();

    virtual void Tick(float DeltaTime) override;
    void TickActorBoids(float DeltaTime);
    void TickDataBoids(float DeltaTime);

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

private:
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
};
