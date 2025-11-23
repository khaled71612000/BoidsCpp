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
    const int32 dataBoidsCount = DataBoids.Num();
    if (dataBoidsCount == 0 || !DataBoidISM)
    {
        return;
    }

    RebuildDataTree();

    // 1) Parallel simu on raw data
    ParallelFor(dataBoidsCount, [this, DeltaTime](int32 Index)
        {
            FBoidData& boid = DataBoids[Index];

            TArray<int32> neighborIndices;
            GetNeighborsData(Index, boid.PerceptionRadius, neighborIndices);

            FVector accForce = FVector::ZeroVector;
            if (!boid.bDisableSeparation)
            {
                accForce += ComputeSeparationData(Index, neighborIndices);
            }
            if (!boid.bDisableAlign)
            {
                accForce += ComputeAlignmentData(Index, neighborIndices);
            }
            if (!boid.bDisableCohesion)
            {
                accForce += ComputeCohesionData(Index, neighborIndices);
            }

            boid.Acceleration = accForce;

            boid.Velocity += boid.Acceleration * DeltaTime;
            boid.Velocity = boid.Velocity.GetClampedToSize(boid.MinSpeed, boid.MaxSpeed);
            if (boid.bDisableZ)
            {
                boid.Velocity.Z = 0.0f;
            }

            boid.Position += boid.Velocity * DeltaTime;
            bool isAdjusted = false;
            if (!WorldMin.IsNearlyZero() || !WorldMax.IsNearlyZero())
            {
                // X
                if (boid.Position.X < WorldMin.X)
                {
                    boid.Position.X = WorldMin.X;
                    boid.Velocity.X = FMath::Abs(boid.Velocity.X);
                    isAdjusted = true;
                }
                else if (boid.Position.X > WorldMax.X)
                {
                    boid.Position.X = WorldMax.X;
                    boid.Velocity.X = -FMath::Abs(boid.Velocity.X);
                    isAdjusted = true;
                }

                // Y
                if (boid.Position.Y < WorldMin.Y)
                {
                    boid.Position.Y = WorldMin.Y;
                    boid.Velocity.Y = FMath::Abs(boid.Velocity.Y);
                    isAdjusted = true;
                }
                else if (boid.Position.Y > WorldMax.Y)
                {
                    boid.Position.Y = WorldMax.Y;
                    boid.Velocity.Y = -FMath::Abs(boid.Velocity.Y);
                    isAdjusted = true;
                }

                // Z
                if (!boid.bDisableZ)
                {
                    if (boid.Position.Z < WorldMin.Z)
                    {
                        boid.Position.Z = WorldMin.Z;
                        boid.Velocity.Z = FMath::Abs(boid.Velocity.Z);
                        isAdjusted = true;
                    }
                    else if (boid.Position.Z > WorldMax.Z)
                    {
                        boid.Position.Z = WorldMax.Z;
                        boid.Velocity.Z = -FMath::Abs(boid.Velocity.Z);
                        isAdjusted = true;
                    }
                }

                if (isAdjusted)
                {
                    // Data is just set unlike actors
                }
            }
        });

    // 2) Gamethread data to actor transform
    for (int32 i = 0; i < dataBoidsCount; ++i)
    {
        const FBoidData& boidData = DataBoids[i];

        FRotator rotation = FRotator::ZeroRotator;
        if (boidData.Velocity.SizeSquared() > KINDA_SMALL_NUMBER)
        {
            rotation = boidData.Velocity.Rotation();
        }

        const FTransform xForm(rotation, boidData.Position);
        DataBoidISM->UpdateInstanceTransform(i, xForm, true, false , false);
    }

    DataBoidISM->MarkRenderStateDirty();
}

