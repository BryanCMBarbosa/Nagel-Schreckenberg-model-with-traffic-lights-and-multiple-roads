#ifndef ROADSECTION_H
#define ROADSECTION_H

#include <vector>
#include "Car.h"
#include "TrafficLight.h"

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

    void connect(std::vector<std::pair<int, std::shared_ptr<Road>>> connections);
    void disconnect(RoadSection* otherSection);

    ~RoadSection();
};

#endif
