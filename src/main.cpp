#include "Simulator.h"
#include "Car.h"
#include "PIDController.h"

int main() {
    std::unique_ptr<Vehicle> vehicle = std::make_unique<Car>();
    std::unique_ptr<Controller> controller = std::make_unique<PIDController>(0.3, 0.05, 0.05);
    std::unique_ptr<Simulator> simulator = std::make_unique<Simulator>(vehicle.get(), controller.get());
    simulator->run();
    return 0;
}