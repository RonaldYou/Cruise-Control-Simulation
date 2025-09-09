#include "Simulator.h"
#include "Car.h"
#include "PIDController.h"

int main() {
    Vehicle* vehicle = new Car();
    Controller* controller = new PIDController(0.03, 0.006, 0.0);
    Simulator* simulator = new Simulator(vehicle, controller);
    simulator->run();
    delete vehicle;
    delete controller;
    delete simulator;
    return 0;
}