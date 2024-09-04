#include "Road.h"

Road::Road(int id, int roadSize, int maxSpd, double brakeP, double changingP, int initialNumCars, RandomNumberGenerator& gen)
    : roadID(id), roadSize(roadSize), maxSpeed(maxSpd), brakeProb(brakeP), changingRoadProb(changingP), initialNumCars(initialNumCars), rng(gen) 
{
    setupSections();
    addCars(initialNumCars);
}

void Road::setupSections()
{
    sections.reserve(roadSize);
    for (int i = 0; i < roadSize; i++)
    {
        sections.emplace_back(this, i);
    }
}

void Road::simulateStep()
{
    for (auto& i : carsPositions)
    {
        if (sections[i].currentCar)
        {
            //Acceleration logic
            if (sections[i].currentCar->speed < maxSpeed)
            {
                sections[i].currentCar->speed++;
            }

            //Decision logic for changing roads at shared sections
            int distanceSharedSection = calculateDistanceToSharedSection(sections[i]);
            if (!sections[i].currentCar->roadChangeDecisionMade && distanceSharedSection != -1 && rng.getRandomDouble() < changingRoadProb)
            {
                sections[i].currentCar->indexAndTargetRoad = decideTargetRoad(sections[i]);
                sections[i].currentCar->roadChangeDecisionMade = true;
                if (sections[i].currentCar->indexAndTargetRoad.first == -1)
                    sections[i].currentCar->willChangeRoad = false;
                else
                    sections[i].currentCar->willChangeRoad = true;
            }

            //Braking logic
            int distanceToNextCar = calculateDistanceToNextCarOrTrafficLight(sections[i], i, distanceSharedSection);
            if (sections[i].currentCar->speed > distanceToNextCar)
            {
                sections[i].currentCar->speed = distanceToNextCar;
            }

            //Random brake logics
            if (sections[i].currentCar->speed > 0 && rng.getRandomDouble() < brakeProb)
            {
                sections[i].currentCar->speed--;
            }

            if (sections[i].currentCar->speed > distanceSharedSection)
            {
                sections[i].currentCar->willSurpassSharedSection = true;
            }
        }
    }
    moveCars();
}

void Road::moveCars()
{
    std::vector<int> newCarsPositions;

    for (auto& i : carsPositions)
    {
        int newPos;
        if (sections[i].currentCar and sections[i].currentCar->speed > 0)
        {
            if (sections[i].currentCar->willChangeRoad and sections[i].currentCar->willSurpassSharedSection)
            {
                int remainingMove = sections[i].currentCar->speed - calculateDistanceToSharedSection(sections[i]);
                Road* newRoad = sections[i].currentCar->indexAndTargetRoad.second;
                newPos = (sections[i].currentCar->indexAndTargetRoad.first + remainingMove) % newRoad->roadSize;
                newRoad->sections[newPos].currentCar = std::move(sections[i].currentCar);
                newRoad->sections[newPos].currentCar->position = newPos;
                sections[i].currentCar = nullptr;
                newRoad->sections[newPos].currentCar->willChangeRoad = false;
                newRoad->sections[newPos].currentCar->roadChangeDecisionMade = false;
                newRoad->sections[newPos].currentCar->willSurpassSharedSection = false;
                
                if (newRoad->sections[newPos].currentCar->speed > newRoad->maxSpeed)
                    newRoad->sections[newPos].currentCar->speed = newRoad->maxSpeed;

                newRoad->carsPositions.push_back(newPos);
                //auto indexIt = std::find(carsPositions.begin(), carsPositions.end(), i);
                //carsPositions.erase(indexIt);
            }
            else
            {
                newPos = (i + sections[i].currentCar->speed) % sections.size();
                sections[newPos].currentCar = std::move(sections[i].currentCar);
                sections[newPos].currentCar->position = newPos;
                sections[i].currentCar = nullptr;

                if (sections[newPos].currentCar->willSurpassSharedSection)
                {
                    sections[newPos].currentCar->willChangeRoad = false;
                    sections[newPos].currentCar->roadChangeDecisionMade = false;
                    sections[newPos].currentCar->willSurpassSharedSection = false;
                }

                newCarsPositions.push_back(newPos);
            }
        }
        else if (sections[i].currentCar and sections[i].currentCar->speed == 0)
        {
            newCarsPositions.push_back(i);
        }
    }
    carsPositions.clear();
    carsPositions = std::move(newCarsPositions);
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
            } while (sections[position].currentCar);
            
            sections[position].currentCar = std::make_unique<Car>(position);
            carsPositions.push_back(position);
        }
        std::sort(carsPositions.begin(), carsPositions.end());
    }
    else
    {
        if (!sections[position].currentCar)
        {
            sections[position].currentCar = std::make_unique<Car>(position);
        }
    }
}


