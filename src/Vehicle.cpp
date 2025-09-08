#include "Vehicle.h"

// Constructor implementation
Vehicle::Vehicle(double mass, double dragCoef, double frontalArea, 
                 double rollingResistanceCoef, double maxEngineForce)
    : speed(0.0), mass(mass), dragCoef(dragCoef), frontalArea(frontalArea),
      rollingResistanceCoef(rollingResistanceCoef), maxEngineForce(maxEngineForce)) {
}

void Vehicle::update(double throttle, double dt) {
    double F_engine = throttle * maxEngineForce;
    double F_drag = 0.5 * PhysicsConstants::RHO * dragCoef * frontalArea * speed * speed;
    double F_roll = rollingResistanceCoef * mass * PhysicsConstants::GRAVITY;
    double F_net = F_engine - F_drag - F_roll;
    double acceleration = F_net / mass;
    speed += acceleration * dt;
}
