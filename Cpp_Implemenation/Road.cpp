#include "Road.h"

Road::Road(int id, int roadSize, int maxSpd, double brakeP, double changingP, int initialNumCars, RandomNumberGenerator& gen)
    : roadID(id), roadSize(roadSize), maxSpeed(maxSpd), brakeProb(brakeP), changingRoadProb(changingP), initialNumCars(initialNumCars), rng(gen) 
{
}

void Road::setupSections()
{
    sections.reserve(roadSize);
    for (int i = 0; i < roadSize; i++)
    {
        sections.emplace_back(std::make_shared<RoadSection>(shared_from_this(), i));
    }
}

void Road::simulateStep()
{
    std::sort(carsPositions.begin(), carsPositions.end(), std::greater<int>());

    std::vector<int> newCarsPositions;

    for (auto& i : carsPositions)
    {
        auto& car = sections[i]->currentCar;

        if (car)
        {
            //Acceleration
            if (car->speed < maxSpeed)
            {
                car->speed++;
            }

            //Decision to change road
            int distanceSharedSection = calculateDistanceToSharedSection(*sections[i]);
            if (!car->roadChangeDecisionMade && car->speed >= distanceSharedSection)
            {
                if (rng.getRandomDouble() < changingRoadProb)
                {
                    car->indexAndTargetRoad = decideTargetRoad(*sections[(i + distanceSharedSection) % roadSize]);
                    car->willChangeRoad = (car->indexAndTargetRoad.first != -1);
                }
                else
                {
                    car->willChangeRoad = false;
                }
                car->roadChangeDecisionMade = true;
            }

            //Braking
            int distanceToNextCar = calculateDistanceToNextCarOrTrafficLight(*sections[i], i, distanceSharedSection);
            if (car->speed > distanceToNextCar)
            {
                car->speed = distanceToNextCar;
            }

            //Random slowing down
            if (car->speed > 0 && rng.getRandomDouble() < brakeProb)
            {
                car->speed--;
            }

            //Update willSurpassSharedSection
            if (car->speed >= distanceSharedSection)
            {
                car->willSurpassSharedSection = true;
            }
            else
            {
                car->willSurpassSharedSection = false;
            }
        }
    }

    moveCars();
}

void Road::addCars(int numCars, int position)
{
    if (position == -1)
    {
        std::uniform_int_distribution<> positionDist(0, roadSize - 1);
        int position;

        for (int i = 0; i < numCars; ++i)
        {
            do
            {
                position = positionDist(rng.getGenerator());
            } while (sections[position]->currentCar);
            
            sections[position]->currentCar = std::make_shared<Car>(position, roadID);
            carsPositions.push_back(position);
        }
    }
    else
    {
        if (!sections[position]->currentCar)
        {
            sections[position]->currentCar = std::make_shared<Car>(position, roadID);
            carsPositions.push_back(position);
        }
    }
}

void Road::moveCars()
{
    std::vector<int> newCarsPositions;

    std::sort(carsPositions.begin(), carsPositions.end(), std::greater<int>());

    for (auto& i : carsPositions)
    {
        auto& car = sections[i]->currentCar;

        if (car && car->speed > 0)
        {
            int newPos;
            if (car->willChangeRoad && car->willSurpassSharedSection)
            {
                int distanceToSharedSection = calculateDistanceToSharedSection(*sections[i]);
                int remainingMove = car->speed - distanceToSharedSection;
                std::shared_ptr<Road> newRoad = car->indexAndTargetRoad.second.lock();

                if (newRoad && newRoad->roadSize > 0)
                {
                    newPos = (car->indexAndTargetRoad.first + remainingMove) % newRoad->roadSize;

                    if (newRoad->sections[newPos]->currentCar == nullptr)
                    {
                        if (newRoad->sections[newPos]->trafficLight && !newRoad->sections[newPos]->trafficLight->state)
                        {
                            car->speed = 0;
                            newCarsPositions.push_back(i);
                            continue;
                        }

                        newRoad->sections[newPos]->currentCar = car;
                        car->position = newPos;
                        car->willChangeRoad = false;
                        car->roadChangeDecisionMade = false;
                        car->willSurpassSharedSection = false;

                        if (car->speed > newRoad->maxSpeed)
                        {
                            car->speed = newRoad->maxSpeed;
                        }
                        newRoad->carsPositions.push_back(newPos);
                        sections[i]->currentCar = nullptr;
                    }
                    else
                    {
                        car->speed = 0;
                        newCarsPositions.push_back(i);
                        continue;
                    }
                }
                else
                {
                    car->speed = 0;
                    newCarsPositions.push_back(i);
                    continue;
                }
            }
            else //Moving normally in the same road
            {
                newPos = (i + car->speed) % roadSize;

                if (sections[newPos]->currentCar == nullptr)
                {
                    if (sections[newPos]->trafficLight && !sections[newPos]->trafficLight->state)
                    {
                        car->speed = 0;
                        newCarsPositions.push_back(i);
                        continue;
                    }

                    sections[newPos]->currentCar = car;
                    car->position = newPos;
                    sections[i]->currentCar = nullptr;
                    newCarsPositions.push_back(newPos);
                }
                else
                {
                    car->speed = 0;
                    newCarsPositions.push_back(i);
                }
            }
        }
        else if (car && car->speed == 0)
        {
            newCarsPositions.push_back(i);
        }
        else
        {
            //No car at this position, nothing to do here
        }
    }
    carsPositions = std::move(newCarsPositions);
}


