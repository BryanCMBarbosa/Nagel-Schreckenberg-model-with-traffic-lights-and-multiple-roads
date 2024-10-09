#include "TrafficLightGroup.h"

TrafficLightGroup::TrafficLightGroup()
    : currentIndex(0), inGreenPhase(true), groupTimer(0)
{
}

void TrafficLightGroup::addTrafficLight(std::shared_ptr<TrafficLight> trafficLight)
{
    trafficLights.push_back(trafficLight);
    trafficLight->setGroup(shared_from_this());
    trafficLight->state = false;
    trafficLight->timer = 0;
}

void TrafficLightGroup::initialize()
{
    if (!trafficLights.empty())
    {
        currentIndex = 0;
        inGreenPhase = true;
        groupTimer = 0;
        for (size_t i = 0; i < trafficLights.size(); ++i)
        {
            if (i == currentIndex)
            {
                trafficLights[i]->state = true;
            }
            else
            {
                trafficLights[i]->state = false;
            }
            trafficLights[i]->timer = 0;
        }
    }
}

void TrafficLightGroup::update()
{
    if (trafficLights.empty())
        return;

    auto currentTrafficLight = trafficLights[currentIndex];
    auto nextIndex = (currentIndex + 1) % trafficLights.size();
    auto nextTrafficLight = trafficLights[nextIndex];

    if (inGreenPhase)
    {
        groupTimer++;

        if (groupTimer >= currentTrafficLight->timeOpen)
        {
            currentTrafficLight->state = false;
            groupTimer = 0;
            inGreenPhase = false;
        }
    }
    else
    {
        groupTimer++;

        if (groupTimer >= nextTrafficLight->timeClosed)
        {
            currentIndex = nextIndex;
            trafficLights[currentIndex]->state = true;
            groupTimer = 0;
            inGreenPhase = true;
        }
    }
}
