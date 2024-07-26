#include "RoadSection.h"
#include <algorithm>

RoadSection::RoadSection() : currentCar(nullptr), isSharedSection(false), road(nullptr)
{
}

RoadSection::~RoadSection()
{
    delete currentCar;
}

void RoadSection::connect(std::shared_ptr<RoadSection> otherSection)
{
    if (otherSection && std::find(connectedSections.begin(), connectedSections.end(), otherSection) == connectedSections.end())
    {
        connectedSections.push_back(otherSection);
        otherSection->connectedSections.push_back(shared_from_this());  //Mutual connection
        isSharedSection = true;
        otherSection->isSharedSection = true;
    }
}

void RoadSection::disconnect(std::shared_ptr<RoadSection> otherSection)
{
    auto it = std::find(connectedSections.begin(), connectedSections.end(), otherSection);
    if (it != connectedSections.end())
    {
        connectedSections.erase(it);
        auto it_other = std::find(otherSection->connectedSections.begin(), otherSection->connectedSections.end(), shared_from_this());
        if (it_other != otherSection->connectedSections.end())
        {
            otherSection->connectedSections.erase(it_other);
        }
    }
}
