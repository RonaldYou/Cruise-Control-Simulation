#ifndef CONTROLLER_H
#define CONTROLLER_H

class Controller{
    public:
        virtual double compute(double setpoint, double measurement, double dt) = 0;
};

#endif // CONTROLLER_H