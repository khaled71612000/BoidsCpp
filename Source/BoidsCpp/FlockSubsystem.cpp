#include "FlockSubsystem.h"

#include "Boid.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h" 

UFlockSubsystem::UFlockSubsystem()
{
    RootNodeIndex = INDEX_NONE;
}

void UFlockSubsystem::Tick(float DeltaTime)
{
    switch (SimulationMode)
    {
    case EFlockSimulationMode::Actors:
        TickActorBoids(DeltaTime);
        break;
    case EFlockSimulationMode::DataOriented:
        TickDataBoids(DeltaTime);
        break;
    default:
        TickActorBoids(DeltaTime);
        break;
    }
}

void UFlockSubsystem::TickActorBoids(float DeltaTime)
{
    RebuildTree();

    const int32 count = Boids.Num();
    ParallelFor(count, [this](int32 Index)
        {
            ABoid* Boid = Boids[Index];
            if (IsValid(Boid))
            {
                Boid->ComputeFlockForces();
            }
        });

    for (ABoid* Boid : Boids)
    {
        if (IsValid(Boid))
        {
            Boid->ApplyFlock(DeltaTime);
        }
    }
}

void UFlockSubsystem::TickDataBoids(float DeltaTime)
{
    // @todo_Khaled data oriented sim will go here
}

void UFlockSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

	if (const UFlockDeveloperSettings* Settings = UFlockDeveloperSettings::Get())
	{
		MaxBoidsPerNode = Settings->MaxBoidsPerNode;
		MaxTreeDepth = Settings->MaxTreeDepth;
		MinNodeHalfSize = Settings->MinNodeHalfSize;
        SimulationMode = Settings->SimulationMode;
	}

    OctNodes.Reset();
    Boids.Reset();
    RootNodeIndex = INDEX_NONE;
}

void UFlockSubsystem::Deinitialize()
{
    Super::Deinitialize();

    OctNodes.Reset();
    Boids.Reset();
    RootNodeIndex = INDEX_NONE;
}

void UFlockSubsystem::AddToBoidList(ABoid* Boid)
{
    if (!Boid)
    {
        return;
    }

    Boids.AddUnique(Boid);
}

void UFlockSubsystem::RemoveFromBoidList(ABoid* Boid)
{
    //@todo_khaled maybe boids who leave the spawner just spawn at the middle
    //use multithreading to update the tree and use a large count to make up for the simulation
    if (!Boid)
    {
        return;
    }

    Boids.Remove(Boid);
}

void UFlockSubsystem::RebuildTree()
{
    OctNodes.Reset();
    RootNodeIndex = INDEX_NONE;

    if (Boids.Num() == 0)
    {
        return;
    }

    OctNodes.Reserve(Boids.Num() * 2);

    const FVector min3D = WorldMin;
    const FVector max3D = WorldMax;

    FBoidSpatialNode root;
    root.Center = (min3D + max3D) * 0.5f;
    root.HalfSize = (max3D - min3D) * 0.5f;
    root.Depth = 0;

    RootNodeIndex = OctNodes.Add(root);

    for (ABoid* boid : Boids)
    {
        if (!IsValid(boid))
        {
            continue;
        }

        InsertBoidIntoNode(RootNodeIndex, boid);
    }
}


void UFlockSubsystem::InsertBoidIntoNode(int32 NodeIndex, ABoid* Boid)
{
    if (!OctNodes.IsValidIndex(NodeIndex) || !IsValid(Boid))
    {
        return;
    }

    FBoidSpatialNode& node = OctNodes[NodeIndex];

    const FVector loc = Boid->GetActorLocation();
    const FVector delta = (loc - node.Center).GetAbs();

    //@todo_khaled Outside this node's bounds?
    if (delta.X > node.HalfSize.X ||
        delta.Y > node.HalfSize.Y ||
        delta.Z > node.HalfSize.Z)
    {
        node.Boids.Add(Boid);
        return;
    }

    if (node.HalfSize.GetMin() <= MinNodeHalfSize)
    {
        node.Boids.Add(Boid);
        return;
    }

    if (node.IsLeaf() &&
        (node.Boids.Num() < MaxBoidsPerNode || node.Depth >= MaxTreeDepth))
    {
        node.Boids.Add(Boid);
        return;
    }

    if (node.IsLeaf())
    {
        SubdivideNode(NodeIndex);
    }

    const FBoidSpatialNode& updatedNode = OctNodes[NodeIndex];

    const bool isRight = (loc.X >= updatedNode.Center.X);
    const bool isTop = (loc.Y >= updatedNode.Center.Y);
    const bool isFront = (loc.Z >= updatedNode.Center.Z);

    int32 childIdx = 0;
    if (isRight)
    {
        childIdx |= 1;
    }
    if (isTop)
    { 
        childIdx |= 2;
    }
    if (isFront)
    { 
        childIdx |= 4;
    }

    const int32 childNodeIndex = updatedNode.Children[childIdx];
    if (!OctNodes.IsValidIndex(childNodeIndex))
    {
        OctNodes[NodeIndex].Boids.Add(Boid);
        return;
    }

    InsertBoidIntoNode(childNodeIndex, Boid);
}

