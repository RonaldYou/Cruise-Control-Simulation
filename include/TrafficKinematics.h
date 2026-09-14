#ifndef TRAFFICKINEMATICS_H
#define TRAFFICKINEMATICS_H

#include <vector>

namespace traffic {

struct Position {
    double x_m = 0.0;
    double y_m = 0.0;
    double z_m = 0.0;
};

struct Velocity {
    double x_mps = 0.0;
    double y_mps = 0.0;
    double z_mps = 0.0;
};

struct Acceleration {
    double x_mps2 = 0.0;
    double y_mps2 = 0.0;
    double z_mps2 = 0.0;
};

struct Heading {
    double yaw_rad = 0.0;
};

struct KinematicState {
    Position position;
    Velocity velocity;
    Acceleration acceleration;
    Heading heading;
};

struct VehicleSnapshot {
    int id = -1;
    int lane = 0;
    double length_m = 4.5;
    double width_m = 1.8;
    KinematicState kinematics;
};

using TrafficScene = std::vector<VehicleSnapshot>;

}  // namespace traffic

#endif  // TRAFFICKINEMATICS_H
