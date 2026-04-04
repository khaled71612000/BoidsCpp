<p align="center">
  <img src="https://github.com/khaled71612000/BoidsCpp/assets/59780800/b57b7711-6e21-420b-9a0f-106fb45c7a14" width="100%" />
</p>

<h1 align="center">BoidsCpp</h1>

<p align="center">
  <img src="https://img.shields.io/badge/Unreal%20Engine%205-0E1128?style=flat-square&logo=unrealengine&logoColor=white" />
  <img src="https://img.shields.io/badge/C++-00599C?style=flat-square&logo=c%2B%2B&logoColor=white" />
  <img src="https://img.shields.io/badge/license-MIT-green?style=flat-square" />
</p>

<p align="center">
Unreal Engine 5 C++ implementation of Craig Reynolds' <strong>Boids flocking algorithm</strong> — powered by a custom <strong>Octree</strong> for O(log n) neighbour queries and a data-oriented ISM path that scales to <strong>15–18k agents at 60 FPS</strong>.
</p>

---

## Demo

<p align="center">
  <a href="https://youtu.be/L2dsAzmNYwI">
    <img src="https://img.youtube.com/vi/L2dsAzmNYwI/maxresdefault.jpg" width="80%" />
  </a>
</p>

---

## Features

- **Separation, alignment, cohesion** — each rule independently toggleable per boid at runtime
- **Two simulation modes** — Actor-based (full visual fidelity) and Data/ISM-based (high performance), switchable at runtime
- **Octree spatial partitioning** — O(log n) neighbour queries via `FBoidSpatialNode`, eliminates O(n²) brute force
- **Influence volumes** — `ABoidRing` steers nearby boids with pass-through, swirl, or avoidance modes
- **World bounds wrapping** — boids seamlessly re-enter the opposite side of the spawn volume
- **External forces** — `AddForce` / `RequestLaunch` API for gameplay interactions
- **No-code level setup** — place spawner volumes and rings; tweak everything in Project Settings

---

## Architecture

### `UFlockSubsystem` — World Subsystem
The central simulation manager. Ticks all boids, rebuilds the Octree every frame, and owns both the actor-boid list and the data-boid `FBoidData` structs. Exposes `GetNeighbours(center, radius)` used by every boid.

### `ABoid` — Actor Boid
A full `AActor` with animated crow skeletal mesh. Each tick reads neighbours from the subsystem and resolves `Separation()`, `Align()`, and `Cohesion()` into a final velocity. Supports per-axis and per-rule toggles plus external force injection.

### `FBoidSpatialNode` / `FBoidDataSpatialNode` — Octree
Recursive spatial tree built from scratch each tick. Each node stores a center + half-extent and up to 8 children. `SphereIntersectsNode()` drives the recursive neighbour query, cutting neighbour search from O(n²) to O(log n).

### `ABoidVolumeSpawner`
Box volume that spawns boids (actor or ISM path). Boids track this volume for bounds wrapping — leaving on one side re-enters on the opposite.

### `ABoidRing`
Influence actor with configurable inner/outer radii. Three interaction modes: pass-through (no effect), swirl (tangential force), avoidance (repulsion). Optional launch boost on inner-radius entry.

### `UFlockDeveloperSettings`
Exposes all flock parameters (simulation mode, max neighbours, tree depth, boid count) to **Project Settings → Flock** — no recompile needed.

---

## Performance

**Without** spatial partitioning (O(n²)):
<p><img src="https://github.com/user-attachments/assets/f5374c7c-022c-4944-a00e-d5a7faff9398" width="90%" /></p>

**With** Octree-based neighbour queries (O(log n)):
<p><img src="https://github.com/user-attachments/assets/12589508-ee3c-48c3-b974-26063859d7fc" width="90%" /></p>

**15–18k ISM boids at ~60 FPS:**
<p><img src="https://github.com/user-attachments/assets/d606400a-04e1-421a-9cf1-7e3fd6a41b55" width="90%" /></p>

---

## Screenshots

<p align="center">
  <img src="https://github.com/user-attachments/assets/be15899a-0790-4127-8469-e495da572a85" width="48%" />
  <img src="https://github.com/user-attachments/assets/f2e62988-d085-41b8-89f9-48936c0256b7" width="48%" />
</p>
<p align="center">
  <img src="https://github.com/khaled71612000/BoidsCpp/assets/59780800/5ff6f549-8c8e-470d-918d-90dbfa3d03b3" width="48%" />
  <img src="https://github.com/khaled71612000/BoidsCpp/assets/59780800/a1cf2645-6f31-45e0-bfef-9e69aa942d4a" width="48%" />
</p>
<p align="center">
  <img src="https://github.com/khaled71612000/BoidsCpp/assets/59780800/dd67af29-e111-4c67-8b7f-2e8b4a3cc908" width="48%" />
  <img src="https://github.com/khaled71612000/BoidsCpp/assets/59780800/7af14b98-7cc8-4631-be4c-fa87049b2fb8" width="48%" />
</p>

---

## Getting Started

**Prerequisites:** Unreal Engine 5.x, Visual Studio 2022 (Desktop C++), Git LFS

```bash
git lfs install
git clone https://github.com/khaled71612000/BoidsCpp.git
```

1. Right-click `BoidsCpp.uproject` → **Generate Visual Studio project files**
2. Open `.sln` → Build (**Development Editor | Win64**)
3. Launch via Unreal Editor
4. Tune everything in **Project Settings → Flock**

---

## References

- [Boids Algorithm — Craig Reynolds](https://www.red3d.com/cwr/boids/)
- [Steering Behaviors for Autonomous Characters](https://www.red3d.com/cwr/steer/gdc99/)