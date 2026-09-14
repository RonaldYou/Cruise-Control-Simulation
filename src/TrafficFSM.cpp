#include "TrafficFSM.h"

#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace traffic {

namespace {

double clamp(double value, double low, double high) {
    return std::max(low, std::min(value, high));
}

void map_acceleration_to_command(
    double accel_mps2,
    const TrafficTuningConfig& tuning,
    ControlCommand& command) {
    if (accel_mps2 >= 0.0) {
        command.throttle = clamp(accel_mps2 / tuning.max_accel_mps2, 0.0, 1.0);
        command.brake = 0.0;
        return;
    }

    command.throttle = 0.0;
    command.brake = clamp(-accel_mps2 / tuning.max_brake_mps2, 0.0, 1.0);
}

}  // namespace

PIDLoop::PIDLoop(double kp, double ki, double kd)
    : kp_(kp)
    , ki_(ki)
    , kd_(kd)
    , integral_(0.0)
    , prev_error_(0.0)
    , has_prev_(false) {
}

double PIDLoop::compute(double error, double dt_s) {
    if (dt_s <= 1e-6) {
        return 0.0;
    }

    integral_ += error * dt_s;
    const double derivative = has_prev_ ? (error - prev_error_) / dt_s : 0.0;
    prev_error_ = error;
    has_prev_ = true;

    return (kp_ * error) + (ki_ * integral_) + (kd_ * derivative);
}

void PIDLoop::reset() {
    integral_ = 0.0;
    prev_error_ = 0.0;
    has_prev_ = false;
}

void ITrafficState::on_enter(const TrafficContext& /*context*/) {
}

void ITrafficState::on_exit(const TrafficContext& /*context*/) {
}

CruiseState::CruiseState(const TrafficTuningConfig& tuning)
    : speed_pid_(tuning.cruise_kp, tuning.cruise_ki, tuning.cruise_kd) {
}

TrafficStateId CruiseState::id() const {
    return TrafficStateId::Cruise;
}

void CruiseState::on_enter(const TrafficContext& /*context*/) {
    speed_pid_.reset();
}

TrafficStateId CruiseState::update(
    const TrafficContext& context,
    const SpatialAwarenessState& awareness,
    ControlCommand& command) {
    if (awareness.lead_same_lane.has_value()) {
        const LeadObservation& lead = *awareness.lead_same_lane;
        if (lead.ttc_s < context.tuning.critical_ttc_s) {
            command.throttle = 0.0;
            command.brake = 1.0;
            return TrafficStateId::EmergencyBrake;
        }

        if (lead.distance_m <= context.tuning.follow_entry_distance_m) {
            return TrafficStateId::AdaptiveFollow;
        }
    }

    const double speed_error = context.tuning.desired_cruise_speed_mps - context.ego.kinematics.velocity.z_mps;
    const double requested_accel = speed_pid_.compute(speed_error, context.dt_s);
    map_acceleration_to_command(requested_accel, context.tuning, command);
    return TrafficStateId::Cruise;
}

AdaptiveFollowState::AdaptiveFollowState(const TrafficTuningConfig& tuning)
    : gap_pid_(tuning.follow_kp, tuning.follow_ki, tuning.follow_kd) {
}

TrafficStateId AdaptiveFollowState::id() const {
    return TrafficStateId::AdaptiveFollow;
}

void AdaptiveFollowState::on_enter(const TrafficContext& /*context*/) {
    gap_pid_.reset();
}

TrafficStateId AdaptiveFollowState::update(
    const TrafficContext& context,
    const SpatialAwarenessState& awareness,
    ControlCommand& command) {
    if (!awareness.lead_same_lane.has_value()) {
        return TrafficStateId::Cruise;
    }

    const LeadObservation& lead = *awareness.lead_same_lane;
    if (lead.ttc_s < context.tuning.critical_ttc_s) {
        command.throttle = 0.0;
        command.brake = 1.0;
        return TrafficStateId::EmergencyBrake;
    }

    const double ego_speed = context.ego.kinematics.velocity.z_mps;
    const double desired_gap = context.tuning.min_follow_distance_m
        + (context.tuning.desired_time_gap_s * std::max(0.0, ego_speed));

    const double gap_error = lead.distance_m - desired_gap;
    const double pid_term = gap_pid_.compute(gap_error, context.dt_s);
    const double damping_term = context.tuning.relative_speed_gain * lead.relative_speed_mps;
    const double requested_accel = pid_term + damping_term;

    const bool speed_restricted =
        lead.distance_m < (desired_gap * 1.1)
        && lead.vehicle.kinematics.velocity.z_mps
            < (context.tuning.desired_cruise_speed_mps - context.tuning.lane_change_speed_block_delta_mps);

    if (speed_restricted) {
        const int left_lane = context.ego.lane - 1;
        const int right_lane = context.ego.lane + 1;

        if (left_lane >= context.min_lane
            && is_lane_gap_safe(awareness.left_gap, context.spatial_config)) {
            command.initiate_lane_change = true;
            command.desired_lane = left_lane;
            command.lateral_velocity_mps = -context.tuning.lane_change_lateral_speed_mps;
            return TrafficStateId::LaneChange;
        }

        if (right_lane <= context.max_lane
            && is_lane_gap_safe(awareness.right_gap, context.spatial_config)) {
            command.initiate_lane_change = true;
            command.desired_lane = right_lane;
            command.lateral_velocity_mps = context.tuning.lane_change_lateral_speed_mps;
            return TrafficStateId::LaneChange;
        }
    }

    if (lead.distance_m > (context.tuning.follow_entry_distance_m * 1.5) && gap_error > 0.0) {
        return TrafficStateId::Cruise;
    }

    map_acceleration_to_command(requested_accel, context.tuning, command);
    return TrafficStateId::AdaptiveFollow;
}

