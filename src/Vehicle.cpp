#include "Vehicle.h"
#include "Constants.h"
#include <stdexcept>
#include <algorithm>
/**
 * @brief Constructs a Vehicle with specified physical parameters
 * @param mass Vehicle mass in kilograms (must be positive)
 * @param dragCoef Aerodynamic drag coefficient (dimensionless, non-negative)
 * @param frontalArea Vehicle frontal area in square meters (must be positive)
 * @param rollingResistanceCoef Rolling resistance coefficient (dimensionless, non-negative)
 * @param maxEngineForce Maximum engine force in Newtons (must be non-negative)
 * @throws std::invalid_argument If any parameter is invalid (negative mass, etc.)
 */
Vehicle::Vehicle(double mass, 
                 double dragCoef, 
                 double frontalArea, 
                 double rollingResistanceCoef, 
                 double maxEngineForce)
               : speed_(0.0), 
                 mass_(mass), 
                 dragCoef_(dragCoef), 
                 frontalArea_(frontalArea),
                 rollingResistanceCoef_(rollingResistanceCoef), 
                 maxEngineForce_(maxEngineForce) {

    if (mass_ <= 0) {
        throw std::invalid_argument("Mass must be positive");
    }
    if (dragCoef_ < 0) {
        throw std::invalid_argument("Drag coefficient must be non-negative");
    }
    if (frontalArea_ <= 0) {
        throw std::invalid_argument("Frontal area must be positive");
    }
    if (rollingResistanceCoef_ < 0) {
        throw std::invalid_argument("Rolling resistance coefficient must be non-negative");
    }
    if (maxEngineForce_ < 0) {
        throw std::invalid_argument("Max engine force must be non-negative");
    }
}


/**
 * @brief Updates vehicle physics simulation for one time step
 * 
 * Calculates engine force, drag force, and rolling resistance to determine
 * net force and resulting acceleration. Updates vehicle speed accordingly.
 * Speed is clamped to non-negative values (vehicles cannot go backwards).
 * 
 * @param throttle Throttle input from 0.0 (no throttle) to 1.0 (full throttle)
 * @param dt Time step in seconds (should be positive and small for accuracy)
 * 
 * @note Uses standard aerodynamic drag equation: F_drag = 0.5 * ρ * Cd * A * v²
 * @note Rolling resistance: F_roll = Cr * m * g
 */
void Vehicle::update(double throttle, double dt) {
    double F_engine = throttle * maxEngineForce;
    double F_drag = 0.5 * PhysicsConstants::RHO * dragCoef * frontalArea * speed * speed;
    double F_roll = rollingResistanceCoef * mass * PhysicsConstants::GRAVITY;
    double F_net = F_engine - F_drag - F_roll;
    double acceleration = F_net / mass;
    speed_ = std::max(0.0, speed_ + acceleration * dt);
}doxdox