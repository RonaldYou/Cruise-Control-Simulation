#include "Simulator.h"

Simulator::Simulator(Vehicle* vehicle, Controller* controller) : vehicle_(vehicle), controller_(controller){
    cout<<"What is the target speed? ";
    cin>>targetSpeed_;
}

Simulator::run(){
    while(true){
        double speed = vehicle_->getSpeed();
        double throttle = controller_->compute(speed, targetSpeed_, dt);
        vehicle_->update(throttle, dt);
        log(time, speed, throttle);
        time += dt;
    }
}

Simulator::log(double time, double speed, double throttle){
    cout << "\rTime: " << time << "| Target: " << targetSpeed_ << "| Speed: " << speed << "| Throttle: " << throttle << "     ";
    cout.flush();
}
