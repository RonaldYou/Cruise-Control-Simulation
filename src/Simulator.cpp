#include "Simulator.h"
#include <iostream>

Simulator::Simulator(Vehicle* vehicle, Controller* controller) : vehicle_(vehicle), controller_(controller){
    std::cout<<"What is the target speed? ";
    std::cin>>targetSpeed_;
}

void Simulator::run(){
    while(true){
        double speed = vehicle_->getSpeed();
        double throttle = controller_->compute(targetSpeed_, speed, dt);
        vehicle_->update(throttle, dt);
        log(time, speed, throttle);
        time += dt;
    }
}

void Simulator::log(double time, double speed, double throttle){
    std::cout << "\rTime: " << time << "| Target: " << targetSpeed_ << "| Speed: " << speed << "| Throttle: " << throttle << "     ";
    std::cout.flush();
}
