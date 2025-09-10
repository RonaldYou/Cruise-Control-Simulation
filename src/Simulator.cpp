#include "Simulator.h"
#include <iostream>
#include <fstream>
#include <thread>
#include <chrono>

Simulator::Simulator(Vehicle* vehicle, Controller* controller) : vehicle_(vehicle), controller_(controller){
    std::cout<<"What is the target speed? ";
    std::cin>>targetSpeed_;
}

void Simulator::run(){
    while(true){
        double speed = vehicle_->getSpeed();
        double throttle = controller_->compute(targetSpeed_, speed, dt);
        vehicle_->update(throttle, dt);
        log(time, speed, throttle, true);
        time += dt;
        std::this_thread::sleep_for(std::chrono::milliseconds(SimulationConstants::LOOP_DELAY));
    }
}

void Simulator::log(double time, double speed, double throttle, bool logFile){
    std::cout << "\rTime: " << time << "| Target: " << targetSpeed_ << "| Speed: " << speed << "| Throttle: " << throttle << "     ";
    std::cout.flush();

    if(logFile){
        std::ofstream Log("../simulation.csv", std::ios::app);
        if(Log.is_open()){
            Log << time << "," << targetSpeed_ << "," << speed << "," << throttle << "\n";
            Log.close();
        }
    }
}
