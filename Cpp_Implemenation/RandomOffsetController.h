#ifndef RANDOM_OFFSET_CONTROLLER_H
#define RANDOM_OFFSET_CONTROLLER_H

#include "TrafficLightController.h"
#include "RandomNumberGenerator.h"

class RandomOffsetController : public TrafficLightController
{
public:
    RandomOffsetController(RandomNumberGenerator& rng);

    ~RandomOffsetController() override;

    void initialize() override;
    void update(unsigned long long currentTime) override;

private:
    RandomNumberGenerator& rng;
    std::vector<unsigned int> randomOffsets;

    void initializeRandomOffsets();
};

#endif