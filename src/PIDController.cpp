#include "PIDController.h"
#include <algorithm>

PIDController::PIDController(double kp, double ki, double kd)
    : kp_(kp), ki_(ki), kd_(kd), integral_(0.0), prevError_(0.0) {}

double PIDController::compute(double setpoint, double measurement, double dt) {
    double error = setpoint - measurement;
    integral_ += error * dt;
    double derivative = (error - prevError_) / dt;
    prevError_ = error;
    return std::clamp(kp_ * error + ki_ * integral_ + kd_ * derivative, 0.0, 1.0);
}
