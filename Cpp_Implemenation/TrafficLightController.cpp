#include "TrafficLightController.h"

TrafficLightController::TrafficLightController(unsigned int cycleTime)
    : cycleTime(cycleTime) {}

void TrafficLightController::addTrafficLightGroup(std::shared_ptr<TrafficLightGroup> trafficLightGroup)
{
    trafficLightGroups.push_back(trafficLightGroup);
}

unsigned int TrafficLightController::getCycleTime() const
{
    return cycleTime;
}

void TrafficLightController::setCycleTime(unsigned int time)
{
    cycleTime = time;
}

double TrafficLightController::calculateFreeFlowTime(std::shared_ptr<TrafficLight> trafficLight) const
{
    double distance = trafficLight->distanceToPreviousTrafficLight;
    double VMax = trafficLight->getRoadSpeed();
    double p = trafficLight->getBrakeProb();

    if (distance <= 0 || VMax <= 0 || p < 0 || p > 1)
        throw std::invalid_argument("Invalid traffic light parameters.");

    double VFree = VMax - p;
    return distance / VFree;
}

TrafficLightController::~TrafficLightController() {}