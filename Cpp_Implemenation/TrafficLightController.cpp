#include "TrafficLightController.h"

void TrafficLightController::addIntersection(const std::shared_ptr<TrafficLightGroup>& group)
{
    intersections.push_back(group);
}

void TrafficLightController::initialize()
{
    for (auto& intersection : intersections)
        intersection->initialize();
}