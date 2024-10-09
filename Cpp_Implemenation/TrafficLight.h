#ifndef TRAFFIC_LIGHT_H
#define TRAFFIC_LIGHT_H

#include <memory>
#include "TrafficLightGroup.h"

class TrafficLightGroup;

class TrafficLight : public std::enable_shared_from_this<TrafficLight>
{
public:
    bool externalControl;
    int timeOpen;
    int timeClosed;
    bool state;     // true = green/open, false = red/closed
    int timer;

    std::weak_ptr<TrafficLightGroup> group;

    TrafficLight(bool externalControl, int timeOpen, int timeClosed);
    void setGroup(std::shared_ptr<TrafficLightGroup> groupPtr);
    void turnGreen();
    bool isGreen() const;
};

#endif
