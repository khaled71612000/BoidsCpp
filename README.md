# BoidsCpp

![Unreal Engine](https://img.shields.io/badge/Unreal%20Engine-0E1128?logo=unrealengine&logoColor=white)
![C++](https://img.shields.io/badge/C++-00599C?logo=c%2B%2B&logoColor=white)
![License](https://img.shields.io/badge/license-MIT-green)

Unreal Engine C++ implementation of Craig Reynolds' **Boids flocking algorithm** with a custom **Octree spatial partitioning** system. Simulates large numbers of agents (birds, fish, drones) exhibiting emergent group behaviour through three simple steering rules: separation, alignment, and cohesion.

Supports two simulation modes — actor-based (full AActor per boid) and data-driven ISM-based (instanced mesh, scales to thousands) — switchable at runtime.

## Architecture

Key classes (from source code):

- **`ABoid`** (extends `AActor`) — Individual boid agent. Owns a skeletal mesh (animated) and static mesh. Each tick calls `ComputeFlockForces()` which queries the FlockSubsystem for spatial neighbours, then computes `Separation()`, `Align()`, and `Cohesion()` force vectors. Supports external force injection (`AddForce`, `RequestLaunch`), configurable perception radius, and per-rule toggle (disable alignment/cohesion/separation independently).

- **`UFlockSubsystem`** (extends `UTickableWorldSubsystem`) — The brains of the simulation. Manages all boids via an **Octree** (`FBoidSpatialNode` tree) for O(log n) neighbour queries instead of O(n²) brute force. Rebuilt every tick via `RebuildTree()`. Also handles the data-driven path: `InitDataBoids()` spawns `FBoidData` structs backed by an `UInstancedStaticMeshComponent` — no Actor overhead, scales to thousands.

- **`FBoidSpatialNode`** / **`FBoidDataSpatialNode`** — Octree node structs. Each node stores a center + half-extent and up to 8 children indices. Leaf nodes hold boid references. `SphereIntersectsNode()` drives the recursive spatial query.

- **`ABoidVolumeSpawner`** — Spawns boids uniformly within a box volume. Feeds the boids their spawn volume reference for bounds-wrapping.

- **`ABoidPointSpawner`** — Spawns boids from a list of discrete world points.

- **`FlockDeveloperSettings`** (`UDeveloperSettings` subclass) — Project settings panel for flock parameters (simulation mode, max boids per node, tree depth) editable in the UE5 Project Settings UI without recompiling.

## Features

- Separation, alignment, and cohesion rules — individually toggleable at runtime
- Octree spatial partitioning — efficient neighbour queries at scale
- Two simulation modes: Actor (full visual fidelity) and Data/ISM (high performance)
- Configurable perception radius, speed range, and rotation interpolation per boid
- Bounds wrapping — boids seamlessly re-enter the opposite side of the spawn volume
- External force injection and launch impulse support
- Animated crow skeletal mesh included

## Tech Stack

| Technology | Role |
|---|---|
| Unreal Engine 5 | Game engine |
| C++ | Simulation logic, subsystem, spatial structures |
| UInstancedStaticMeshComponent | High-performance data-mode rendering |
| UTickableWorldSubsystem | Global simulation manager |

## Getting Started

### Prerequisites
- Unreal Engine 5.x
- Visual Studio 2022 with **Desktop development with C++**
- Git LFS (`git lfs install`)

### Setup
```bash
git lfs install
git clone https://github.com/khaled71612000/BoidsCpp.git
```
1. Right-click `BoidsCpp.uproject` → **Generate Visual Studio project files**
2. Open `.sln` → Build (**Development Editor | Win64**)
3. Launch via Unreal Editor
4. Tune flock parameters in **Project Settings → Flock Developer Settings**
