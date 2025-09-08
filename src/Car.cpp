#ifndef CAR_H
#define CAR_H

#include "Vehicle.h"
#include "Constants.h"

class Car : public Vehicle {
    public:
        explicit Car(double mass, 
            double dragCoef, 
            double frontalArea, 
            double rollingResistanceCoef, 
            double maxEngineForce)
            : Vehicle(mass, dragCoef, frontalArea, rollingResistanceCoef, maxEngineForce) {}

        explicit Car() : Vehicle(
            SedanConstants::KERB_WEIGHT,
            SedanConstants::DRAG_COEFFICIENT,
            SedanConstants::FRONTAL_AREA,
            SedanConstants::ROLLING_RESISTANCE_COEFFICIENT,
            SedanConstants::MAX_ENGINE_FORCE) {}
};

#endif // CAR_H