std::pair<int, std::shared_ptr<Road>> Road::decideTargetRoad(RoadSection& section)
{
    std::vector<std::pair<int, std::shared_ptr<Road>>> potentialRoads;

    for (auto& weakConnSection : section.connectedSections)
    {
        auto connSection = weakConnSection.lock();
        if (connSection)
        {
            auto connRoad = connSection->road.lock();
            if (connRoad && connRoad->roadID != roadID)
            {
                potentialRoads.emplace_back(connSection->index, connRoad);
            }
        }
    }

    if (!potentialRoads.empty())
    {
        std::uniform_int_distribution<> dist(0, potentialRoads.size() - 1);
        return potentialRoads[dist(rng.getGenerator())];
    }

    return std::make_pair(-1, std::shared_ptr<Road>());
}


int Road::calculateDistanceToSharedSection(RoadSection& currentSection)
{
    int distance = 0;
    int maxLookahead = maxSpeed;

    for (int d = 1; d <= maxLookahead; ++d)
    {
        int index = (currentSection.index + d) % roadSize;
        if (sections[index]->isSharedSection)
        {
            return d;
        }
    }

    return roadSize;
}


int Road::calculateDistanceToNextCarOrTrafficLight(RoadSection& currentSection, int currentPosition, int distanceSharedSection)
{
    int maxSpeed = currentSection.currentCar->speed;
    int distance = 0;

    for (int d = 1; d <= maxSpeed; ++d)
    {
        int index = (currentPosition + d) % roadSize;

        if (sections[index]->currentCar)
            return d - 1;

        if (sections[index]->trafficLight && !sections[index]->trafficLight->state)
        {
            if (index == (currentPosition + 1) % roadSize)
                return d - 1;
            else
                continue;
        }


        if (anyCarInSharedSection(*sections[index]))
            return d - 1;

        if (currentSection.currentCar->willChangeRoad && currentSection.currentCar->roadChangeDecisionMade && d == distanceSharedSection)
        {
            std::shared_ptr<Road> newRoad = currentSection.currentCar->indexAndTargetRoad.second.lock();
            if (newRoad)
            {
                int remainingMove = maxSpeed - distanceSharedSection;
                int newRoadIndex = (currentSection.currentCar->indexAndTargetRoad.first + remainingMove) % newRoad->roadSize;

                if (newRoad->sections[newRoadIndex]->currentCar)
                    return d - 1;

                if (newRoad->sections[newRoadIndex]->trafficLight && !newRoad->sections[newRoadIndex]->trafficLight->state)
                {
                    if (newRoadIndex == currentSection.currentCar->indexAndTargetRoad.first)
                        return d - 1;
                    else
                        continue;
                }

                if (anyCarInSharedSection(*newRoad->sections[newRoadIndex]))
                    return d - 1;
            }
            else
            {
                return d - 1;
            }
        }
    }

    return maxSpeed;
}

bool Road::anyCarInSharedSection(RoadSection& section)
{
    if (section.isSharedSection)
    {
        for (auto& weakConnSection : section.connectedSections)
        {
            auto connSection = weakConnSection.lock();
            if (connSection && connSection->currentCar)
                return true;
        }
    }
    return false;
}


Road::~Road()
{
}