void UFlockSubsystem::SetWorldBoundsFromVolume(UBoxComponent* Volume)
{
    if (!Volume)
    {
        return;
    }

    const FBoxSphereBounds& bounds = Volume->Bounds;
    const FVector newMin = bounds.Origin - bounds.BoxExtent;
    const FVector newMax = bounds.Origin + bounds.BoxExtent;
    if (WorldMin == FVector::ZeroVector && WorldMax == FVector::ZeroVector)
    {
        WorldMin = newMin;
        WorldMax = newMax;
    }
    else
    {
        WorldMin.X = FMath::Min(WorldMin.X, newMin.X);
        WorldMin.Y = FMath::Min(WorldMin.Y, newMin.Y);
        WorldMin.Z = FMath::Min(WorldMin.Z, newMin.Z);

        WorldMax.X = FMath::Max(WorldMax.X, newMax.X);
        WorldMax.Y = FMath::Max(WorldMax.Y, newMax.Y);
        WorldMax.Z = FMath::Max(WorldMax.Z, newMax.Z);
    }
}

void UFlockSubsystem::QueryNode(
    int32 NodeIndex,
    const FVector& QueryPos,
    float RadiusSq,
    ABoid* QueryBoid,
    TArray<ABoid*>& OutNeighbors) const
{
    if (!OctNodes.IsValidIndex(NodeIndex))
    {
        return;
    }

    const FBoidSpatialNode& node = OctNodes[NodeIndex];
    if (!SphereIntersectsNode(QueryPos, RadiusSq, node))
    {
        return;
    }

    for (ABoid* other : node.Boids)
    {
        if (!IsValid(other) || other == QueryBoid)
        {
            continue;
        }

        const float distSq = FVector::DistSquared(
            QueryPos,
            other->GetActorLocation()
        );

        if (distSq <= RadiusSq)
        {
            OutNeighbors.Add(other);
        }
    }

    if (!node.IsLeaf())
    {
        for (int32 childIdx = 0; childIdx < 8; ++childIdx)
        {
            const int32 childNodeIndex = node.Children[childIdx];
            if (childNodeIndex != INDEX_NONE)
            {
                QueryNode(childNodeIndex, QueryPos, RadiusSq, QueryBoid, OutNeighbors);
            }
        }
    }
}


bool UFlockSubsystem::SphereIntersectsNode(
    const FVector& QueryPos,
    float RadiusSq,
    const FBoidSpatialNode& Node) const
{
    const FVector delta = (QueryPos - Node.Center).GetAbs();

    const float dx = FMath::Max(delta.X - Node.HalfSize.X, 0.0f);
    const float dy = FMath::Max(delta.Y - Node.HalfSize.Y, 0.0f);
    const float dz = FMath::Max(delta.Z - Node.HalfSize.Z, 0.0f);

    const float distSq = dx * dx + dy * dy + dz * dz;
    return distSq <= RadiusSq;
}


