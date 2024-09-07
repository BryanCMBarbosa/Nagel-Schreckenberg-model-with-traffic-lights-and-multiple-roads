#ifndef ROADSECTION_H
#define ROADSECTION_H

#include <vector>
#include "Car.h"
#include "TrafficLight.h"

class Road; 
class TrafficLight;

class RoadSection
{
public:
    Car* currentCar;  // Now using a raw pointer to Car
    std::vector<RoadSection*> connectedSections;
    bool isSharedSection;
    Road* road;
    TrafficLight* trafficLight;
    int index;

    RoadSection();

    RoadSection(Road* road, int index);

    void addCar();

    void connect(RoadSection* otherSection);
    void disconnect(RoadSection* otherSection);

    ~RoadSection();  // Destructor to manage cleanup if needed
};

#endif