void UFlockSubsystem::InitDataBoids(
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
    float InRotationInterpSpeed)
{
    if (!InISM || NumBoids <= 0)
    {
        return;
    }

    SimulationMode = EFlockSimulationMode::DataOriented;
    DataBoidISM = InISM;

    DataBoids.SetNum(NumBoids);
    DataOctNodes.Reset();
    DataRootNodeIndex = INDEX_NONE;

    DataBoidISM->ClearInstances();
    for (int32 i = 0; i < NumBoids; ++i)
    {
        const FVector spawnPos = FMath::RandPointInBox(SpawnBounds);
        const FVector direction = FMath::VRand().GetSafeNormal();

        FBoidData& Boid = DataBoids[i];
        Boid.bDisableZ = bInDisableZ;
        Boid.bDisableAlign = bInDisableAlign;
        Boid.bDisableCohesion = bInDisableCohesion;
        Boid.bDisableSeparation = bInDisableSeparation;
        Boid.Position = spawnPos;
        Boid.Velocity = direction * FMath::RandRange(InMinAlignForce, InMaxAlignForce);
        Boid.MinSpeed = InMinAlignForce;
        Boid.MaxSpeed = InMaxAlignForce;
        Boid.PerceptionRadius = InPerceptionRadius;
        Boid.RotationInterpSpeed = InRotationInterpSpeed;

        const FRotator rot = Boid.Velocity.SizeSquared() > KINDA_SMALL_NUMBER
            ? Boid.Velocity.Rotation()
            : FRotator::ZeroRotator;

        const FTransform xForm(rot, spawnPos);
        DataBoidISM->AddInstance(xForm);
    }
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

    DataBoids.Reset();
    DataOctNodes.Reset();
    DataRootNodeIndex = INDEX_NONE;
    DataBoidISM = nullptr;
}

