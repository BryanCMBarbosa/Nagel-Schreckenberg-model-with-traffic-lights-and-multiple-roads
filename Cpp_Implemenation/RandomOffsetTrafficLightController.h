#ifndef RANDOM_OFFSET_TRAFFIC_LIGHT_CONTROLLER_H
#define RANDOM_OFFSET_TRAFFIC_LIGHT_CONTROLLER_H

#include "TrafficLightController.h"
#include "RandomNumberGenerator.h"

class RandomOffsetTrafficLightController : public TrafficLightController
{
public:
    explicit RandomOffsetTrafficLightController(RandomNumberGenerator& rng);
    void initializeOffsets();
    void updateTrafficLights(unsigned long long timeStep) override;
    int calculateTFree(std::shared_ptr<TrafficLight> light);

private:
    RandomNumberGenerator& rng;

    std::map<std::pair<int, int>, int> randomOffsets; //Stores assigned offsets
};

#endif
