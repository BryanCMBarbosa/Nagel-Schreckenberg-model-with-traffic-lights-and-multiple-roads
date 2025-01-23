#include "SynchronizedTrafficLightController.h"

SynchronizedTrafficLightController::SynchronizedTrafficLightController() = default;

void SynchronizedTrafficLightController::updateTrafficLights(unsigned long long /*timeStep*/)
{
    for (auto& group : intersections)
    {
        //Ensure there are exactly two traffic lights in the group
        if (group->trafficLights.size() < 2)
        {
            std::cerr << "Intersection does not have both traffic lights." << std::endl;
            continue;
        }

        auto& northLight = group->trafficLights[0]; //North-Bound (even index)
        auto& eastLight = group->trafficLights[1];  //East-Bound (odd index)

        if (northLight->state)
        {
            northLight->elapsedTime++;

            if (northLight->elapsedTime >= northLight->timeOpen)
            {

                northLight->state = false;
                northLight->elapsedTime = 0;

                eastLight->state = true;
                eastLight->timeOpen = calculateCycleTime(northLight) / 2; // Set green phase duration
                eastLight->elapsedTime = 0;
            }
        }
        else if (eastLight->state)
        {
            eastLight->elapsedTime++;

            if (eastLight->elapsedTime >= eastLight->timeOpen)
            {
                eastLight->state = false;
                eastLight->elapsedTime = 0;

                northLight->state = true;
                northLight->timeOpen = calculateCycleTime(eastLight) / 2;
                northLight->elapsedTime = 0;
            }
        }
        else
        {
            northLight->state = true;
            northLight->timeOpen = calculateCycleTime(northLight) / 2;
            northLight->elapsedTime = 0;

            eastLight->state = false;
        }
    }
}

int SynchronizedTrafficLightController::calculateCycleTime(std::shared_ptr<TrafficLight> light)
{
    std::cout << "D: " << light->distanceToPreviousTrafficLight << std::endl;
    std::cout << "Cycle Time: " << light->distanceToPreviousTrafficLight / std::max(0.01, (light->getRoadSpeed() - light->getBrakeProb())) << std::endl;
    return static_cast<int>(light->distanceToPreviousTrafficLight / std::max(0.01, (light->getRoadSpeed() - light->getBrakeProb())));
}