void UFlockSubsystem::Deinitialize()
{
    OctNodes.Reset();
    Boids.Reset();
    RootNodeIndex = INDEX_NONE;

    DataBoids.Reset();
    DataOctNodes.Reset();
    DataRootNodeIndex = INDEX_NONE;
    DataBoidISM = nullptr;
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

    FBoidSpatialNode root;
    root.Center = (WorldMin + WorldMax) * 0.5f;
    root.HalfSize = (WorldMax - WorldMin) * 0.5f;
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



FVector UFlockSubsystem::ComputeSeparationData(int32 Index, const TArray<int32>& Neighbors)
{
    const FBoidData& boid = DataBoids[Index];
    FVector force = FVector::ZeroVector;
    int32 count = 0;
    for (int32 otherIndex : Neighbors)
    {
        const FBoidData& other = DataBoids[otherIndex];

        FVector diff = boid.Position - other.Position;
        const float dist = diff.Size();
        if (dist < KINDA_SMALL_NUMBER)
        {
            continue;
        }

        force += diff / dist;
        ++count;
    }

    if (count > 0)
    {
        force /= static_cast<float>(count);
        force.Normalize();
        force *= boid.MaxSpeed;
    }

    return force;
}

FVector UFlockSubsystem::ComputeAlignmentData(int32 Index, const TArray<int32>& Neighbors)
{
    const FBoidData& boid = DataBoids[Index];
    FVector force = FVector::ZeroVector;
    int32 count = 0;

    for (int32 otherIndex : Neighbors)
    {
        const FBoidData& other = DataBoids[otherIndex];
        force += other.Velocity;
        ++count;
    }

    if (count > 0)
    {
        force /= static_cast<float>(count);
        force.Normalize();
        force *= boid.MaxSpeed;
    }

    return force;
}

FVector UFlockSubsystem::ComputeCohesionData(int32 Index, const TArray<int32>& Neighbors)
{
    const FBoidData& boid = DataBoids[Index];
    FVector force = FVector::ZeroVector;
    int32 count = 0;
    for (int32 otherIndex : Neighbors)
    {
        const FBoidData& other = DataBoids[otherIndex];
        force += other.Position;
        ++count;
    }

    if (count > 0)
    {
        force /= static_cast<float>(count);
        force -= boid.Position;
        force.Normalize();
        force *= boid.MaxSpeed;
    }

    return force;
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


void UFlockSubsystem::RebuildDataTree()
{
    DataOctNodes.Reset();
    DataRootNodeIndex = INDEX_NONE;

    const int32 currentBoid = DataBoids.Num();
    if (currentBoid == 0)
    {
        return;
    }

    DataOctNodes.Reserve(currentBoid * 2);

    FBoidDataSpatialNode root;
    root.Center = (WorldMin + WorldMax) * 0.5f;
    root.HalfSize = (WorldMax - WorldMin) * 0.5f;
    root.Depth = 0;

    DataRootNodeIndex = DataOctNodes.Add(root);
    for (int32 i = 0; i < currentBoid; ++i)
    {
        InsertDataBoidIntoNode(DataRootNodeIndex, i);
    }
}

void UFlockSubsystem::InsertDataBoidIntoNode(int32 NodeIndex, int32 BoidIndex)
{
    if (!DataOctNodes.IsValidIndex(NodeIndex) || !DataBoids.IsValidIndex(BoidIndex))
    {
        return;
    }

    FBoidDataSpatialNode& node = DataOctNodes[NodeIndex];
    const FVector loc = DataBoids[BoidIndex].Position;
    const FVector delta = (loc - node.Center).GetAbs();

    if (delta.X > node.HalfSize.X ||
        delta.Y > node.HalfSize.Y ||
        delta.Z > node.HalfSize.Z)
    {
        node.BoidIndices.Add(BoidIndex);
        return;
    }

    if (node.HalfSize.GetMin() <= MinNodeHalfSize)
    {
        node.BoidIndices.Add(BoidIndex);
        return;
    }

    if (node.IsLeaf() &&
        (node.BoidIndices.Num() < MaxBoidsPerNode || node.Depth >= MaxTreeDepth))
    {
        node.BoidIndices.Add(BoidIndex);
        return;
    }

    if (node.IsLeaf())
    {
        SubdivideDataNode(NodeIndex);
    }

    const FBoidDataSpatialNode& updatedNode = DataOctNodes[NodeIndex];
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
    if (!DataOctNodes.IsValidIndex(childNodeIndex))
    {
        DataOctNodes[NodeIndex].BoidIndices.Add(BoidIndex);
        return;
    }

    InsertDataBoidIntoNode(childNodeIndex, BoidIndex);
}

void UFlockSubsystem::SubdivideDataNode(int32 NodeIndex)
{
    if (!DataOctNodes.IsValidIndex(NodeIndex))
    {
        return;
    }

    const FBoidDataSpatialNode nodeCopy = DataOctNodes[NodeIndex];
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
            FBoidDataSpatialNode Child;
            Child.Center = nodeCopy.Center + CenterOffset;
            Child.HalfSize = quarter;
            Child.Depth = nodeCopy.Depth + 1;
            return DataOctNodes.Add(Child);
        };

    childIndices[0] = CreateChild(FVector(-quarter.X, -quarter.Y, -quarter.Z));
    childIndices[1] = CreateChild(FVector(+quarter.X, -quarter.Y, -quarter.Z));
    childIndices[2] = CreateChild(FVector(-quarter.X, +quarter.Y, -quarter.Z));
    childIndices[3] = CreateChild(FVector(+quarter.X, +quarter.Y, -quarter.Z));
    childIndices[4] = CreateChild(FVector(-quarter.X, -quarter.Y, +quarter.Z));
    childIndices[5] = CreateChild(FVector(+quarter.X, -quarter.Y, +quarter.Z));
    childIndices[6] = CreateChild(FVector(-quarter.X, +quarter.Y, +quarter.Z));
    childIndices[7] = CreateChild(FVector(+quarter.X, +quarter.Y, +quarter.Z));

    FBoidDataSpatialNode& nodeRef = DataOctNodes[NodeIndex];
    for (int32 i = 0; i < 8; ++i)
    {
        nodeRef.Children[i] = childIndices[i];
    }

    TArray<int32> existingBoids = MoveTemp(nodeRef.BoidIndices);
    nodeRef.BoidIndices.Reset();

    for (int32 boidIndex : existingBoids)
    {
        if (!DataBoids.IsValidIndex(boidIndex))
        {
            continue;
        }

        InsertDataBoidIntoNode(NodeIndex, boidIndex);
    }
}

void UFlockSubsystem::QueryDataNode(int32 NodeIndex, const FVector& QueryPos, float RadiusSq, int32 QueryBoidIndex, TArray<int32>& OutNeighbors) const
{
    if (!DataOctNodes.IsValidIndex(NodeIndex))
    {
        return;
    }

    const FBoidDataSpatialNode& node = DataOctNodes[NodeIndex];
    if (!SphereIntersectsDataNode(QueryPos, RadiusSq, node))
    {
        return;
    }

    for (int32 otherIndex : node.BoidIndices)
    {
        if (!DataBoids.IsValidIndex(otherIndex) || otherIndex == QueryBoidIndex)
        {
            continue;
        }

        const float distSq = FVector::DistSquared(
            QueryPos,
            DataBoids[otherIndex].Position
        );

        if (distSq <= RadiusSq)
        {
            OutNeighbors.Add(otherIndex);
        }
    }

    if (!node.IsLeaf())
    {
        for (int32 childIdx = 0; childIdx < 8; ++childIdx)
        {
            const int32 childNodeIndex = node.Children[childIdx];
            if (childNodeIndex != INDEX_NONE)
            {
                QueryDataNode(childNodeIndex, QueryPos, RadiusSq, QueryBoidIndex, OutNeighbors);
            }
        }
    }
}

bool UFlockSubsystem::SphereIntersectsDataNode(const FVector& QueryPos, float RadiusSq, const FBoidDataSpatialNode& Node) const
{
    const FVector Delta = (QueryPos - Node.Center).GetAbs();

    const float dx = FMath::Max(Delta.X - Node.HalfSize.X, 0.0f);
    const float dy = FMath::Max(Delta.Y - Node.HalfSize.Y, 0.0f);
    const float dz = FMath::Max(Delta.Z - Node.HalfSize.Z, 0.0f);

    const float distSq = dx * dx + dy * dy + dz * dz;
    return distSq <= RadiusSq;
}

void UFlockSubsystem::GetNeighborsData(int32 QueryBoidIndex, float Radius, TArray<int32>& OutNeighbors) const
{
    OutNeighbors.Reset();

    if (!DataBoids.IsValidIndex(QueryBoidIndex) || DataRootNodeIndex == INDEX_NONE)
    {
        return;
    }

    const FVector queryPos = DataBoids[QueryBoidIndex].Position;
    const float radiusSq = Radius * Radius;

    QueryDataNode(DataRootNodeIndex, queryPos, radiusSq, QueryBoidIndex, OutNeighbors);
}
void UFlockSubsystem::DrawDebugTree(bool bDraw) const
{
#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
    if (!bDraw)
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

    if (WorldMin != WorldMax)
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
            5.0f);
    }

    // ---- Actor octree ----
    if (RootNodeIndex != INDEX_NONE && OctNodes.Num() > 0)
    {
        for (const FBoidSpatialNode& node : OctNodes)
        {
            if (!node.IsLeaf() || node.Boids.Num() == 0)
            {
                continue;
            }

            const FVector center = node.Center;
            const FVector extent = node.HalfSize;

            const float depthT = (MaxTreeDepth > 0)
                ? FMath::Clamp(node.Depth * invMaxDepth, 0.0f, 1.0f)
                : 0.0f;

            const int32 boidCount = node.Boids.Num();

            const FLinearColor linearColor = FLinearColor::LerpUsingHSV(
                FLinearColor::Blue,
                FLinearColor::Red,
                depthT);

            const FColor color = linearColor.ToFColor(true);
            DrawDebugBox(
                world,
                center,
                extent,
                color,
                false,
                0.0f,
                0,
                8.f);
        }
    }

    // ---- Data octree (for data boids) ----
    if (DataRootNodeIndex != INDEX_NONE && DataOctNodes.Num() > 0)
    {
        for (const FBoidDataSpatialNode& node : DataOctNodes)
        {
            if (!node.IsLeaf() || node.BoidIndices.Num() == 0)
            {
                continue;
            }

            const FVector center = node.Center;
            const FVector extent = node.HalfSize;

            const float depthT = (MaxTreeDepth > 0)
                ? FMath::Clamp(node.Depth * invMaxDepth, 0.0f, 1.0f)
                : 0.0f;

            const int32 boidCount = node.BoidIndices.Num();

            const FLinearColor linearColor = FLinearColor::LerpUsingHSV(
                FLinearColor::Blue,
                FLinearColor::Red,
                depthT);

            const FColor color = linearColor.ToFColor(true);
            DrawDebugBox(
                world,
                center,
                extent,
                color,
                false,
                0.0f,
                0,
                8.f);
        }
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
