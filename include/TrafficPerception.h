#ifndef TRAFFICPERCEPTION_H
#define TRAFFICPERCEPTION_H

#include "TrafficKinematics.h"
#include <optional>

namespace traffic {

struct LeadObservation {
    VehicleSnapshot vehicle;
    double distance_m = 0.0;
    double relative_speed_mps = 0.0;
    double ttc_s = 1e9;
};

struct LaneGapObservation {
    int lane = -1;
    bool occupied = false;
    double nearest_front_gap_m = 1e9;
    double nearest_rear_gap_m = 1e9;
};

struct SpatialAwarenessConfig {
    double max_detection_distance_m = 150.0;
    double min_front_gap_m = 15.0;
    double min_rear_gap_m = 10.0;
};

struct SpatialAwarenessState {
    std::optional<LeadObservation> lead_same_lane;
    LaneGapObservation left_gap;
    LaneGapObservation right_gap;
};

std::optional<VehicleSnapshot> find_lead_vehicle(
    const VehicleSnapshot& ego,
    const TrafficScene& traffic,
    int lane,
    double max_detection_distance_m);

double get_distance_to_lead_vehicle(
    const VehicleSnapshot& ego,
    const VehicleSnapshot& lead);

double get_relative_velocity_to_lead_vehicle(
    const VehicleSnapshot& ego,
    const VehicleSnapshot& lead);

double compute_ttc_seconds(double distance_m, double relative_speed_mps);

LaneGapObservation evaluate_lane_gap(
    const VehicleSnapshot& ego,
    const TrafficScene& traffic,
    int lane,
    double max_detection_distance_m);

bool is_lane_gap_safe(
    const LaneGapObservation& gap,
    const SpatialAwarenessConfig& config);

SpatialAwarenessState build_spatial_awareness(
    const VehicleSnapshot& ego,
    const TrafficScene& traffic,
    const SpatialAwarenessConfig& config);

}  // namespace traffic

#endif  // TRAFFICPERCEPTION_H
