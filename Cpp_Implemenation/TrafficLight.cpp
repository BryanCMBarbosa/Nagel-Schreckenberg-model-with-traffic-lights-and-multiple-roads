#include "TrafficLight.h"

TrafficLight::TrafficLight(int pos,  Road* r, TrafficLight* p) : position(pos), road(r), pair(p)
{
}

void TrafficLight::toggleState()
{
    state = !state;
}

bool TrafficLight::updateLight(int time)
{
    if (!state)
    {
        if (pair != nullptr)
        {
            if (!pair->state)
            {
                toggleState();
                timeOpen = time;
                return true;
            }
            else
            {
                return false;
            }
        }
        else
        {
            toggleState();
            timeOpen = time;
            return true;
        }
    }
    else
    {
        toggleState();
        timeClosed = time;
        return true;
    }
}