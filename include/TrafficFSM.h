#ifndef TRAFFICFSM_H
#define TRAFFICFSM_H

#include "TrafficPerception.h"
#include <memory>
#include <unordered_map>

namespace traffic {

enum class TrafficStateId {
    Cruise,
    AdaptiveFollow,
    LaneChange,
    EmergencyBrake
};

struct TrafficTuningConfig {
    // Safety distances and TTC thresholds. Increase these for more conservative behavior.
    double desired_cruise_speed_mps = 27.0;
    double min_follow_distance_m = 8.0;
    double desired_time_gap_s = 1.5;
    double follow_entry_distance_m = 40.0;
    double critical_ttc_s = 1.2;
    double emergency_release_ttc_s = 2.5;

    // Lane-change trigger and smoothness controls.
    double lane_change_speed_block_delta_mps = 1.5;
    double lane_change_lateral_speed_mps = 1.0;

    // Longitudinal actuator limits.
    double max_accel_mps2 = 2.5;
    double max_brake_mps2 = 6.0;

    // PID gains for speed hold and following-distance control.
    double cruise_kp = 0.35;
    double cruise_ki = 0.04;
    double cruise_kd = 0.05;
    double follow_kp = 0.22;
    double follow_ki = 0.03;
    double follow_kd = 0.04;

    // Relative speed damping term: higher values react faster to lead deceleration.
    double relative_speed_gain = 0.30;
};

struct ControlCommand {
    double throttle = 0.0;
    double brake = 0.0;
    int desired_lane = -1;
    bool initiate_lane_change = false;
    double lateral_velocity_mps = 0.0;
};

struct TrafficContext {
    VehicleSnapshot ego;
    TrafficScene traffic;
    SpatialAwarenessConfig spatial_config;
    TrafficTuningConfig tuning;
    double dt_s = 0.1;
    int min_lane = 0;
    int max_lane = 2;
    int active_lane_change_target = -1;
};

class PIDLoop {
public:
    PIDLoop(double kp, double ki, double kd);

    double compute(double error, double dt_s);
    void reset();

private:
    double kp_;
    double ki_;
    double kd_;
    double integral_;
    double prev_error_;
    bool has_prev_;
};

class ITrafficState {
public:
    virtual ~ITrafficState() = default;

    virtual TrafficStateId id() const = 0;
    virtual void on_enter(const TrafficContext& context);
    virtual void on_exit(const TrafficContext& context);

    virtual TrafficStateId update(
        const TrafficContext& context,
        const SpatialAwarenessState& awareness,
        ControlCommand& command) = 0;
};

class CruiseState final : public ITrafficState {
public:
    explicit CruiseState(const TrafficTuningConfig& tuning);

    TrafficStateId id() const override;
    void on_enter(const TrafficContext& context) override;
    TrafficStateId update(
        const TrafficContext& context,
        const SpatialAwarenessState& awareness,
        ControlCommand& command) override;

private:
    PIDLoop speed_pid_;
};

class AdaptiveFollowState final : public ITrafficState {
public:
    explicit AdaptiveFollowState(const TrafficTuningConfig& tuning);

    TrafficStateId id() const override;
    void on_enter(const TrafficContext& context) override;
    TrafficStateId update(
        const TrafficContext& context,
        const SpatialAwarenessState& awareness,
        ControlCommand& command) override;

private:
    PIDLoop gap_pid_;
};

class LaneChangeState final : public ITrafficState {
public:
    TrafficStateId id() const override;
    TrafficStateId update(
        const TrafficContext& context,
        const SpatialAwarenessState& awareness,
        ControlCommand& command) override;
};

class EmergencyBrakeState final : public ITrafficState {
public:
    TrafficStateId id() const override;
    TrafficStateId update(
        const TrafficContext& context,
        const SpatialAwarenessState& awareness,
        ControlCommand& command) override;
};

class TrafficFSMController {
public:
    explicit TrafficFSMController(const TrafficTuningConfig& tuning = TrafficTuningConfig());

    ControlCommand step(const TrafficContext& context);
    TrafficStateId current_state() const;
    const SpatialAwarenessState& latest_awareness() const;

private:
    void transition_to(TrafficStateId next_state, const TrafficContext& context);

    TrafficTuningConfig tuning_;
    std::unordered_map<TrafficStateId, std::unique_ptr<ITrafficState>> states_;
    TrafficStateId current_state_id_ = TrafficStateId::Cruise;
    SpatialAwarenessState latest_awareness_;
    int active_lane_target_ = -1;
};

}  // namespace traffic

#endif  // TRAFFICFSM_H
