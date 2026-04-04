![9](https://github.com/khaled71612000/BoidsCpp/assets/59780800/b57b7711-6e21-420b-9a0f-106fb45c7a14)

# BoidsCpp

![Unreal Engine](https://img.shields.io/badge/Unreal%20Engine-0E1128?logo=unrealengine&logoColor=white)
![C++](https://img.shields.io/badge/C++-00599C?logo=c%2B%2B&logoColor=white)
![License](https://img.shields.io/badge/license-MIT-green)

## Live Demo

[![Watch Demo](https://img.youtube.com/vi/L2dsAzmNYwI/maxresdefault.jpg)](https://youtu.be/L2dsAzmNYwI)

> Click the thumbnail above to watch the full demo video.

Unreal Engine C++ prototype that simulates classic **boid flocking** in a `UWorldSubsystem`, with optional **data-oriented** simulation and gameplay influences (rings, volumes, world bounds).

![download (1)](https://github.com/user-attachments/assets/be15899a-0790-4127-8469-e495da572a85)
![Boids1](https://github.com/user-attachments/assets/f2e62988-d085-41b8-89f9-48936c0256b7)

---

## Key Components

### `UFlockSubsystem` (World Subsystem)

- Stores all boids (actor and data) and updates them every tick
- Uses a custom **Octree** (`FBoidSpatialNode`) for O(log n) neighbour queries — versus O(n²) brute force
- Supports both Actor-based and Data/ISM-based simulation modes, switchable at runtime
- Optional world bounds from a `UBoxComponent` volume

### `ABoid` (Actor Boid)

- Single boid actor with skeletal mesh (animated crow) and static mesh
- Reads neighbours from the subsystem and computes `Separation()`, `Align()`, `Cohesion()` forces
- Per-boid toggles: disable Z movement, disable alignment, separation, or cohesion independently
- Supports external force injection (`AddForce`) and launch impulse (`RequestLaunch`)

### `ABoidVolumeSpawner`

- Box volume that spawns a configurable number of boids
- Supports both actor and data-oriented (ISM) paths
- Boids wrap back into bounds when they leave the volume

### `ABoidRing`

- Influence actor that steers nearby boids — pass-through, swirl, or avoidance modes
- Optional launch boost when boids enter an inner radius

### `UFlockDeveloperSettings`

- Project Settings panel under **Project Settings → Flock**
- Controls simulation mode (actors / data), max neighbours, tree depth — no recompile needed

---

## Features

- Classic flocking rules: separation, alignment, cohesion — individually toggleable per boid
- Octree spatial partitioning — efficient neighbour queries at scale
- Actor or data-oriented (ISM) simulation paths
- Level setup via spawner volumes and rings, no custom code needed
- Optional world bounds so boids stay inside a defined space

---

## Performance

Without spatial partitioning:
<img width="1189" height="525" alt="no-octree" src="https://github.com/user-attachments/assets/f5374c7c-022c-4944-a00e-d5a7faff9398" />

With Octree-based neighbour queries:
<img width="1051" height="378" alt="with-octree" src="https://github.com/user-attachments/assets/12589508-ee3c-48c3-b974-26063859d7fc" />

Supports **15–18k** ISM boid instances at ~60 FPS on a high-end CPU.
<img width="1056" height="509" alt="15k boids" src="https://github.com/user-attachments/assets/d606400a-04e1-421a-9cf1-7e3fd6a41b55" />

---

## Screenshots

![image](https://github.com/khaled71612000/BoidsCpp/assets/59780800/5ff6f549-8c8e-470d-918d-90dbfa3d03b3)
![image](https://github.com/khaled71612000/BoidsCpp/assets/59780800/a1cf2645-6f31-45e0-bfef-9e69aa942d4a)
![image](https://github.com/khaled71612000/BoidsCpp/assets/59780800/dd67af29-e111-4c67-8b7f-2e8b4a3cc908)
![image](https://github.com/khaled71612000/BoidsCpp/assets/59780800/7af14b98-7cc8-4631-be4c-fa87049b2fb8)

---

## Tech Stack

| Technology | Role |
|---|---|
| Unreal Engine 5 | Game engine |
| C++ | Simulation logic, subsystem, spatial structures |
| UInstancedStaticMeshComponent | High-performance data-mode rendering |
| UTickableWorldSubsystem | Global simulation manager |

---

## Getting Started

### Prerequisites
- Unreal Engine 5.x
- Visual Studio 2022 with **Desktop development with C++**
- Git LFS (`git lfs install`)

```bash
git lfs install
git clone https://github.com/khaled71612000/BoidsCpp.git
```
1. Right-click `BoidsCpp.uproject` → **Generate Visual Studio project files**
2. Open `.sln` → Build (**Development Editor | Win64**)
3. Launch via Unreal Editor
4. Tune flock parameters in **Project Settings → Flock Developer Settings**

---

## References

- [Boids Algorithm – Craig Reynolds](https://www.red3d.com/cwr/boids/)
- [Steering Behaviors for Autonomous Characters](https://www.red3d.com/cwr/steer/gdc99/)