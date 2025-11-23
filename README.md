![9](https://github.com/khaled71612000/BoidsCpp/assets/59780800/b57b7711-6e21-420b-9a0f-106fb45c7a14)

# BoidsCpp

Small Unreal Engine C++ prototype that simulates classic **boid flocking** in a `UWorldSubsystem`, with optional **data-oriented** simulation and simple **gameplay influences** (rings, volumes, world bounds).

![download (1)](https://github.com/user-attachments/assets/be15899a-0790-4127-8469-e495da572a85)

---

## Overview

- Central flock manager (`UFlockSubsystem`)
- Boid actors (`ABoid`) and optional data-boids (`FBoidData` + ISM)
- Spawner volumes and rings to shape flock behavior

---

## Key Components

### `UFlockSubsystem` (World Subsystem)

- Stores all boids (actor and data) and updates them every tick  
- Uses a spatial tree to find neighbors efficiently  
- Optional world bounds from a `UBoxComponent` volume

### `ABoid` (Actor Boid)

- Single boid actor with a skeletal mesh  
- Reads neighbors from the subsystem and applies forces  
- Tweakable settings: speed range, perception radius, rotation smoothing  
- Flags to disable Z, alignment, separation, cohesion if needed

### `ABoidVolumeSpawner`

- Box volume that spawns a configurable number of boids  
- Lets you choose boid class and behavior flags  
- Optional data-oriented path using a `UInstancedStaticMeshComponent`

### `ABoidRing`

- Simple influence actor that affects nearby boids  
- Can encourage boids to pass through, steer around, or swirl  
- Optional launch boost when boids enter an inner radius

### `UFlockDeveloperSettings`

- Project settings page under **Project Settings → Flock**  
- Controls flock mode (actors / data), neighbor caps, and tree detail

---

## Features

- Classic flocking rules (separation / alignment / cohesion)  
- Centralized simulation in a `UWorldSubsystem`  
- Actor or data-oriented simulation paths  
- Level setup via spawners and rings, no custom code needed  
- Optional world bounds so boids stay inside a defined space  

---

## Performance

- With collision enabled and no spatial partitioning:  
  <img width="1189" height="525" alt="7502c911-bd1c-4e61-b538-ce12a3365ed5" src="https://github.com/user-attachments/assets/f5374c7c-022c-4944-a00e-d5a7faff9398" />

- With octree based neighbor queries:  
  <img width="1051" height="378" alt="77cfaf43-272a-4bc0-8a96-0576d6726bb6" src="https://github.com/user-attachments/assets/12589508-ee3c-48c3-b974-26063859d7fc" />

Right now it supports ISM rendering **15–18k** boid isntance on a high end CPU at ~60 FPS (depending on settings and debug options).
<img width="1056" height="509" alt="image" src="https://github.com/user-attachments/assets/d606400a-04e1-421a-9cf1-7e3fd6a41b55" />

---
## References

- [Boids Algorithm – Craig Reynolds](https://www.red3d.com/cwr/boids/)  
- [Steering Behaviors for Autonomous Characters](https://www.red3d.com/cwr/steer/gdc99/)  
- [YouTube Demo](https://youtu.be/L2dsAzmNYwI?si=cwHMqw6KVNRLABKw)

---

## Screenshots

![image](https://github.com/khaled71612000/BoidsCpp/assets/59780800/5ff6f549-8c8e-470d-918d-90dbfa3d03b3)  
![image](https://github.com/khaled71612000/BoidsCpp/assets/59780800/a1cf2645-6f31-45e0-bfef-9e69aa942d4a)  
![image](https://github.com/khaled71612000/BoidsCpp/assets/59780800/dd67af29-e111-4c67-8b7f-2e8b4a3cc908)  
![image](https://github.com/khaled71612000/BoidsCpp/assets/59780800/7af14b98-7cc8-4631-be4c-fa87049b2fb8)
