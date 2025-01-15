#include "TrafficLight.h"
#include <iostream>
#include <algorithm>

TrafficLight::TrafficLight(bool externalControl, int timeOpen, int timeClosed, std::shared_ptr<Road> ownerRoad, int roadPosition)
    : externalControl(externalControl), timeOpen(timeOpen), timeClosed(timeClosed), ownerRoad(ownerRoad), roadPosition(roadPosition), state(false), timer(0)
{
}

void TrafficLight::setGroup(std::shared_ptr<TrafficLightGroup> groupPtr)
{
    group = groupPtr;
}

void TrafficLight::calculateDistanceToPreviousTrafficLight()
{
    if (auto roadPtr = ownerRoad.lock())
    {
        std::vector<int> otherTrafficLightsSameRoad = roadPtr->trafficLightPositions;

        auto positionIterator = std::find(otherTrafficLightsSameRoad.begin(), otherTrafficLightsSameRoad.end(), roadPosition);

        if (positionIterator == otherTrafficLightsSameRoad.end())
            std::cerr << "No traffic light in the position " << roadPosition << " of the road " << roadPtr->roadID << "." << std::endl;
        else if (otherTrafficLightsSameRoad.size() != 1)
        {
            if (positionIterator == otherTrafficLightsSameRoad.begin())
            {
                if (roadPtr->isPeriodic) // The "-1" is because cars always stop one section before the Traffic Light
                    distanceToPreviousTrafficLight = roadPosition + (roadPtr->roadSize - *(otherTrafficLightsSameRoad.rbegin())) - 1;
                else
                    distanceToPreviousTrafficLight = roadPosition - 1;
            }
            else
                distanceToPreviousTrafficLight = roadPosition - *std::prev(positionIterator) - 1;
        }
        else if (roadPtr->isPeriodic)
            distanceToPreviousTrafficLight = roadPtr->roadSize - 1;
        else
            distanceToPreviousTrafficLight = std::max(roadPosition - 1, 0);
    }
    else
        std::cerr << "Road object is no longer available for TrafficLight at position " << roadPosition << "." << std::endl;
}

int TrafficLight::getRoadSpeed() const
{
    if (auto roadPtr = ownerRoad.lock())
        return roadPtr->maxSpeed;
    else
        throw std::runtime_error("Road is no longer accessible.");
}

double TrafficLight::getBrakeProb() const
{
    if (auto roadPtr = ownerRoad.lock())
        return roadPtr->brakeProb;
    else
        throw std::runtime_error("Road is no longer accessible.");
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
