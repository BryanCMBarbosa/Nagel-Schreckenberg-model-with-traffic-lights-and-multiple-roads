#ifndef ROADSECTION_H
#define ROADSECTION_H

#include "Car.h"
#include "TrafficLight.h"
#include <vector>
#include <algorithm>

class Road; 
class Car;
class TrafficLight;

class RoadSection: public std::enable_shared_from_this<RoadSection>
{
public:
    std::shared_ptr<Car> currentCar;
    std::vector<std::weak_ptr<RoadSection>> connectedSections;
    bool isSharedSection;
    std::weak_ptr<Road> road;
    std::shared_ptr<TrafficLight> trafficLight;
    int index;

    RoadSection();

    RoadSection(std::shared_ptr<Road> road, int index);

    void addCar();

    void connect(std::weak_ptr<RoadSection> connectedSection);
    void disconnect(RoadSection* otherSection);

    ~RoadSection();
};

#endif