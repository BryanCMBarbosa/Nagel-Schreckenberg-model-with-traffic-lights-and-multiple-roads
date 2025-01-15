#include "RandomOffsetController.h"

RandomOffsetController::RandomOffsetController(RandomNumberGenerator& rng)
    : TrafficLightController(), rng(rng) {}

void RandomOffsetController::initialize()
{
    initializeRandomOffsets();

    for (auto& group : trafficLightGroups)
        group->initialize();
}

void RandomOffsetController::update(unsigned long long currentTime)
{
    for (size_t groupIndex = 0; groupIndex < trafficLightGroups.size(); ++groupIndex)
    {
        auto& group = trafficLightGroups[groupIndex];
        unsigned int offset = randomOffsets[groupIndex];

        for (size_t j = 0; j < group->trafficLights.size(); ++j)
        {
            bool isGreen = ((currentTime + offset) / (cycleTime / 2)) % 2 == 0;

            if (j % 2 == 0) //North-bound
                group->trafficLights[j]->state = isGreen;
            else //East-bound
                group->trafficLights[j]->state = !isGreen;
        }
    }
}

void RandomOffsetController::initializeRandomOffsets()
{
    randomOffsets.resize(trafficLightGroups.size());

    for (size_t groupIndex = 0; groupIndex < trafficLightGroups.size(); ++groupIndex)
        randomOffsets[groupIndex] = rng.getRandomInt(0, cycleTime);
}

RandomOffsetController::~RandomOffsetController() {}