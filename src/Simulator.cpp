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
#include <algorithm>
#include <random>

/* =============================================================================
 * Constructor - Initialize simulation and renderer
 * ============================================================================= */
Simulator::Simulator(Vehicle* vehicle, Controller* controller)
    : vehicle_(vehicle)
    , controller_(controller)
    , targetSpeed_(0.0)
    , carPositionZ_(0.0)
    , carPositionX_(-3.0)
    , egoLane_(0)
    , egoLaneChangeTarget_(-1)
    , rng_(std::random_device{}())
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

    traffic::TrafficTuningConfig tuning;
    tuning.desired_cruise_speed_mps = targetSpeed_;
    tuning.follow_entry_distance_m = 45.0;
    tuning.desired_time_gap_s = 1.8;
    tuning.critical_ttc_s = 1.4;
    egoTrafficController_ = std::make_unique<traffic::TrafficFSMController>(tuning);

    initializeTraffic();
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

        traffic::TrafficContext egoContext;
        egoContext.ego = buildEgoSnapshot(speed);
        egoContext.traffic = buildTrafficScene();
        egoContext.dt_s = dt_;
        egoContext.min_lane = 0;
        egoContext.max_lane = 2;
        egoContext.spatial_config.max_detection_distance_m = 120.0;
        egoContext.spatial_config.min_front_gap_m = 14.0;
        egoContext.spatial_config.min_rear_gap_m = 10.0;

        const traffic::ControlCommand egoCommand = egoTrafficController_->step(egoContext);
        const double throttle = egoCommand.throttle;
        const double brake = egoCommand.brake;

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
        vehicle_->update(throttle, brake, dt_, terrainForce);
        updateEgoLanePosition(egoCommand);

        speed = vehicle_->getSpeed();

        /* Update car position (integrate velocity)
         *
         * For visualization, we track the car's Z position (forward distance).
         * The car stays at X=0, Y=0 in the world, moving forward along +Z.
         */
        double distanceTraveled = speed * dt_;
        carPositionZ_ += distanceTraveled;

        /* Update background traffic vehicles */
        updateTraffic();

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
        glm::vec3 carPos(static_cast<float>(carPositionX_),
                 static_cast<float>(currentElevation_),
                 static_cast<float>(carPositionZ_));

        /* Render the scene */
        renderer_->render(carPos,
                          static_cast<float>(speed),
                          static_cast<float>(targetSpeed_),
                          static_cast<float>(throttle - brake),
                          static_cast<float>(grade),
                          elevationHistory_,
                          traffic_);

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

void Simulator::initializeTraffic() {
    traffic_.clear();

    std::uniform_int_distribution<int> laneDist(0, 2);
    std::uniform_real_distribution<double> speedOffsetDist(-2.0, 1.0);

    for (int i = 0; i < TrafficConstants::NUM_CARS; ++i) {
        TrafficVehicle vehicle;
        vehicle.positionZ = carPositionZ_ + TrafficConstants::SPAWN_DISTANCE_AHEAD + (i * 18.0);
        vehicle.lane = laneDist(rng_);
        vehicle.speed = std::max(1.0, targetSpeed_ + speedOffsetDist(rng_));
        traffic_.push_back(vehicle);
    }

    if (!traffic_.empty()) {
        traffic_[0].lane = egoLane_;
        traffic_[0].positionZ = carPositionZ_ + 30.0;
        traffic_[0].speed = std::max(1.0, targetSpeed_ - 7.0);
    }

    if (traffic_.size() > 1) {
        traffic_[1].lane = 1;
        traffic_[1].positionZ = carPositionZ_ + 90.0;
        traffic_[1].speed = targetSpeed_ + 1.0;
    }
}

void Simulator::updateTraffic() {
    std::uniform_int_distribution<int> laneDist(0, 2);
    std::uniform_real_distribution<double> speedOffsetDist(-3.0, 2.0);

    for (TrafficVehicle& vehicle : traffic_) {
        vehicle.positionZ += vehicle.speed * dt_;

        const double relativeZ = vehicle.positionZ - carPositionZ_;
        if (relativeZ < TrafficConstants::DESPAWN_DISTANCE_BEHIND) {
            vehicle.positionZ = carPositionZ_ + TrafficConstants::SPAWN_DISTANCE_AHEAD + 50.0;
            vehicle.lane = laneDist(rng_);
            vehicle.speed = std::max(1.0, targetSpeed_ + speedOffsetDist(rng_));
        }
    }
}

traffic::TrafficScene Simulator::buildTrafficScene() const {
    traffic::TrafficScene scene;
    scene.reserve(traffic_.size());

    for (size_t index = 0; index < traffic_.size(); ++index) {
        traffic::VehicleSnapshot snapshot;
        snapshot.id = static_cast<int>(index + 1);
        snapshot.lane = traffic_[index].lane;
        snapshot.kinematics.position.x_m = laneCenterX(traffic_[index].lane);
        snapshot.kinematics.position.y_m = 0.0;
        snapshot.kinematics.position.z_m = traffic_[index].positionZ;
        snapshot.kinematics.velocity.z_mps = traffic_[index].speed;
        scene.push_back(snapshot);
    }

    return scene;
}

traffic::VehicleSnapshot Simulator::buildEgoSnapshot(double speed) const {
    traffic::VehicleSnapshot ego;
    ego.id = 0;
    ego.lane = egoLane_;
    ego.kinematics.position.x_m = carPositionX_;
    ego.kinematics.position.y_m = 0.0;
    ego.kinematics.position.z_m = carPositionZ_;
    ego.kinematics.velocity.z_mps = speed;
    return ego;
}

double Simulator::laneCenterX(int lane) const {
    switch (lane) {
        case 0:
            return -3.0;
        case 1:
            return 0.0;
        case 2:
            return 3.0;
        default:
            return -3.0;
    }
}

void Simulator::updateEgoLanePosition(const traffic::ControlCommand& command) {
    if (command.initiate_lane_change) {
        egoLaneChangeTarget_ = command.desired_lane;
    }

    if (egoLaneChangeTarget_ < 0) {
        return;
    }

    const double targetX = laneCenterX(egoLaneChangeTarget_);
    const double maxStep = std::abs(command.lateral_velocity_mps) > 0.0
        ? std::abs(command.lateral_velocity_mps) * dt_
        : 1.0 * dt_;
    const double deltaX = targetX - carPositionX_;

    if (std::abs(deltaX) <= maxStep) {
        carPositionX_ = targetX;
        egoLane_ = egoLaneChangeTarget_;
        egoLaneChangeTarget_ = -1;
        return;
    }

    carPositionX_ += (deltaX > 0.0 ? 1.0 : -1.0) * maxStep;
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
