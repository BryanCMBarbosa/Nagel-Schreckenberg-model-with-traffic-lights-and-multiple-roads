#include "Road.h"

Road::Road(int id, int roadSize, int maxSpd, double brakeP, double changingP, int initialNumCars, RandomNumberGenerator& gen)
    : roadID(id), roadSize(roadSize), maxSpeed(maxSpd), brakeProb(brakeP), changingRoadProb(changingP), initialNumCars(initialNumCars), rng(gen) 
{
    setupSections();
    addCars(initialNumCars);
    for (auto & s : sections)
    {
        if (s.currentCar)
        {
            std::cout << "There is a car here." << std::endl; //Confirms that there's a Car in the position
        }
    }
}

void Road::setupSections()
{
    sections.reserve(roadSize);
    for (int i = 0; i < roadSize; i++)
    {
        sections.emplace_back(this);
    }
}

void Road::simulateStep()
{
    for (int i = 0; i < sections.size(); ++i)
    {
        if (sections[i].currentCar)
        {
            //Acceleration logic
            if (sections[i].currentCar->speed < maxSpeed)
            {
                sections[i].currentCar->speed++;
            }

            //Decision logic for changing roads at shared sections
            if (!sections[i].currentCar->roadChangeDecisionMade && calculateDistanceToSharedSection(sections[i]) != -1 && rng.getRandomDouble() < changingRoadProb)
            {
                sections[i].currentCar->indexAndTargetRoad = decideTargetRoad(sections[i]);
                sections[i].currentCar->roadChangeDecisionMade = true;
            }

            //Braking logic
            int distanceToNextCar = calculateDistanceToNextCarOrTrafficLight(sections[i], i);
            if (distanceToNextCar < sections[i].currentCar->speed)
            {
                sections[i].currentCar->speed = distanceToNextCar;
            }

            //Random brake logics
            if (sections[i].currentCar->speed >= 0 && rng.getRandomDouble() < brakeProb)
            {
                sections[i].currentCar->speed--;
            }
        }
    }
    moveCars();
}

void Road::moveCars()
{
    for (int i = 0; i < sections.size(); ++i)
    {
        int newPos;
        if (sections[i].currentCar && sections[i].currentCar->willChangeRoad)
        {
            int distanceToSharedSection = calculateDistanceToSharedSection(sections[i]);
            if (sections[i].currentCar->speed > distanceToSharedSection)
            {
                int remainingMove = sections[i].currentCar->speed - distanceToSharedSection;
                newPos = (sections[i].currentCar->indexAndTargetRoad.first + remainingMove) % sections[i].currentCar->indexAndTargetRoad.second->roadSize;
                sections[i].currentCar->indexAndTargetRoad.second->sections[newPos].currentCar = std::move(sections[i].currentCar);
                sections[i].currentCar->indexAndTargetRoad.second->sections[newPos].currentCar->position = newPos;
                sections[i].currentCar = nullptr;
                sections[i].currentCar->indexAndTargetRoad.second->sections[newPos].currentCar->willChangeRoad = false;
                sections[i].currentCar->indexAndTargetRoad.second->sections[newPos].currentCar->roadChangeDecisionMade = false;
            }
        }
        else if (sections[i].currentCar && sections[i].currentCar->speed > 0)
        {
            newPos = (i + sections[i].currentCar->speed) % sections.size();
            sections[newPos].currentCar = std::move(sections[i].currentCar);
            sections[newPos].currentCar->position = newPos;
            sections[i].currentCar = nullptr;
        }
    }
}

void Road::addCars(int numCars, int position)
{
    if (position == -1)
    {
        std::uniform_int_distribution<> positionDist(0, roadSize - 1);

        for (int i = 0; i < numCars; ++i)
        {
            int position;
            do
            {
                position = positionDist(rng.getGenerator());
            } while (sections[position].currentCar);
            
            sections[position].currentCar = std::make_unique<Car>(position);
        }
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

int Road::calculateDistanceToNextCarOrTrafficLight(RoadSection& currentSection, int currentPosition)
{
    int distance = 0;
    int count = sections.size();  //Cache the size of the sections for efficiency.
    while (distance < count)
    {
        int index = (currentPosition + distance) % count;
        if (sections[index].currentCar || (sections[index].trafficLight && !sections[index].trafficLight->state) || anyCarInSharedSection(sections[index]))
        {
            return distance;  //Return the distance to the first car or red traffic light found.
        }
        ++distance;
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

