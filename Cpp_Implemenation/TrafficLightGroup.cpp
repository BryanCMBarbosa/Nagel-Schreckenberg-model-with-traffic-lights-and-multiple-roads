#include "TrafficLightGroup.h"

TrafficLightGroup::TrafficLightGroup()
    : currentIndex(0), inGreenPhase(true), inTransitionPhase(false), groupTimer(0), transitionTime(0)
{
}

void TrafficLightGroup::setTransitionTime(int time)
{
    transitionTime = time;
}

void TrafficLightGroup::addTrafficLight(std::shared_ptr<TrafficLight> trafficLight)
{
    trafficLights.push_back(trafficLight);
    trafficLight->setGroup(shared_from_this());
}

void TrafficLightGroup::initialize()
{
    if (!trafficLights.empty())
    {
        currentIndex = 0;
        inGreenPhase = true;
        inTransitionPhase = false;
        groupTimer = 0;
        calculateTotalCycleTime();

        for (auto& tl : trafficLights)
        {
            tl->state = false;
            tl->timer = 0;
        }

        trafficLights[currentIndex]->state = true;
    }
}

void TrafficLightGroup::calculateTotalCycleTime()
{
    totalCycleTime = 0;
    for (const auto& tl : trafficLights)
    {
        totalCycleTime += tl->timeOpen;
    }
    totalCycleTime += transitionTime * trafficLights.size();
}

void TrafficLightGroup::update()
{
    if (trafficLights.empty())
        return;

    groupTimer++;

    if (inGreenPhase)
    {
        auto currentTrafficLight = trafficLights[currentIndex];

        if (groupTimer >= currentTrafficLight->timeOpen)
        {
            currentTrafficLight->state = false;
            groupTimer = 0;
            inGreenPhase = false;
            inTransitionPhase = true;
        }
    }
    else if (inTransitionPhase)
    {
        if (groupTimer >= transitionTime)
        {
            groupTimer = 0;
            inGreenPhase = true;
            inTransitionPhase = false;

            currentIndex = (currentIndex + 1) % trafficLights.size();
            auto nextTrafficLight = trafficLights[currentIndex];
            nextTrafficLight->state = true;
        }
    }
}
