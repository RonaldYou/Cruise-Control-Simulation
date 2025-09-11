#ifndef VEHICLE_H
#define VEHICLE_H

class Vehicle{
    private:
        double speed_;
        const double mass_;
        const double dragCoef_;
        const double frontalArea_;
        const double rollingResistanceCoef_;
        const double maxEngineForce_;

    public:
        explicit Vehicle(double mass, 
                double dragCoef, 
                double frontalArea,
                double rollingResistanceCoef, 
                double maxEngineForce);

        [[nodiscard]] double getSpeed() const noexcept { return speed_; }
        [[nodiscard]] double getMass() const noexcept { return mass_; }
        [[nodiscard]] double getDragCoef() const noexcept { return dragCoef_; }
        [[nodiscard]] double getFrontalArea() const noexcept { return frontalArea_; }
        [[nodiscard]] double getRollingResistanceCoef() const noexcept { return rollingResistanceCoef_; }
        [[nodiscard]] double getMaxEngineForce() const noexcept { return maxEngineForce_; }

        void update(double throttle, double dt, double terrainForce);

        virtual ~Vehicle() = default;
};

#endif // VEHICLE_H