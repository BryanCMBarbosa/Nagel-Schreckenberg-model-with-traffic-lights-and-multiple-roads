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
    bool externalControl;
    int timeOpen;
    int timeClosed;
    bool state;     // true = green/open, false = red/closed
    int timer;
    int distanceToPreviousTrafficLight;
    std::weak_ptr<Road> ownerRoad; // Changed from Road& to std::weak_ptr<Road>
    int roadPosition;

    std::weak_ptr<TrafficLightGroup> group;

    TrafficLight(bool externalControl, int timeOpen, int timeClosed, std::shared_ptr<Road> ownerRoad, int roadPosition);
    void setGroup(std::shared_ptr<TrafficLightGroup> groupPtr);
    void calculateDistanceToPreviousTrafficLight();
    int getRoadSpeed() const;
    double getBrakeProb() const;
    void turnGreen();
    bool isGreen() const;
};

#endif
