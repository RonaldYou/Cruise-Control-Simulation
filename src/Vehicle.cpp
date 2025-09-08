#include "Vehicle.h"
#include "Constants.h"
#include <stdexcept>
#include <algorithm>

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

void Vehicle::update(double throttle, double dt) {
    double F_engine = throttle * maxEngineForce;
    double F_drag = 0.5 * PhysicsConstants::RHO * dragCoef * frontalArea * speed * speed;
    double F_roll = rollingResistanceCoef * mass * PhysicsConstants::GRAVITY;
    double F_net = F_engine - F_drag - F_roll;
    double acceleration = F_net / mass;
    speed = std::max(0.0, speed + acceleration * dt);
}
