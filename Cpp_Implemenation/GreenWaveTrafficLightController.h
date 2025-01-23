#ifndef GREEN_WAVE_TRAFFIC_LIGHT_CONTROLLER_H
#define GREEN_WAVE_TRAFFIC_LIGHT_CONTROLLER_H

#include "TrafficLightController.h"

class GreenWaveTrafficLightController : public TrafficLightController {
public:
    GreenWaveTrafficLightController();
    void updateTrafficLights(unsigned long long timeStep) override;

private:
    int calculateOffset(int i, int j, int T_free, int T_cycle);
    int calculateTFree(std::shared_ptr<TrafficLight> light);
};

#endif
