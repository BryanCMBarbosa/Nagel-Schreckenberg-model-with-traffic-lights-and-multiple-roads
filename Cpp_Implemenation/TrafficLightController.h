#ifndef TRAFFIC_LIGHT_CONTROLLER_H
#define TRAFFIC_LIGHT_CONTROLLER_H

#include <vector>
#include <memory>
#include "TrafficLightGroup.h"
#include "RandomNumberGenerator.h"

class TrafficLightController
{
public:
    virtual void updateTrafficLights(unsigned long long timeStep) = 0;
    virtual ~TrafficLightController() = default;

    void addIntersection(const std::shared_ptr<TrafficLightGroup>& group);
    void initialize();

protected:
    std::vector<std::shared_ptr<TrafficLightGroup>> intersections;

    TrafficLightController() = default;
};

#endif