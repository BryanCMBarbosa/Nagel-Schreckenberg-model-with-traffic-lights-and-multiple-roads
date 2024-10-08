#ifndef TRAFFIC_LIGHT_H
#define TRAFFIC_LIGHT_H

#include <vector>
#include <memory>

class TrafficLight : public std::enable_shared_from_this<TrafficLight>
{
public:
    bool externalControl;
    int timeOpen;
    int timeClosed; //For non-paired traffic lights
    bool state; //true = green/open, false = red/closed
    int timer;
    std::vector<std::weak_ptr<TrafficLight>> pairedTrafficLights;

    TrafficLight(bool externalControl, int timeOpen);
    void addPairedTrafficLight(std::shared_ptr<TrafficLight> other);
    void setTimeClosed(int timeClosed);
    void update();
};

#endif
