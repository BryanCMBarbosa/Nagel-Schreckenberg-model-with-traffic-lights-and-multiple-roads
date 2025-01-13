#include "Road.h"

Road::Road(int id, int roadSize, bool isPeriodic, double beta, int maxSpd, double brakeP, int initialNumCars, RandomNumberGenerator& gen, int queueSize = 100)
    : roadID(id), roadSize(roadSize), isPeriodic(isPeriodic), beta(beta), maxSpeed(maxSpd), brakeProb(brakeP), initialNumCars(initialNumCars), rng(gen), spaceAveragedFlow(queueSize), averageTravelTimes(queueSize), residenceTimes(queueSize), travelTimes(queueSize), averageSpeed(0.0), cumulativeTimeSpaceAveragedFlow(0.0) 
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

void Road::simulateStep(unsigned long long currentTime)
{
    if (!isPeriodic && sections[0]->connectedSections.empty() && rng.getRandomDouble() < alpha)
    {
        if (!sections[0]->currentCar)
        {
            auto newCar = std::make_shared<Car>(0, roadID);
            sections[0]->currentCar = newCar;
            carsPositions.push_back(0);
        }
    }

    std::sort(carsPositions.begin(), carsPositions.end(), std::greater<int>());
    std::vector<int> newCarsPositions;

    for (auto& i : carsPositions)
    {
        auto& car = sections[i]->currentCar;

        if (car)
        {
            //Increasing residence time for one time step more
            car->residenceTime++;

            //Acceleration
            if (car->speed < maxSpeed)
            {
                car->speed++;
            }

            //Decision to change road
            int distanceSharedSection = calculateDistanceToSharedSection(*sections[i]);
            if (!car->roadChangeDecisionMade && car->speed >= distanceSharedSection)
            {
                if (rng.getRandomDouble() < changingRoadProbs.get((i + distanceSharedSection) % roadSize))
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

            if (!isPeriodic && i + car->speed >= roadSize && !sections[roadSize-1]->isSharedSection)
            {
                car->speed = roadSize - 1 - i; //Adjust speed to prevent out-of-bound movement
            }
        }
    }

    //Metrics based on current state (before moving cars)
    calculateSpaceAveragedFlow();
    calculateCumulativeTimeSpaceAveragedFlow();
    calculateAverageTimeHeadway();
    logTimeHeadways(currentTime);

    //Move cars based on their current speeds
    moveCars();

    //For open boundary, verify if the car on the last section is going to be removed
    int lastSite = roadSize - 1;
    if (!isPeriodic)
    {
        auto& car = sections[lastSite]->currentCar;
        if (car)
        {
            bool carLeaves = rng.getRandomDouble() < beta;
            if (carLeaves)
            {
                residenceTimes.push(car->residenceTime);
                sections[lastSite]->currentCar = nullptr;
                carsPositions.erase(std::remove(carsPositions.begin(), carsPositions.end(), lastSite), carsPositions.end());
            }
        }
    }

    //Metrics based on updated state (after moving cars or removing from the road)
    calculateGeneralDensity();
    calculateAverageDistanceHeadway();
    calculateAverageSpeed();
}

void Road::calculateAverageTravelTime()
{
    averageTravelTimes.push(std::accumulate(travelTimes.begin(), travelTimes.end(), 0.0)/travelTimes.size());
}

void Road::calculateGeneralDensity()
{
    generalDensity = static_cast<double>(carsPositions.size()) / roadSize;  
}

double Road::calculateRegionalDensity(int leftBoundary, int rightBoundary)
{
    if (leftBoundary > rightBoundary || leftBoundary < 0 || rightBoundary >= roadSize)
    {
        throw std::invalid_argument("Invalid boundary values.");
    }

    int carCount = 0;
    int regionLength = rightBoundary - leftBoundary + 1;

    for (int position : carsPositions)
    {
        if (isPeriodic)
        {
            int wrappedPosition = (position + roadSize) % roadSize;
            if ((wrappedPosition >= leftBoundary && wrappedPosition <= rightBoundary) ||
                (leftBoundary > rightBoundary && (wrappedPosition >= leftBoundary || wrappedPosition <= rightBoundary)))
            {
                carCount++;
            }
        }
        else
        {
            if (position >= leftBoundary && position <= rightBoundary)
            {
                carCount++;
            }
        }
    }

    return static_cast<double>(carCount) / regionLength;
}

void Road::calculateSpaceAveragedFlow()
{
    int totalFlow = 0;

    for(auto& position : carsPositions)
    {
        auto& car = sections[position]->currentCar;
        if(car && car->speed > 0)
            totalFlow++;
    }

    spaceAveragedFlow.push(static_cast<double>(totalFlow) / roadSize);
}

void Road::calculateCumulativeTimeSpaceAveragedFlow()
{
    double flowSum = 0.0;
    flowSum = std::accumulate(spaceAveragedFlow.begin(), spaceAveragedFlow.end(), 0);
    cumulativeTimeSpaceAveragedFlow = flowSum / spaceAveragedFlow.size();
}

int Road::calculateDistanceHeadwayBetweenTwoCars(int carIndex1, int carIndex2)
{
    std::sort(carsPositions.begin(), carsPositions.end(), std::greater<int>());

    //Ensure indices are within bounds
    if (carIndex1 < 0 || carIndex1 > carsPositions.size() ||
        carIndex2 < 0 || carIndex2 > carsPositions.size())
        throw std::out_of_range("Car indices are out of range.");

    int position1 = carsPositions[carIndex1];
    int position2 = carsPositions[carIndex2];

    return (position2 - position1 + roadSize) % roadSize;
}

void Road::calculateAverageDistanceHeadway()
{
    std::sort(carsPositions.begin(), carsPositions.end(), std::greater<int>());

    if (carsPositions.size() < 2)
    {
        averageDistanceHeadway = std::numeric_limits<double>::infinity(); //No meaningful headway if fewer than two cars
    }
    else
    {
        double totalHeadway = 0.0;

        for (size_t i = 0; i < carsPositions.size() - 1; i++)
        {
            totalHeadway += calculateDistanceHeadwayBetweenTwoCars(i, i + 1);
        }

        averageDistanceHeadway = totalHeadway / (carsPositions.size() - 1);
    }
}

void Road::calculateAverageTimeHeadway()
{
    if (spaceAveragedFlow.empty())
    {
        averageTimeHeadway = 0.0;
        return;
    }

    double totalHeadway = 0.0;

    for (double flow : spaceAveragedFlow)
    {
        if (flow > 0) //Avoid division by zero
        {
            totalHeadway += 1.0 / flow;
        }
        else
        {
            totalHeadway += std::numeric_limits<double>::infinity(); //Infinite headway for zero flow
        }
    }

    averageTimeHeadway = totalHeadway / spaceAveragedFlow.size();
}

void Road::setupTimeHeadwayPoints(int queueSize)
{
    timeHeadwayPoints.clear();

    //General point in the middle of the road
    timeHeadwayPoints.push_back(roadSize / 2);

    //Points 4 sites before each traffic light
    for (int position : trafficLightPositions)
    {
        int point;
        if (isPeriodic)
            point = (position - 4 + roadSize) % roadSize; //Handle periodic wrapping
        else
        {
            point = position - 4; //No wrapping for open boundaries
            if (point < 0) continue; //Skip invalid points for open boundaries
        }

        if (std::find(timeHeadwayPoints.begin(), timeHeadwayPoints.end(), point) == timeHeadwayPoints.end())
            timeHeadwayPoints.push_back(point); //Avoid duplicates
    }

    //Initialize last timestamps and queues for all points
    for (int point : timeHeadwayPoints)
    {
        lastTimestamps.add(point, std::numeric_limits<unsigned long long>::max()); //No car has passed yet
        loggedTimeHeadways.add(point, LimitedQueue<unsigned long long>(queueSize));
    }
}

void Road::logTimeHeadways(unsigned long long currentTime)
{
    for (int point : timeHeadwayPoints)
    {
        auto& section = sections[point];
        if (section->currentCar)
        {
            //A car is passing this point
            if (lastTimestamps.get(point) != std::numeric_limits<unsigned long long>::max())
            {
                //Calculate time headway
                unsigned long long timeHeadway = currentTime - lastTimestamps.get(point);
                loggedTimeHeadways.get(point).push(timeHeadway); // Add to point-specific queue
            }
            //Update the last timestamp
            lastTimestamps.add(point, currentTime);
        }
    }
}

const Dictionary<int, LimitedQueue<unsigned long long>>& Road::getLoggedTimeHeadways() const
{
    return loggedTimeHeadways;
}

void Road::calculateAverageSpeed()
{
    int speedSum = 0;
    for(auto& position : carsPositions)
    {
        speedSum += sections[position]->currentCar->speed;
    }
    averageSpeed = static_cast<double>(speedSum) / carsPositions.size();
}

std::vector<int> Road::getRoadRepresentation() const
{
    std::vector<int> representation(roadSize, -1);

    for (int position : carsPositions)
    {
        auto& car = sections[position]->currentCar;
        if (car)
        {
            representation[position] = car->speed;
        }
    }

    return representation;
}

std::vector<std::pair<int, int>> Road::detectJams()
{
    std::vector<std::pair<int, int>> jams;
    if (carsPositions.size() >= 3)
    {
        std::sort(carsPositions.begin(), carsPositions.end());

        std::vector<std::pair<int, int>> jams; //starting position, size of jam
        int consecutiveStoppedCars = 1;
        int firstCarInJam = carsPositions[0];

        for (size_t i = 1; i < carsPositions.size(); i++)
        {
            int distance = (carsPositions[i] - carsPositions[i - 1]);

            if (distance == 1)
            {
                auto& prevCar = sections[carsPositions[i - 1]]->currentCar;
                auto& currCar = sections[carsPositions[i]]->currentCar;

                if (prevCar && currCar && prevCar->speed == 0 && currCar->speed == 0)
                {
                    consecutiveStoppedCars++;

                    if (consecutiveStoppedCars == 3)
                    {
                        jams.push_back({firstCarInJam, consecutiveStoppedCars});
                    }
                    else if (consecutiveStoppedCars > 3)
                    {
                        jams.back().second = consecutiveStoppedCars;
                    }
                }
                else
                {
                    consecutiveStoppedCars = 1;
                    firstCarInJam = carsPositions[i];
                }
            }
            else
            {
                consecutiveStoppedCars = 1;
                firstCarInJam = carsPositions[i];
            }
        }

        if (isPeriodic)
        {
            int circularDistance = (carsPositions[0] - carsPositions.back() + roadSize) % roadSize;
            if (circularDistance == 1)
            {
                auto& lastCar = sections[carsPositions.back()]->currentCar;
                auto& firstCar = sections[carsPositions[0]]->currentCar;

                if (lastCar && firstCar && lastCar->speed == 0 && firstCar->speed == 0)
                {
                    consecutiveStoppedCars++;
                    if (consecutiveStoppedCars >= 3)
                    {
                        if (!jams.empty() && jams.back().first == firstCarInJam)
                        {
                            jams.back().second = consecutiveStoppedCars;
                        }
                        else
                        {
                            jams.push_back({firstCarInJam, consecutiveStoppedCars});
                        }
                    }
                }
            }
        }
    }
    return jams;
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
        calculateGeneralDensity();
        calculateAverageDistanceHeadway();
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
                        travelTimes.push(car->timeOnCurrentRoad);
                        car->timeOnCurrentRoad = 0;
                        calculateAverageTravelTime();

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

    for (int d = 1; d <= maxSpeed; ++d)
    {
        int index = (currentPosition + d) % roadSize;

        if (sections[index]->currentCar)
            return d - 1;

        if (sections[index]->trafficLight && !sections[index]->trafficLight->state)
            return d;

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
                        return d;
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