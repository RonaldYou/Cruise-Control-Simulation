/*
 * main.cpp - Entry Point
 *
 * Creates the vehicle, controller, and simulator, then runs the simulation.
 * The Simulator handles OpenGL initialization internally.
 */

#include "Simulator.h"
#include "Car.h"
#include "PIDController.h"
#include <iostream>
#include <stdexcept>

int main() {
    try {
        /* Create the vehicle (physics simulation)
         *
         * Car is a concrete implementation of Vehicle with sedan-like
         * properties (mass, drag coefficient, max engine force, etc.)
         */
        std::unique_ptr<Vehicle> vehicle = std::make_unique<Car>();

        /* Create the cruise control algorithm
         *
         * PID Controller parameters:
         *   Kp = 0.3  (Proportional gain - response to current error)
         *   Ki = 0.05 (Integral gain - response to accumulated error)
         *   Kd = 0.05 (Derivative gain - response to rate of change)
         */
        std::unique_ptr<Controller> controller = std::make_unique<PIDController>(0.3, 0.05, 0.05);

        /* Create and run the simulator
         *
         * The simulator creates an OpenGL window and enters the main loop.
         * It runs until the user closes the window.
         */
        std::unique_ptr<Simulator> simulator = std::make_unique<Simulator>(vehicle.get(), controller.get());
        simulator->run();

        return 0;

    } catch (const std::exception& e) {
        /* Handle any errors (OpenGL init failure, shader compile errors, etc.) */
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}