std::pair<int, Road*> Road::decideTargetRoad(RoadSection& section)
{
    std::vector<std::pair<int, Road*>> potentialRoads;
    for (auto& connectedSection : section.connectedSections)
    {
        if (connectedSection->road != this)
        {
            potentialRoads.push_back(std::make_pair(connectedSection->index, connectedSection->road));
        }
    }
    if (!potentialRoads.empty())
    {
        std::uniform_int_distribution<> dist(0, potentialRoads.size() - 1);
        return potentialRoads[dist(rng.getGenerator())]; //Randomly select a pair <Shared Section index, connected road>
    }
    return std::make_pair(-1, nullptr);
}

int Road::calculateDistanceToSharedSection(RoadSection& currentSection)
{
    int speed = currentSection.currentCar->speed;
    int index = currentSection.index;
    int distance = 0;
    while (speed > 0)
    {
        distance++;
        index = (currentSection.index + distance) % roadSize;
        if (sections[index].isSharedSection)
            return distance;
        speed--;
    }
    return -1;
}

int Road::calculateDistanceToNextCarOrTrafficLight(RoadSection& currentSection, int currentPosition, int distanceSharedSection)
{
    int distance = 0;
    int count = sections.size();  //Cache the size of the sections for efficiency.
    int index;
    if (currentSection.currentCar->roadChangeDecisionMade and currentSection.currentCar->willChangeRoad and currentSection.currentCar->indexAndTargetRoad.first != -1)
    {
        while (distance <= distanceSharedSection)
        {
            index = (currentPosition + distance + 1) % count;
            if (sections[index].currentCar || (sections[index].trafficLight && !sections[index].trafficLight->state) || anyCarInSharedSection(sections[index]))
            {
                return distance;  //Return the distance to the first car or red traffic light found.
            }
            ++distance;
        }
        int speed = sections[index].currentCar->speed;
        int remainingMove = speed - distanceSharedSection;
        if (remainingMove > 0)
        {
            Road* newRoad = sections[index].currentCar->indexAndTargetRoad.second;
            int distanceInNewRoad = 0;
            index = (sections[index].currentCar->indexAndTargetRoad.first + distanceInNewRoad + 1) % newRoad->sections.size();
            while (distance < speed)
            {
                if (newRoad->sections[index].currentCar || (newRoad->sections[index].trafficLight && !newRoad->sections[index].trafficLight->state) || anyCarInSharedSection(newRoad->sections[index]))
                {
                    return distance;
                }
                ++distance;
                ++distanceInNewRoad;
            }
        }
    }
    else
    {
        while (distance < count)
        {
            index = (currentPosition + distance + 1) % count;
            if (sections[index].currentCar || (sections[index].trafficLight && !sections[index].trafficLight->state) || anyCarInSharedSection(sections[index]))
            {
                return distance;  //Return the distance to the first car or red traffic light found.
            }
            ++distance;
        }
    }
    
    return count; //Return the road length if no obstacles are found.
}

bool Road::anyCarInSharedSection(RoadSection& section)
{
    if (section.isSharedSection)
    {
        int connSectionsNumber = section.connectedSections.size();
        for (int i = 0; i < connSectionsNumber; i++)
        {
            if (section.connectedSections[i]->currentCar)
            {
                return true;
            }
        }
    }
    return false;
}