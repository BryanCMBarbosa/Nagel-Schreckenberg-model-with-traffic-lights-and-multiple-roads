#ifndef SYNCHRONIZED_TRAFFIC_LIGHT_CONTROLLER_H
#define SYNCHRONIZED_TRAFFIC_LIGHT_CONTROLLER_H

#include "TrafficLightController.h"

class SynchronizedTrafficLightController : public TrafficLightController {
public:
    SynchronizedTrafficLightController();
    void updateTrafficLights(unsigned long long timeStep) override;

private:
    int calculateCycleTime(std::shared_ptr<TrafficLight> light); // Internal calculation
};

#endif
