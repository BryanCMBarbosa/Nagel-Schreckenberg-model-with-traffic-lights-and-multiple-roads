#include "Car.h"
#include "Road.h"

Car::Car(int pos) 
    : position(pos), 
      speed(0), 
      willChangeRoad(false), 
      roadChangeDecisionMade(false),
      willSurpassSharedSection(false), 
      indexAndTargetRoad(0, nullptr)
      {}

Car::Car(Car&& other) noexcept 
    : speed(other.speed), 
      position(other.position), 
      willChangeRoad(other.willChangeRoad), 
      roadChangeDecisionMade(other.roadChangeDecisionMade), 
      willSurpassSharedSection(other.willSurpassSharedSection),
      side(other.side), 
      distanceToSharedSection(other.distanceToSharedSection), 
      sharedSectionIndex(other.sharedSectionIndex), 
      indexAndTargetRoad(std::move(other.indexAndTargetRoad))
{
    //Reset other object to a valid state
    other.indexAndTargetRoad = std::make_pair(0, nullptr);
}

Car& Car::operator=(Car&& other) noexcept 
{
    if (this != &other) 
    {
        speed = other.speed;
        position = other.position;
        willChangeRoad = other.willChangeRoad;
        roadChangeDecisionMade = other.roadChangeDecisionMade;
        willSurpassSharedSection = other.willSurpassSharedSection;
        side = other.side;
        distanceToSharedSection = other.distanceToSharedSection;
        sharedSectionIndex = other.sharedSectionIndex;
        indexAndTargetRoad = std::move(other.indexAndTargetRoad);

        other.indexAndTargetRoad = std::make_pair(0, nullptr);
    }
    return *this;
}

Car::~Car() 
{
}
