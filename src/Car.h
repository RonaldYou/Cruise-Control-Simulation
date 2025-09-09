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
            double maxEngineForce);

        explicit Car();
};

#endif // CAR_H