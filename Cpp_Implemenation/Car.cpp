#include "Car.h"
#include "Road.h"

Car::Car(int pos, int roadID) 
    : position(pos), 
      speed(0), 
      willChangeRoad(false), 
      roadChangeDecisionMade(false),
      willSurpassSharedSection(false), 
      indexAndTargetRoad(-1, std::weak_ptr<Road>()),
      originalRoadID(roadID)
{
}

Car::Car(Car&& other) noexcept 
    : speed(other.speed), 
      position(other.position), 
      willChangeRoad(other.willChangeRoad), 
      roadChangeDecisionMade(other.roadChangeDecisionMade), 
      willSurpassSharedSection(other.willSurpassSharedSection),
      side(other.side), 
      distanceToSharedSection(other.distanceToSharedSection), 
      sharedSectionIndex(other.sharedSectionIndex), 
      indexAndTargetRoad(std::move(other.indexAndTargetRoad)),
      originalRoadID(other.originalRoadID)
{
    other.indexAndTargetRoad = std::make_pair(-1, std::weak_ptr<Road>());
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
        originalRoadID = other.originalRoadID;

        other.indexAndTargetRoad = std::make_pair(-1, std::weak_ptr<Road>());
    }
    return *this;
}

Car::~Car() 
{
}
