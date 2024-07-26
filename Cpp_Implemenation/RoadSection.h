#ifndef ROADSECTION_H
#define ROADSECTION_H

#include <vector>
#include <memory>
#include "Car.h"
#include "TrafficLight.h"

class Road; 
class TrafficLight;

class RoadSection : public std::enable_shared_from_this<RoadSection>
    {
public:
    Car* currentCar;
    std::vector<std::shared_ptr<RoadSection>> connectedSections;
    bool isSharedSection;
    Road* road;
    TrafficLight* trafficLight;


    RoadSection();
    virtual ~RoadSection();

    void connect(std::shared_ptr<RoadSection> otherSection);
    void disconnect(std::shared_ptr<RoadSection> otherSection);
};

#endif
