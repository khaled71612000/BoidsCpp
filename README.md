# BoidsCpp

**BoidsCpp** is a project developed using C++ to simulate the behavior of boids (bird-like objects) using the Boids algorithm. This project focuses on implementing flocking behavior, including alignment, cohesion, and separation, to create realistic group movements.

## Key Components

### Source

#### Boid.cpp
- **Flocking Behavior:** Implements the core flocking algorithms for alignment, cohesion, and separation.
- **Movement Logic:** Manages the movement and interaction of individual boids.
- **Influence Handling:** Boids can be influenced by environmental factors such as rings.
- **Velocity and Acceleration:** Manages boid movement by applying forces and updating velocity and acceleration.
- **Bounds Checking:** Ensures boids stay within the defined simulation area, reversing velocity if they attempt to leave the bounds.
- **Rotation Update:** Adjusts boid orientation based on movement direction for realistic behavior.

#### BoidRing.cpp
- **Ring Influence:** Manages the influence of rings on boids, determining whether boids pass through or go around the rings.
- **Probability and Duration:** Sets the probability of influence and the duration of the effect on boids.

#### BoidVolumeSpawner.cpp
- **Spawner Setup:** Initializes the spawning volume and sets up spawning parameters.
- **Boid Spawning:** Handles the spawning of boids within a defined volume, ensuring they are added to the flock subsystem.

## Features

- **Realistic Flocking:** Simulates flocking behavior with alignment, cohesion, and separation.
- **Dynamic Interaction:** Boids interact dynamically with each other and environmental elements like rings.
- **Configurable Parameters:** Allows customization of simulation parameters through configuration files.
- **Environmental Influence:** Boids can be influenced by external factors, adding complexity to the simulation.

## References
- Craig Reynolds' Boids: [Boids Algorithm](https://www.red3d.com/cwr/boids/)
- Steering Behaviors: [Steering Behaviors For Autonomous Characters](https://www.red3d.com/cwr/steer/gdc99/)

This project serves as an excellent example for developers interested in exploring AI and simulation techniques using C++.
[YouTube Video](https://youtu.be/L2dsAzmNYwI?si=cwHMqw6KVNRLABKw)

![9](https://github.com/khaled71612000/BoidsCpp/assets/59780800/b57b7711-6e21-420b-9a0f-106fb45c7a14)
![image](https://github.com/khaled71612000/BoidsCpp/assets/59780800/5ff6f549-8c8e-470d-918d-90dbfa3d03b3)
![image](https://github.com/khaled71612000/BoidsCpp/assets/59780800/a1cf2645-6f31-45e0-bfef-9e69aa942d4a)
![image](https://github.com/khaled71612000/BoidsCpp/assets/59780800/dd67af29-e111-4c67-8b7f-2e8b4a3cc908)
![image](https://github.com/khaled71612000/BoidsCpp/assets/59780800/7af14b98-7cc8-4631-be4c-fa87049b2fb8)
