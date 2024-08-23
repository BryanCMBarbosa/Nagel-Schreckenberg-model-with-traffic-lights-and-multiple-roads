#ifndef CAR_H
#define CAR_H

class Road;

class Car
{
public:
    int speed;
    int position;
    bool willChangeRoad;
    bool roadChangeDecisionMade;
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
