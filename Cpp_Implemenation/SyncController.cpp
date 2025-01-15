#include "SyncController.h"

SyncController::SyncController()
    : TrafficLightController(60) {}

void SyncController::initialize()
{
    calculateCycleTime();

    for (auto& group : trafficLightGroups)
    {
        group->initialize();
        for (size_t j = 0; j < group->trafficLights.size(); ++j)
        {
            if (j % 2 == 0)
                group->trafficLights[j]->state = true; //North-bound starts green
            else
                group->trafficLights[j]->state = false; //East-bound starts red
        }
    }
}

void SyncController::update(unsigned long long currentTime)
{
    bool isNorthBoundGreen = (currentTime / phaseTime) % 2 == 0;

    for (auto& group : trafficLightGroups)
    {
        for (size_t j = 0; j < group->trafficLights.size(); ++j)
        {
            if (j % 2 == 0) //North-bound (even index)
                group->trafficLights[j]->state = isNorthBoundGreen;
            else //East-bound (odd index)
                group->trafficLights[j]->state = !isNorthBoundGreen;
        }
    }
}

void SyncController::calculateCycleTime()
{
    if (trafficLightGroups.empty() || trafficLightGroups[0]->trafficLights.empty())
        throw std::runtime_error("No traffic lights to calculate cycle time.");

    double T_free = calculateFreeFlowTime(trafficLightGroups[0]->trafficLights[0]);
    cycleTime = static_cast<unsigned int>(std::round(T_free * 2));
    phaseTime = cycleTime / 2;
}

SyncController::~SyncController() {}