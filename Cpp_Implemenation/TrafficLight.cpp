#include "TrafficLight.h"

TrafficLight::TrafficLight(bool externalControl, int timeOpen, int timeClosed)
    : externalControl(externalControl), timeOpen(timeOpen), timeClosed(timeClosed), state(false), timer(0)
{
}

void TrafficLight::setGroup(std::shared_ptr<TrafficLightGroup> groupPtr)
{
    group = groupPtr;
}

void TrafficLight::turnGreen()
{
    state = true;
    timer = 0;
}

bool TrafficLight::isGreen() const
{
    return state;
}
