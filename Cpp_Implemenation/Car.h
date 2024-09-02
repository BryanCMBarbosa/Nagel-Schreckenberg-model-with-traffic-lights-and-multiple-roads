#ifndef CAR_H
#define CAR_H

#include <utility>

class Road;

class Car
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
    std::pair<int, Road*> indexAndTargetRoad;

    Car(int pos);
    Car(Car&& other) noexcept;
    Car& operator=(Car&& other) noexcept;
    ~Car();
};

#endif
