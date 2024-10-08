// TrafficLight.cpp
#include "TrafficLight.h"

TrafficLight::TrafficLight(bool externalControl, int timeOpen)
    : externalControl(externalControl), timeOpen(timeOpen), timeClosed(0), state(true), timer(0)
{
}

void TrafficLight::addPairedTrafficLight(std::shared_ptr<TrafficLight> other)
{
    pairedTrafficLights.push_back(other);
}

void TrafficLight::setTimeClosed(int timeClosed)
{
    this->timeClosed = timeClosed;
}

void TrafficLight::update()
{
    if (!externalControl)
    {
        timer++;
        if (state && timer >= timeOpen)
        {
            state = false;
            timer = 0;

            for (auto& weakLight : pairedTrafficLights)
            {
                if (auto pairedLight = weakLight.lock())
                {
                    pairedLight->state = true;
                    pairedLight->timer = 0;
                }
            }
        }
        else if (!state && timer >= timeClosed)
        {
            state = true;
            timer = 0;

            for (auto& weakLight : pairedTrafficLights)
            {
                if (auto pairedLight = weakLight.lock())
                {
                    pairedLight->state = false;
                    pairedLight->timer = 0;
                }
            }
        }
    }
    else
    {
        //External control will come here
    }
}
