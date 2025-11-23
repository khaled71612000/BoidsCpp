
![9](https://github.com/khaled71612000/BoidsCpp/assets/59780800/b57b7711-6e21-420b-9a0f-106fb45c7a14)
# BoidsCpp
BoidsCPP is a small Unreal Engine prototype that implements a classic boid flocking simulation as a **World Subsystem**, with optional **data oriented** simulation and simple **gameplay influences** (rings, volumes, world bounds).

![download (1)](https://github.com/user-attachments/assets/be15899a-0790-4127-8469-e495da572a85)

## Key Components

### `UFlockSubsystem` (World Subsystem)
Central brain of the flocking system.
- Owns the **boid arrays**:
  - `TArray<ABoid*> Boids` for actor-based simulation
  - `TArray<FBoidData> DataBoids` for data-oriented simulation
- Maintains **spatial partitioning**:
  - `TArray<FBoidSpatialNode>` and `TArray<FBoidDataSpatialNode>` (octree-style)
  - Configurable via `UFlockDeveloperSettings`
- Handles **per-frame simulation**:
  - `Tick(float DeltaTime)` switches between Actor / Data modes
  - `TickActorBoids(DeltaTime)` and `TickDataBoids(DeltaTime)`:
    - Rebuilds spatial tree
    - Queries neighbors
    - Computes separation, alignment, and cohesion-like forces
    - Updates positions and rotations
- Supports **world bounds**:
  - `SetWorldBoundsFromVolume(UBoxComponent* Volume)`
  - `CheckBounds(Position, Velocity, bDisableZ)` to clamp or wrap boids

### `ABoid` (Actor Boid)
An actor-based boid that delegates most logic to the subsystem.
- Visual representation:
  - `USkeletalMeshComponent* BoidSkeletalMesh` as root
- Motion parameters:
  - `float MinAlignForce`, `float MaxAlignForce`
  - `float PerceptionRadius`
  - `float RotationInterpSpeed`
  - Toggle flags: `bDisableZ`, `bDisableAlign`, `bDisableSeparation`, `bDisableCohesion`
- Core methods:
  - `SimulateFlock(float DeltaTime)`
  - `ComputeFlockForces()`
  - `ApplyFlock(float DeltaTime)`
  - `AddForce(const FVector& Force)`
  - `RequestLaunch(const FVector& LaunchVelocity)`
  - Getter helpers such as `GetBoidPosition()`, `GetBoidVelocity()`, `GetPerceptionRadius()`


### `ABoidVolumeSpawner`
Utility actor used to spawn and configure a group of boids inside a box volume.
- Components:
  - `UBoxComponent* SpawnVolume`
  - `UBillboardComponent* SpriteComponent`
- Spawning:
  - `int32 BoidSpawnNumber`
  - `TSubclassOf<ABoid> BoidClass`
- Optional **data-oriented boid** support:
  - `UInstancedStaticMeshComponent* DataBoidISM`
  - Parameters passed to the subsystem for data boids:
    - Perception radius
    - Min / Max speed / force
    - Rotation interpolation speed
- Behavior toggles applied to spawned boids:
  - `bDisableZ`
  - `bDisableAlign`
  - `bDisableSeparation`
  - `bDisableCohesion`

### `ABoidRing`
A gameplay “influence field” that affects nearby boids.
- Component:
  - `UStaticMeshComponent* DetectionSphere` used as an overlap / detection volume
- Parameters:
  - `bool bGoThroughRing`  
    - `true` → boids are encouraged to pass through the ring
    - `false` → boids steer around it
  - `float LaunchSpeed` – optional forward launch velocity
  - `float InnerRadiusFrac` – inner region fraction relative to the detection sphere radius
- Behavior:
  - `InfluenceBoids()`:
    - Finds overlapping `ABoid` instances
    - Computes a direction to the ring and a tangential direction for swirl-like motion
    - Applies a weighted force via `AddForce`
    - When configured, calls `RequestLaunch` with a forward launch vector

### `UFlockDeveloperSettings`
Project-level tuning for the flock system, exposed under **Project Settings → Flock**.
- Octree settings:
  - `int32 MaxBoidsPerNode`
  - `int32 MaxTreeDepth`
  - `float MinNodeHalfSize`
- Simulation:
  - `EFlockSimulationMode SimulationMode` (Actors / DataOriented)
  - `int32 MaxNeighborsToConsider`

## Features

- **World Subsystem–based design**
  - Centralized flock logic independent of any specific map
  - Easy to find and profile (single subsystem, single tick entry point)

- **Two simulation paths**
  - **Actor Mode**:
    - Boids as regular `AActor` instances
    - Good for small to medium flocks with per-boid gameplay
  - **Data-Oriented Mode**:
    - Boids stored as `FBoidData` structs
    - Positions and velocities simulated in bulk
    - Rendered through a `UInstancedStaticMeshComponent` for large flocks

- **Spatial partitioning**
  - Custom octree-like structure for neighbor queries
  - Uses squared distance checks for cache-friendly, branch-light queries
  - Configurable via developer settings

- **Configurable boid behavior**
  - Perception radius
  - Min/max speed (via `MinAlignForce` / `MaxAlignForce` parameters)
  - Optional disabling of:
    - Z-axis motion
    - Alignment
    - Separation
    - Cohesion

- **Influence rings (steering volumes)**
  - Simple way to:
    - Pull boids through a ring
    - Make them orbit or steer around an object
    - Apply launch forces for “burst” behavior

- **World bounds support**
  - Optional world bounds derived from a `UBoxComponent`
  - Simple containment / wrap logic via `CheckBounds`

## References
- Craig Reynolds' Boids: [Boids Algorithm](https://www.red3d.com/cwr/boids/)
- Steering Behaviors: [Steering Behaviors For Autonomous Characters](https://www.red3d.com/cwr/steer/gdc99/)

This project serves as an excellent example for developers interested in exploring AI and simulation techniques using C++.
[YouTube Video](https://youtu.be/L2dsAzmNYwI?si=cwHMqw6KVNRLABKw)

![image](https://github.com/khaled71612000/BoidsCpp/assets/59780800/5ff6f549-8c8e-470d-918d-90dbfa3d03b3)
![image](https://github.com/khaled71612000/BoidsCpp/assets/59780800/a1cf2645-6f31-45e0-bfef-9e69aa942d4a)
![image](https://github.com/khaled71612000/BoidsCpp/assets/59780800/dd67af29-e111-4c67-8b7f-2e8b4a3cc908)
![image](https://github.com/khaled71612000/BoidsCpp/assets/59780800/7af14b98-7cc8-4631-be4c-fa87049b2fb8)
