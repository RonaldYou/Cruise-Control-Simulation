# Cruise-Control-Simulation

This project is a **C++ simulation of an automotive cruise control system**. It models a vehicle's longitudinal dynamics and uses a **PID controller** to maintain a target speed. The simulation is designed as an educational project to explore control systems, object-oriented programming, and software design applied to automotive engineering.

## Features
- **Vehicle dynamics model**: Simulates velocity response to throttle input with mass, drag, and rolling resistance.  
- **PID controller with anti-windup**: Tuned using real-world techniques (Ziegler–Nichols, step tests) with support for anti-windup and rate limiting to ensure stability even under actuator saturation.  
- **Data logging**: Records time, setpoint, velocity, error, and control signals for offline analysis.  
- **Modular design**: Uses C++ classes for Vehicle, Controller, and Simulation, demonstrating principles of inheritance, encapsulation, and clean architecture.  

## How It Works
1. A setpoint (target speed) is defined.  
2. The PID controller compares the setpoint with the vehicle’s current velocity.  
3. Based on proportional, integral, and derivative terms, the controller adjusts throttle input.  
4. The vehicle model updates its velocity using physics equations (F = ma with drag and resistance).  
5. The simulation loop iterates over time, producing logs that can be plotted for analysis.  



## Future Goals

Planned improvements to expand realism and showcase more advanced control behavior:

- **Road slopes**: Add incline/decline forces into the vehicle dynamics to simulate real driving conditions.

- **ASCII display**: Provide a console-based visualization of speed, throttle, and road for quick feedback without external plotting tools.

- **Overtaking cars**: Introduce simulated slower vehicles ahead, requiring dynamic throttle/braking decisions for safe overtaking.

- **Following cars**: Implement adaptive cruise control behavior to maintain safe distances from vehicles in front.

- **Energy efficiency modeling**: Extend the vehicle model to account for fuel or battery usage, useful for EV efficiency analysis.

- **Advanced control strategies**: Compare PID with more modern approaches (e.g., Model Predictive Control) to analyze trade-offs in stability and performance.

Ziegler–Nichols tuning
| Controller | Kp      | Ki          | Kd        |
| ---------- | ------- | ----------- | --------- |
| P          | 0.5 Ku  | –           | –         |
| PI         | 0.45 Ku | 1.2 Kp / Tu | –         |
| PID        | 0.6 Ku  | 2 Kp / Tu   | Kp Tu / 8 |


## Getting Started
### Build
```bash
mkdir build && cd build
cmake ..
make


