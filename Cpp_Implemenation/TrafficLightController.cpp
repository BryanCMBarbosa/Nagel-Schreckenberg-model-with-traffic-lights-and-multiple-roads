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
    double v_max = trafficLight->getRoadSpeed();
    double p = trafficLight->getBrakeProb();

    if (distance <= 0 || v_max <= 0 || p < 0 || p > 1)
        throw std::invalid_argument("Invalid traffic light parameters.");

    double v_free = v_max - p;
    return distance / v_free;
}

TrafficLightController::~TrafficLightController() {}