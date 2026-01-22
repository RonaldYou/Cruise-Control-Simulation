/*
 * Simulator.cpp - Main Simulation Loop Implementation
 */

#include "Simulator.h"
#include "Constants.h"
#include <glm/glm.hpp>
#include <iostream>
#include <fstream>
#include <thread>
#include <chrono>

/* =============================================================================
 * Constructor - Initialize simulation and renderer
 * ============================================================================= */
Simulator::Simulator(Vehicle* vehicle, Controller* controller)
    : vehicle_(vehicle)
    , controller_(controller)
    , targetSpeed_(0.0)
    , carPositionZ_(0.0)
{
    /* Get target speed from user (in km/h, converted to m/s internally) */
    double targetKmh;
    std::cout << "What is the target speed (km/h)? ";
    std::cin >> targetKmh;
    targetSpeed_ = targetKmh / 3.6;  /* Convert km/h to m/s */

    /* Create the OpenGL renderer
     *
     * Window size: 1280x720 (720p)
     * This also initializes GLFW and OpenGL context
     */
    renderer_ = std::make_unique<Renderer>(1280, 720, "Cruise Control Simulation");
}

/* =============================================================================
 * run() - Main simulation loop
 *
 * This loop runs until the window is closed. Each iteration:
 * 1. Physics update (fixed timestep)
 * 2. Render the current state
 *
 * Note: For simplicity, we're using a fixed timestep for both physics and
 * rendering. A more sophisticated approach would decouple them (fixed
 * physics timestep, variable render rate).
 * ============================================================================= */
void Simulator::run() {
    Terrain terrain;

    /*
     * Main loop - runs until user closes the window
     *
     * GLFW tracks window close requests (clicking X, pressing Alt+F4, etc.)
     * renderer_->shouldClose() checks this flag.
     */
    while (!renderer_->shouldClose()) {
        /* Update terrain (randomly changes grade over time) */
        terrain.update(dt_);

        /* Get current vehicle speed */
        double speed = vehicle_->getSpeed();

        /* Compute throttle from cruise control (PID controller)
         *
         * The controller compares target speed to actual speed and
         * outputs a throttle value (0.0 to 1.0) to minimize the error.
         */
        double throttle = controller_->compute(targetSpeed_, speed, dt_);

        /* Calculate force from terrain grade
         *
         * Uphill (positive grade): negative force (resists motion)
         * Downhill (negative grade): positive force (assists motion)
         */
        double terrainForce = terrain.getGradeForce(vehicle_->getMass());

        /* Update vehicle physics
         *
         * This applies throttle and terrain forces to compute new velocity.
         */
        vehicle_->update(throttle, dt_, terrainForce);

        /* Update car position (integrate velocity)
         *
         * For visualization, we track the car's Z position (forward distance).
         * The car stays at X=0, Y=0 in the world, moving forward along +Z.
         */
        double distanceTraveled = speed * dt_;
        carPositionZ_ += distanceTraveled;

        /* Update elevation based on current grade
         * elevation change = distance * grade (rise = run * slope) */
        double grade = terrain.getCurrentGrade();
        currentElevation_ += distanceTraveled * grade;

        /* Store elevation history (approximately one point per meter) */
        static double distanceSinceLastPoint = 0.0;
        distanceSinceLastPoint += distanceTraveled;
        if (distanceSinceLastPoint >= 1.0) {
            elevationHistory_.push_back(static_cast<float>(currentElevation_));
            distanceSinceLastPoint = 0.0;

            /* Keep history bounded */
            if (elevationHistory_.size() > MAX_ELEVATION_HISTORY) {
                elevationHistory_.erase(elevationHistory_.begin());
            }
        }

        /* Update simulation time */
        time_ += dt_;

        /* =====================================================================
         * RENDERING
         * ===================================================================== */

        /* Prepare for rendering (clear buffers) */
        renderer_->beginFrame();

        /* Build car position vector for renderer
         *
         * X: -3 (in left lane)
         * Y: current elevation
         * Z: distance traveled (forward)
         */
        glm::vec3 carPos(-3.0f, static_cast<float>(currentElevation_), static_cast<float>(carPositionZ_));

        /* Render the scene */
        renderer_->render(carPos,
                          static_cast<float>(speed),
                          static_cast<float>(targetSpeed_),
                          static_cast<float>(throttle),
                          static_cast<float>(grade),
                          elevationHistory_);

        /* Finish frame (swap buffers, poll events) */
        renderer_->endFrame();

        /* Log to console (optional, for debugging)
         * Commented out to avoid console spam during rendering */
        // log(time_, speed, throttle, false);

        /* Sleep to achieve real-time simulation (1 simulated second = 1 real second)
         *
         * LOOP_DELAY (200ms) matches TIME_STEP (0.2s), so each loop iteration
         * advances 0.2 simulated seconds and waits 0.2 real seconds.
         * This keeps simulation time synchronized with wall-clock time.
         */
        std::this_thread::sleep_for(std::chrono::milliseconds(SimulationConstants::LOOP_DELAY));
    }
}

void Simulator::log(double time, double speed, double throttle, bool logFile){
    std::cout << "Time: " << time << "| Target: " << targetSpeed_ << "| Speed: " << speed << "| Throttle: " << throttle << "     ";
    //std::cout.flush();

    if(logFile){
        std::ofstream Log("../simulation.csv", std::ios::app);
        if(Log.is_open()){
            Log << time << "," << targetSpeed_ << "," << speed << "," << throttle << "\n";
            Log.close();
        }
    }
}
