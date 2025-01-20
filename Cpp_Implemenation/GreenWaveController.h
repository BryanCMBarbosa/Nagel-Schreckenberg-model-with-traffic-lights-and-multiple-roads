#ifndef GREEN_WAVE_CONTROLLER_H
#define GREEN_WAVE_CONTROLLER_H

#include "TrafficLightController.h"
#include <cmath>
#include <stdexcept>

class GreenWaveController : public TrafficLightController
{
public:
    GreenWaveController(unsigned int cycleTime = 30, double vMax = 1.0, double p = 0.0, int columns = 1);

    ~GreenWaveController() override;

    void initialize() override;
    void update(unsigned long long currentTime) override;
    void updateParametersBasedOnRoad(double distanceBetweenIntersections, int roadVMax = -1, double roadP = 2.0);

private:
    double vMax;
    double p;
    double vFree;

    unsigned int calculateOffset(size_t rowIndex, size_t colIndex, double distanceBetweenIntersections, double v_free);
};

#endif
