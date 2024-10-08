#include "RoadSection.h"
#include <algorithm>

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

void RoadSection::connect(std::vector<std::pair<int, std::shared_ptr<Road>>> connections)
{
    auto roadPtr = road.lock();
    if (!roadPtr)
    {
        std::cerr << "Error: RoadSection's road has expired." << std::endl;
        return;
    }

    int numberConnections = connections.size();
    for (int i = 0; i < numberConnections; i++)
    {
        if (connections[i].second->roadID == roadPtr->roadID)
            continue;

        bool alreadyFound = false;

        for (auto& existingWeakSection : connectedSections)
        {
            auto existingSection = existingWeakSection.lock();
            if (existingSection)
            {
                auto existingRoadPtr = existingSection->road.lock();
                if (existingRoadPtr && connections[i].second->roadID == existingRoadPtr->roadID)
                {
                    alreadyFound = true;
                    break;
                }
            }
        }
        if (!alreadyFound)
        {
            connectedSections.push_back(connections[i].second->sections[connections[i].first]);
            isSharedSection = true;
        }
    }
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