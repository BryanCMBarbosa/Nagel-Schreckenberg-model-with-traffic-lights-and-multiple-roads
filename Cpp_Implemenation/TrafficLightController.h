#ifndef TRAFFIC_LIGHT_CONTROLLER_H
#define TRAFFIC_LIGHT_CONTROLLER_H

#include <vector>
#include <memory>
#include "TrafficLightGroup.h"

class TrafficLightController : public std::enable_shared_from_this<TrafficLightController>
{
protected:
    std::vector<std::shared_ptr<TrafficLightGroup>> trafficLightGroups;
    unsigned int cycleTime;

    double calculateFreeFlowTime(std::shared_ptr<TrafficLight> trafficLight) const;

public:
    TrafficLightController(unsigned int cycleTime = 60);
    virtual ~TrafficLightController();

    void addTrafficLightGroup(std::shared_ptr<TrafficLightGroup> trafficLightGroup);

    virtual void initialize() = 0;

    virtual void update(unsigned long long currentTime) = 0;

    unsigned int getCycleTime() const;
    void setCycleTime(unsigned int time);
};

#endif