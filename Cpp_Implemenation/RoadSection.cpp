#include "RoadSection.h"
#include <algorithm>

RoadSection::RoadSection() 
    : currentCar(nullptr), isSharedSection(false), road(nullptr), trafficLight(nullptr), index(0) {}

RoadSection::RoadSection(Road* road, int index) 
    : currentCar(nullptr), isSharedSection(false), road(road), trafficLight(nullptr), index(index) {}

RoadSection::~RoadSection()
{
    //Delete the car if it exists to avoid memory leaks
    if (currentCar)
    {
        delete currentCar;
        currentCar = nullptr;
    }
}

void RoadSection::addCar()
{
    currentCar = new Car(index);
}

void RoadSection::connect(RoadSection* otherSection)
{
    if (otherSection && std::find(connectedSections.begin(), connectedSections.end(), otherSection) == connectedSections.end())
    {
        connectedSections.push_back(otherSection);
        otherSection->connectedSections.push_back(this);  // Mutual connection
        isSharedSection = true;
        otherSection->isSharedSection = true;
    }
}

void RoadSection::disconnect(RoadSection* otherSection)
{
    auto it = std::find(connectedSections.begin(), connectedSections.end(), otherSection);
    if (it != connectedSections.end())
    {
        connectedSections.erase(it);
        auto it_other = std::find(otherSection->connectedSections.begin(), otherSection->connectedSections.end(), this);
        if (it_other != otherSection->connectedSections.end())
        {
            otherSection->connectedSections.erase(it_other);
        }
    }
}