TrafficStateId LaneChangeState::id() const {
    return TrafficStateId::LaneChange;
}

TrafficStateId LaneChangeState::update(
    const TrafficContext& context,
    const SpatialAwarenessState& awareness,
    ControlCommand& command) {
    if (context.active_lane_change_target < context.min_lane
        || context.active_lane_change_target > context.max_lane) {
        return TrafficStateId::Cruise;
    }

    command.initiate_lane_change = true;
    command.desired_lane = context.active_lane_change_target;
    const int direction = (command.desired_lane > context.ego.lane) ? 1 : -1;
    command.lateral_velocity_mps = direction * context.tuning.lane_change_lateral_speed_mps;

    if (context.ego.lane == command.desired_lane) {
        return awareness.lead_same_lane.has_value()
            ? TrafficStateId::AdaptiveFollow
            : TrafficStateId::Cruise;
    }

    return TrafficStateId::LaneChange;
}

TrafficStateId EmergencyBrakeState::id() const {
    return TrafficStateId::EmergencyBrake;
}

TrafficStateId EmergencyBrakeState::update(
    const TrafficContext& context,
    const SpatialAwarenessState& awareness,
    ControlCommand& command) {
    command.throttle = 0.0;
    command.brake = 1.0;

    if (!awareness.lead_same_lane.has_value()) {
        return TrafficStateId::Cruise;
    }

    const LeadObservation& lead = *awareness.lead_same_lane;
    if (lead.ttc_s > context.tuning.emergency_release_ttc_s
        && lead.distance_m > context.tuning.min_follow_distance_m) {
        return TrafficStateId::AdaptiveFollow;
    }

    return TrafficStateId::EmergencyBrake;
}

TrafficFSMController::TrafficFSMController(const TrafficTuningConfig& tuning)
    : tuning_(tuning) {
    states_.emplace(TrafficStateId::Cruise, std::make_unique<CruiseState>(tuning_));
    states_.emplace(TrafficStateId::AdaptiveFollow, std::make_unique<AdaptiveFollowState>(tuning_));
    states_.emplace(TrafficStateId::LaneChange, std::make_unique<LaneChangeState>());
    states_.emplace(TrafficStateId::EmergencyBrake, std::make_unique<EmergencyBrakeState>());
}

ControlCommand TrafficFSMController::step(const TrafficContext& context) {
    TrafficContext state_context = context;
    state_context.tuning = tuning_;
    state_context.active_lane_change_target = active_lane_target_;

    latest_awareness_ = build_spatial_awareness(
        state_context.ego,
        state_context.traffic,
        state_context.spatial_config);

    auto state_it = states_.find(current_state_id_);
    if (state_it == states_.end()) {
        throw std::runtime_error("FSM state not registered");
    }

    ControlCommand command;
    const TrafficStateId next_state = state_it->second->update(state_context, latest_awareness_, command);

    if (command.initiate_lane_change && command.desired_lane >= state_context.min_lane) {
        active_lane_target_ = command.desired_lane;
    }

    if (state_context.ego.lane == active_lane_target_) {
        active_lane_target_ = -1;
    }

    if (next_state != current_state_id_) {
        transition_to(next_state, state_context);
    }

    return command;
}

TrafficStateId TrafficFSMController::current_state() const {
    return current_state_id_;
}

const SpatialAwarenessState& TrafficFSMController::latest_awareness() const {
    return latest_awareness_;
}

void TrafficFSMController::transition_to(
    TrafficStateId next_state,
    const TrafficContext& context) {
    auto current_it = states_.find(current_state_id_);
    if (current_it != states_.end()) {
        current_it->second->on_exit(context);
    }

    current_state_id_ = next_state;
    auto next_it = states_.find(current_state_id_);
    if (next_it == states_.end()) {
        throw std::runtime_error("Transition target state not registered");
    }

    next_it->second->on_enter(context);
}

}  // namespace traffic
