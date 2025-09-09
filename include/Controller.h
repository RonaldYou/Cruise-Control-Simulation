#ifndef CONTROLLER_H
#define CONTROLLER_H

class Controller{
    public:
        virtual double compute(double setpoint, double measurement, double dt) = 0;

        virtual ~Controller() = default;
};

#endif // CONTROLLER_H