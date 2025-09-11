#include "Simulator.h"
#include <iostream>
#include <fstream>
#include <thread>
#include <chrono>
#include <cstdlib>

Simulator::Simulator(Vehicle* vehicle, Controller* controller) : vehicle_(vehicle), controller_(controller){
    std::cout<<"What is the target speed? ";
    std::cin>>targetSpeed_;
}

void Simulator::run(){
    Terrain terrain;
    double elevation = 0.0;
    bool firstFrame = true;

    while(true){

        terrain.update(dt);
        
        double speed = vehicle_->getSpeed();
        double throttle = controller_->compute(targetSpeed_, speed, dt);
        double terrainForce = terrain.getGradeForce(vehicle_->getMass());

        vehicle_->update(throttle, dt, terrainForce);
        elevation += speed * terrain.getCurrentGrade() * dt;
        terrain.addElevationPoint(elevation);

        
        auto terrainDisplay = terrain.getASCIIDisplay();
        
        if(!firstFrame){
            int totalLines = 2 + terrainDisplay.size();
            std::cout<< "\033["<< totalLines << "A";
        }

        std::cout << "=== CRUISE CONTROL SIMULATION ===\n";
        
        
        for (const auto& line : terrainDisplay) {
            std::cout << line << std::string(80, ' ') << "\n"; // Clear rest of line
        }

        firstFrame = false;
        std::cout << "Time: " << time << "| Target: " << targetSpeed_ << "| Speed: " << speed << "| Throttle: " << throttle<< std::string(80, ' ') << "\n";

        std::cout.flush();

        //log(time, speed, throttle, false);
        std::cout.flush();
        time += dt;
        std::this_thread::sleep_for(std::chrono::milliseconds(SimulationConstants::LOOP_DELAY));
    }
}

void Simulator::log(double time, double speed, double throttle, bool logFile){
    std::cout << "Time: " << time << "| Target: " << targetSpeed_ << "| Speed: " << speed << "| Throttle: " << throttle << "     ";
    //std::cout.flush();

    if(logFile){
        std::ofstream Log("../simulation.csv", std::ios::app);
        if(Log.is_open()){
            Log << time << "," << targetSpeed_ << "," << speed << "," << throttle << "\n";
            Log.close();
        }
    }
}
