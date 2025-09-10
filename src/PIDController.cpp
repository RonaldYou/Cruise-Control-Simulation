#include "PIDController.h"
#include <algorithm>

PIDController::PIDController(double kp, double ki, double kd)
    : kp_(kp), ki_(ki), kd_(kd), integral_(0.0), prevError_(0.0) {}

double PIDController::compute(double setpoint, double measurement, double dt) {
    double error = setpoint - measurement;
    double derivative = (error - prevError_) / dt;
    prevError_ = error;
    double unsat_u = kp_ * error + ki_ * integral_ + kd_ * derivative;
    double u =std::clamp(unsat_u, 0.0, 1.0);
    if (u == unsat_u) {
        integral_ += error * dt;               // only integrate when not saturated
    }
    return u;
}
