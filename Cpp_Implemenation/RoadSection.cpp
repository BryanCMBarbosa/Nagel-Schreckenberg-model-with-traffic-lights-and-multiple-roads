#include "RoadSection.h"

RoadSection::RoadSection(std::shared_ptr<Road> roadPtr, int idx)
    : road(roadPtr), index(idx), currentCar(nullptr), trafficLight(nullptr), isSharedSection(false){}


RoadSection::~RoadSection()
{
}

void RoadSection::addCar()
{
    auto roadPtr = road.lock();
    if (roadPtr)
        currentCar = std::make_shared<Car>(index, roadPtr->roadID);
    else
        std::cerr << "Error: Road has expired in RoadSection." << std::endl;
}

void RoadSection::connect(std::weak_ptr<RoadSection> connectedSection)
{
    connectedSections.push_back(connectedSection);
    isSharedSection = true;
}

void RoadSection::disconnect(RoadSection* otherSection)
{
    auto it = std::find_if(connectedSections.begin(), connectedSections.end(),
                           [otherSection](const std::weak_ptr<RoadSection>& weakPtr) 
                           {
                               auto ptr = weakPtr.lock();
                               return ptr.get() == otherSection;
                           });
    if (it != connectedSections.end())
    {
        connectedSections.erase(it);

        auto it_other = std::find_if(otherSection->connectedSections.begin(), otherSection->connectedSections.end(),
                                     [this](const std::weak_ptr<RoadSection>& weakPtr)
                                     {
                                         auto ptr = weakPtr.lock();
                                         return ptr.get() == this;
                                     });
        if (it_other != otherSection->connectedSections.end())
        {
            otherSection->connectedSections.erase(it_other);
        }
    }
}