void UFlockSubsystem::DrawDebugTree(bool bDraw) const
{
#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
	if (!bDraw || RootNodeIndex == INDEX_NONE)
	{
		return;
	}

	const UWorld* const world = GetWorld();
	if (!world)
	{
		return;
	}

	const float invMaxDepth = (MaxTreeDepth > 0)
		? 1.0f / static_cast<float>(MaxTreeDepth)
		: 0.0f;

	if (!WorldMin.IsNearlyZero() || !WorldMax.IsNearlyZero())
	{
		const FVector worldCenter = (WorldMin + WorldMax) * 0.5f;
		const FVector worldExtent = (WorldMax - WorldMin) * 0.5f;

		DrawDebugBox(
			world,
			worldCenter,
			worldExtent,
			FColor::White,
			false,
			0.0f,
			0,
			2.0f);
	}

	for (const FBoidSpatialNode& node : OctNodes)
	{
		// Only draw useful nodes: leaves that actually contain boids
		if (!node.IsLeaf() || node.Boids.Num() == 0)
		{
			continue;
		}

		const FVector Center = node.Center;
		const FVector Extent = node.HalfSize;

		const float DepthT = FMath::Clamp(node.Depth * invMaxDepth, 0.0f, 1.0f);
		const int32 BoidCount = node.Boids.Num();

		// Depth → color (blue → red), count → line thickness
		const FLinearColor LinearColor = FLinearColor::LerpUsingHSV(
			FLinearColor::Blue,
			FLinearColor::Red,
			DepthT);

		const FColor Color = LinearColor.ToFColor(true);
		const float Thickness = FMath::Clamp(1.0f + BoidCount * 0.2f, 1.0f, 8.0f);

		DrawDebugBox(
			world,
			Center,
			Extent,
			Color,
			false,  // not persistent
			0.0f,   // lifetime
			0,      // depth priority
			Thickness);
	}
#endif
}


void UFlockSubsystem::GetNeighbors(ABoid* Query, float Radius, TArray<ABoid*>& OutNeighbors) const
{
    OutNeighbors.Reset();

    if (!IsValid(Query) || RootNodeIndex == INDEX_NONE)
    {
        return;
    }

    const FVector queryPos = Query->GetActorLocation();
    const float radiusSq = Radius * Radius;

    QueryNode(RootNodeIndex, queryPos, radiusSq, Query, OutNeighbors);
}

void UFlockSubsystem::SubdivideNode(int32 NodeIndex)
{
    if (!OctNodes.IsValidIndex(NodeIndex))
    {
        return;
    }

    //@todo_khaled nodeCopy is safer but should be revisted when threading
    const FBoidSpatialNode nodeCopy = OctNodes[NodeIndex];

    if (!nodeCopy.IsLeaf())
    {
        return;
    }

    const FVector quarter = nodeCopy.HalfSize * 0.5f;
    if (quarter.GetMin() <= MinNodeHalfSize)
    {
        return;
    }

    int32 childIndices[8];
    auto CreateChild = [&](const FVector& CenterOffset) -> int32
        {
            FBoidSpatialNode Child;
            Child.Center = nodeCopy.Center + CenterOffset;
            Child.HalfSize = quarter;
            Child.Depth = nodeCopy.Depth + 1;
            return OctNodes.Add(Child);
        };


    childIndices[0] = CreateChild(FVector(-quarter.X, -quarter.Y, -quarter.Z)); // 000
    childIndices[1] = CreateChild(FVector(+quarter.X, -quarter.Y, -quarter.Z)); // 001
    childIndices[2] = CreateChild(FVector(-quarter.X, +quarter.Y, -quarter.Z)); // 010
    childIndices[3] = CreateChild(FVector(+quarter.X, +quarter.Y, -quarter.Z)); // 011
    childIndices[4] = CreateChild(FVector(-quarter.X, -quarter.Y, +quarter.Z)); // 100
    childIndices[5] = CreateChild(FVector(+quarter.X, -quarter.Y, +quarter.Z)); // 101
    childIndices[6] = CreateChild(FVector(-quarter.X, +quarter.Y, +quarter.Z)); // 110
    childIndices[7] = CreateChild(FVector(+quarter.X, +quarter.Y, +quarter.Z)); // 111

    FBoidSpatialNode& nodeRef = OctNodes[NodeIndex];
    for (int32 i = 0; i < 8; ++i)
    {
        nodeRef.Children[i] = childIndices[i];
    }

    TArray<ABoid*> existingBoids = MoveTemp(nodeRef.Boids);
    nodeRef.Boids.Reset();

    for (ABoid* boid : existingBoids)
    {
        if (!IsValid(boid))
        {
            continue;
        }

        InsertBoidIntoNode(NodeIndex, boid);
    }
}
