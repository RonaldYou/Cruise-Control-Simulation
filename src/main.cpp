#include "Simulator.h"
#include "Car.h"
#include "PIDController.h"

int main() {
    Vehicle* vehicle = new Car();
    Controller* controller = new PIDController(0.3, 0.05, 0.05);
    Simulator* simulator = new Simulator(vehicle, controller);
    simulator->run();
    delete vehicle;
    delete controller;
    delete simulator;
    return 0;
}