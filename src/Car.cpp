#include "Car.h"

Car::Car(double mass, 
         double dragCoef, 
         double frontalArea, 
         double rollingResistanceCoef, 
         double maxEngineForce)
    : Vehicle(mass, dragCoef, frontalArea, rollingResistanceCoef, maxEngineForce) {}

Car::Car() : Vehicle(
    SedanConstants::KERB_WEIGHT,
    SedanConstants::DRAG_COEFFICIENT,
    SedanConstants::FRONTAL_AREA,
    SedanConstants::ROLLING_RESISTANCE_COEFFICIENT,
    SedanConstants::MAX_ENGINE_FORCE) {}
