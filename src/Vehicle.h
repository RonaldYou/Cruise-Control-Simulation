
class Vehicle{
    protected:
        double speed;
        double mass;
        double dragCoef;
        double frontalArea;
        double rollingResistanceCoef;
        double maxEngineForce;
    public:
        Vehicle(double mass, double dragCoef, double frontalArea,
                double rollingResistanceCoef, double maxEngineForce);
        void update(double throttle, double dt);
};