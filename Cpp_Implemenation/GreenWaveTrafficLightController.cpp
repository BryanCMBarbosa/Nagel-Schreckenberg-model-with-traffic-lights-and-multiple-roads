#include "GreenWaveTrafficLightController.h"

GreenWaveTrafficLightController::GreenWaveTrafficLightController() = default;

void GreenWaveTrafficLightController::updateTrafficLights(unsigned long long timeStep)
{
    for (auto& group : intersections)
    {
        int i = group->row;
        int j = group->column;

        auto& northLight = group->trafficLights[0]; //North-Bound light
        auto& eastLight = group->trafficLights[1];  //East-Bound light

        //Calculate T_free and T_cycle (2*T_free) for the North-Bound light
        int T_free = calculateTFree(northLight);
        int T_cycle = 2 * T_free;

        //Calculate the offset ΔT_{i,j} for this intersection
        int T_delay = calculateOffset(i, j, T_free, T_cycle);

        //Determine if North-Bound light should be green at this time step
        bool shouldBeOpen = ((timeStep + T_delay) % T_cycle) < T_free;

        if (northLight->state)
        {
            northLight->elapsedTime++;

            if (northLight->elapsedTime >= T_free)
            {
                //North-Bound light's green phase ends, toggle both lights
                northLight->state = false; //Turn North-Bound light red
                eastLight->state = true;  //Turn East-Bound light green
                northLight->elapsedTime = 0; //Reset elapsed time for North-Bound light
            }
        }
        else if (eastLight->state)
        {
            //East-Bound light is green (North-Bound is red)
            northLight->elapsedTime++;

            if (northLight->elapsedTime >= T_free)
            {
                //East-Bound light's green phase ends, toggle both lights
                northLight->state = true; //Turn North-Bound light green
                eastLight->state = false; //Turn East-Bound light red
                northLight->elapsedTime = 0; //Reset elapsed time for North-Bound light
            }
        }
        else
        {
            //No light is active (initial state)
            if (shouldBeOpen)
            {
                northLight->state = true; //Start with North-Bound green
                eastLight->state = false; //East-Bound red
                northLight->elapsedTime = 0;
            }
            else
            {
                northLight->state = false; //Start with North-Bound red
                eastLight->state = true;  //East-Bound green
                northLight->elapsedTime = 0;
            }
        }
    }
}

int GreenWaveTrafficLightController::calculateTFree(std::shared_ptr<TrafficLight> light)
{
    return static_cast<int>(light->distanceToPreviousTrafficLight / std::max(0.01, (light->getRoadSpeed() - light->getBrakeProb())));
}

int GreenWaveTrafficLightController::calculateOffset(int i, int j, int T_free, int T_cycle)
{
    return ((i + j) * T_free) % T_cycle;
}
