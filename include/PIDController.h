#ifndef PIDCONTROLLER_H
#define PIDCONTROLLER_H

#include "Controller.h"

class PIDController : public Controller {
    private:
        double kp_;
        double ki_;
        double kd_;
        double integral_;
        double prevError_;
    public:
        PIDController(double kp, double ki, double kd);
        double compute(double setpoint, double measurement, double dt) override;
};

#endif // PIDCONTROLLER_H