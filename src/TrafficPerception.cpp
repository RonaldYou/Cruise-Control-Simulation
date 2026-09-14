#include "TrafficPerception.h"

#include <algorithm>
#include <limits>

namespace traffic {

namespace {

constexpr double kInfinite = std::numeric_limits<double>::infinity();

}  // namespace

std::optional<VehicleSnapshot> find_lead_vehicle(
    const VehicleSnapshot& ego,
    const TrafficScene& traffic,
    int lane,
    double max_detection_distance_m) {
    std::optional<VehicleSnapshot> lead;
    double nearest_distance = max_detection_distance_m;

    for (const VehicleSnapshot& other : traffic) {
        if (other.id == ego.id || other.lane != lane) {
            continue;
        }

        const double gap = get_distance_to_lead_vehicle(ego, other);
        if (gap <= 0.0 || gap > max_detection_distance_m) {
            continue;
        }

        if (gap < nearest_distance) {
            nearest_distance = gap;
            lead = other;
        }
    }

    return lead;
}

double get_distance_to_lead_vehicle(
    const VehicleSnapshot& ego,
    const VehicleSnapshot& lead) {
    const double ego_front = ego.kinematics.position.z_m + (ego.length_m * 0.5);
    const double lead_rear = lead.kinematics.position.z_m - (lead.length_m * 0.5);
    return lead_rear - ego_front;
}

double get_relative_velocity_to_lead_vehicle(
    const VehicleSnapshot& ego,
    const VehicleSnapshot& lead) {
    return lead.kinematics.velocity.z_mps - ego.kinematics.velocity.z_mps;
}

double compute_ttc_seconds(double distance_m, double relative_speed_mps) {
    // relative_speed_mps is lead - ego; closing occurs when this value is negative.
    const double closing_speed = -relative_speed_mps;
    if (closing_speed <= 1e-6) {
        return kInfinite;
    }

    return std::max(0.0, distance_m) / closing_speed;
}

LaneGapObservation evaluate_lane_gap(
    const VehicleSnapshot& ego,
    const TrafficScene& traffic,
    int lane,
    double max_detection_distance_m) {
    LaneGapObservation gap;
    gap.lane = lane;

    if (lane < 0) {
        gap.occupied = true;
        gap.nearest_front_gap_m = 0.0;
        gap.nearest_rear_gap_m = 0.0;
        return gap;
    }

    double nearest_front = max_detection_distance_m;
    double nearest_rear = max_detection_distance_m;
    bool has_vehicle = false;

    for (const VehicleSnapshot& other : traffic) {
        if (other.id == ego.id || other.lane != lane) {
            continue;
        }

        has_vehicle = true;
        const double delta_z = other.kinematics.position.z_m - ego.kinematics.position.z_m;
        if (delta_z >= 0.0) {
            const double front_gap = get_distance_to_lead_vehicle(ego, other);
            nearest_front = std::min(nearest_front, front_gap);
        } else {
            const double rear_gap = get_distance_to_lead_vehicle(other, ego);
            nearest_rear = std::min(nearest_rear, rear_gap);
        }
    }

    gap.nearest_front_gap_m = has_vehicle ? nearest_front : max_detection_distance_m;
    gap.nearest_rear_gap_m = has_vehicle ? nearest_rear : max_detection_distance_m;
    gap.occupied = has_vehicle;
    return gap;
}

bool is_lane_gap_safe(
    const LaneGapObservation& gap,
    const SpatialAwarenessConfig& config) {
    return gap.nearest_front_gap_m >= config.min_front_gap_m
        && gap.nearest_rear_gap_m >= config.min_rear_gap_m;
}

SpatialAwarenessState build_spatial_awareness(
    const VehicleSnapshot& ego,
    const TrafficScene& traffic,
    const SpatialAwarenessConfig& config) {
    SpatialAwarenessState awareness;

    std::optional<VehicleSnapshot> lead = find_lead_vehicle(
        ego,
        traffic,
        ego.lane,
        config.max_detection_distance_m);

    if (lead.has_value()) {
        LeadObservation lead_obs;
        lead_obs.vehicle = *lead;
        lead_obs.distance_m = get_distance_to_lead_vehicle(ego, *lead);
        lead_obs.relative_speed_mps = get_relative_velocity_to_lead_vehicle(ego, *lead);
        lead_obs.ttc_s = compute_ttc_seconds(lead_obs.distance_m, lead_obs.relative_speed_mps);
        awareness.lead_same_lane = lead_obs;
    }

    awareness.left_gap = evaluate_lane_gap(
        ego,
        traffic,
        ego.lane - 1,
        config.max_detection_distance_m);
    awareness.right_gap = evaluate_lane_gap(
        ego,
        traffic,
        ego.lane + 1,
        config.max_detection_distance_m);

    return awareness;
}

}  // namespace traffic

