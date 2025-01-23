#include "RandomOffsetTrafficLightController.h"

RandomOffsetTrafficLightController::RandomOffsetTrafficLightController(RandomNumberGenerator& rng)
    : rng(rng) {}

void RandomOffsetTrafficLightController::initializeOffsets()
{
    int numIntersections = intersections.size();
    int numTrafficLights = 0;
    for (int i = 0; i < numIntersections; i++)
    {
        numTrafficLights = intersections[i]->trafficLights.size();
        for (int j = 0; j < numTrafficLights; j++)
        {
            //Calculate T_free and 2T for the light
            int T_free = calculateTFree(intersections[i]->trafficLights[j]);
            int T_cycle = 2 * T_free;

            //Assign a random offset in the range [1, 2T]
            randomOffsets.insert({{i, j}, T_cycle});
        }
    }
}

void RandomOffsetTrafficLightController::updateTrafficLights(unsigned long long /*timeStep*/)
{
    int numIntersections = intersections.size();

    for (int i = 0; i < numIntersections; i++)
    {
        auto& group = intersections[i];
        int numTrafficLights = group->trafficLights.size();

        //Identify the currently active (green) light in the group
        int activeLightIndex = -1;
        for (int j = 0; j < numTrafficLights; j++)
        {
            if (group->trafficLights[j]->state)
            {
                activeLightIndex = j; //The currently green light
                break;
            }
        }

        if (activeLightIndex != -1)
        {
            auto& activeLight = group->trafficLights[activeLightIndex];
            activeLight->elapsedTime++;

            if (activeLight->elapsedTime >= activeLight->timeOpen)
            {
                //The active light's green phase has ended
                activeLight->state = false;
                activeLight->elapsedTime = 0;

                //Move to the next light in the group
                int nextLightIndex = (activeLightIndex + 1) % numTrafficLights;
                auto& nextLight = group->trafficLights[nextLightIndex];

                //Set the next light to green and assign its random timeOpen
                nextLight->state = true;
                nextLight->timeOpen = randomOffsets[{i, nextLightIndex}];
                nextLight->elapsedTime = 0;
            }
        }
        else
        {
            //If no light is currently green, start with the first light in the group
            auto& firstLight = group->trafficLights[0];
            firstLight->state = true;
            firstLight->timeOpen = randomOffsets[{i, 0}];
            firstLight->elapsedTime = 0;
        }
    }
}

int RandomOffsetTrafficLightController::calculateTFree(std::shared_ptr<TrafficLight> light)
{
    double v_free = light->getRoadSpeed() - light->getBrakeProb();
    return static_cast<int>(light->distanceToPreviousTrafficLight / std::max(0.01, v_free));
}
