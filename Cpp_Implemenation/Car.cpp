#include "Car.h"
#include "Road.h"

Car::Car(int pos) : position(pos), speed(0), willChangeRoad(false), roadChangeDecisionMade(false), targetRoad(nullptr)
{
}
