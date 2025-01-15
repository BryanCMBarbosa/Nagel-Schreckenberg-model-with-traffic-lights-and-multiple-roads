#ifndef GREEN_WAVE_CONTROLLER_H
#define GREEN_WAVE_CONTROLLER_H

#include "TrafficLightController.h"
#include <cmath>
#include <stdexcept>

class GreenWaveController : public TrafficLightController
{
public:
    GreenWaveController(unsigned int cycleTime = 60, double vMax = 1.0, double p = 0.0, size_t numberOfColumns = 1);

    ~GreenWaveController() override;

    void initialize() override;
    void update(unsigned long long currentTime) override;
    void calculateCycleTime(double distanceBetweenIntersections);

private:
    double v_max;
    double p;
    double distanceBetweenIntersections;
    double v_free;
    size_t numberOfColumns;

    unsigned int calculateOffset(size_t rowIndex, size_t colIndex, double distanceBetweenIntersections, double v_free);
};

#endif
