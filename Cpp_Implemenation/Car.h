#ifndef CAR_H
#define CAR_H

#include <utility>
#include <memory>
#include "Road.h"

class Road;

class Car : public std::enable_shared_from_this<Car>
{
public:
    int speed;
    int position;
    bool willChangeRoad;
    bool roadChangeDecisionMade;
    bool willSurpassSharedSection;
    char side;
    int distanceToSharedSection;
    int sharedSectionIndex;
    int originalRoadID; 
    std::pair<int, std::weak_ptr<Road>> indexAndTargetRoad;

    Car(int pos, int roadID);
    Car(Car&& other) noexcept;
    Car& operator=(Car&& other) noexcept;
    ~Car();
};

#endif
