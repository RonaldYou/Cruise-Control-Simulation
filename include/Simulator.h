/*
 * Simulator.h - Main Simulation Controller
 *
 * Coordinates the physics simulation and rendering:
 * - Updates vehicle physics each timestep
 * - Computes cruise control throttle via PID controller
 * - Simulates terrain/grade changes
 * - Renders the scene via OpenGL
 */

#ifndef SIMULATOR_H
#define SIMULATOR_H

#include "Controller.h"
#include "Constants.h"
#include "Vehicle.h"
#include "Terrain.h"
#include "Renderer.h"
#include <memory>

class Simulator {
public:
    /* =========================================================================
     * Constructor
     *
     * Parameters:
     *   vehicle    - The vehicle to simulate (owns physics state)
     *   controller - The cruise control algorithm (PID)
     * ========================================================================= */
    Simulator(Vehicle* vehicle, Controller* controller);

    /* Destructor */
    ~Simulator() = default;

    /* =========================================================================
     * run() - Main simulation loop
     *
     * Runs until the window is closed. Each iteration:
     * 1. Updates physics (vehicle, terrain)
     * 2. Computes control input
     * 3. Renders the frame
     * ========================================================================= */
    void run();

    /* Log data to console/file (for debugging) */
    void log(double time, double speed, double throttle, bool logFile);

private:
    Vehicle* vehicle_;         /* Vehicle being simulated (not owned) */
    Controller* controller_;   /* Cruise control algorithm (not owned) */
    double targetSpeed_;       /* Desired cruise speed (m/s) */
    double dt_ = SimulationConstants::TIME_STEP;  /* Physics timestep */
    double time_ = 0.0;        /* Total simulation time */

    /* Renderer for OpenGL display */
    std::unique_ptr<Renderer> renderer_;

    /* Car's position in 3D space (Z = forward distance traveled) */
    double carPositionZ_ = 0.0;
};

#endif // SIMULATOR_H