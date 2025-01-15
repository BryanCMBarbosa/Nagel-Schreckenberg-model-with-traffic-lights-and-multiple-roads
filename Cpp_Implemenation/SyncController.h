#ifndef SYNC_CONTROLLER_H
#define SYNC_CONTROLLER_H

#include "TrafficLightController.h"

class SyncController : public TrafficLightController
{
public:
    SyncController();

    ~SyncController() override;

    void initialize() override;
    void update(unsigned long long currentTime) override;

private:
    unsigned int phaseTime;

    void calculateCycleTime();
};

#endif