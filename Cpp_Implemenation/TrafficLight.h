#ifndef TRAFFIC_LIGHT_H
#define TRAFFIC_LIGHT_H

#include <memory>
#include "TrafficLightGroup.h"
#include "Road.h"

class TrafficLightGroup;
class Road;

class TrafficLight : public std::enable_shared_from_this<TrafficLight>
{
public:
    short timeOpen;
    short timeClosed;
    short elapsedTime;
    bool state; //true = green/open, false = red/closed
    int distanceToPreviousTrafficLight;
    std::weak_ptr<Road> ownerRoad;
    int roadPosition;
    bool externalControl;

    std::weak_ptr<TrafficLightGroup> group;

    TrafficLight(bool externalControl, std::shared_ptr<Road> ownerRoad, int roadPosition);
    void setGroup(std::shared_ptr<TrafficLightGroup> groupPtr);
    void calculateDistanceToPreviousTrafficLight();
    void setTimeOpen(short time);
    int getRoadSpeed() const;
    double getBrakeProb() const;
    void toggle();
};

#endif
