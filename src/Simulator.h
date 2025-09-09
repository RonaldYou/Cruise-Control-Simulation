#ifndef SIMULATOR_H
#define SIMULATOR_H

#include "Controller.h"
#include "Constants.h"

class Simulator {
    private:
        Vehicle* vehicle_;
        Controller* controller_;
        double targetSpeed_;
        double dt = SimulationConstants::TIME_STEP;
        double time = 0;
    public:
        Simulator(Vehicle* vehicle, Controller* controller);
        void run();
        void log(double time, double speed, double throttle);
};

#endif // SIMULATOR_H