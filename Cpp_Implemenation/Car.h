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
    Road* targetRoad;

    Car(int pos);
};

#endif
