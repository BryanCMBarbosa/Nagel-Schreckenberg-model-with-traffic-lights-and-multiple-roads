#include "Road.h"

Road::Road(int id, int roadSize, bool isPeriodic, double beta, int maxSpd, double brakeP, int initialNumCars, RandomNumberGenerator& gen, int queueSize = 100)
    : roadID(id), roadSize(roadSize), isPeriodic(isPeriodic), beta(beta), newCarInserted(false), maxSpeed(maxSpd), brakeProb(brakeP), initialNumCars(initialNumCars), rng(gen), averageTravelTimes(queueSize), residenceTimes(queueSize), travelTimes(queueSize), averageSpeed(0.0)
{
}

Road::Road(int id, int roadSize, bool isPeriodic, double beta, int maxSpd, double brakeP, double initialDensity, RandomNumberGenerator& gen, int queueSize = 100)
    : roadID(id), roadSize(roadSize), isPeriodic(isPeriodic), beta(beta), newCarInserted(false), maxSpeed(maxSpd), brakeProb(brakeP), initialDensity(initialDensity), rng(gen), averageTravelTimes(queueSize), residenceTimes(queueSize), travelTimes(queueSize), averageSpeed(0.0)
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
            carsPositions.insert(carsPositions.begin(), 0);
            newCarInserted = true;
        }
        else
            newCarInserted = false;
    }
    else
        newCarInserted = false;

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

bool Road::didCarCrossPoint(int position, int newPosition, int measurementPoint)
{
    if (isPeriodic)
    {
        if (position < newPosition)
            return (position < measurementPoint && newPosition >= measurementPoint);
        else if (position > newPosition)
            return (position < measurementPoint) || (newPosition >= measurementPoint);
    }
    else
        return (position < measurementPoint && newPosition >= measurementPoint);
    
    return false;
}

void Road::calculateFlowAtPoints(int position, int newPosition)
{
    if (position < 0 || position >= roadSize || newPosition < 0)
        return;

    for (auto& point : timeHeadwayAndFlowPoints)
    {
        if(didCarCrossPoint(position, newPosition, point))
            flowAtPoints.increment(point, 1);
    }
}

int Road::calculateDistanceHeadwayBetweenTwoCars(int carIndex1, int carIndex2)
{
    //std::sort(carsPositions.begin(), carsPositions.end(), std::greater<int>());

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
    //std::sort(carsPositions.begin(), carsPositions.end(), std::greater<int>());

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

void Road::setupTimeHeadwayAndFlowPoints(int queueSize)
{
    timeHeadwayAndFlowPoints.clear();

    //General point in the middle of the road
    timeHeadwayAndFlowPoints.push_back(roadSize / 2);

    //Points 4 sites before each traffic light
    for (int position : trafficLightPositions)
    {
        int point;
        if (isPeriodic)
            point = (position - 4 + roadSize) % roadSize; //Handle periodic wrapping
        else
        {
            point = position - 4; //No wrapping for open boundaries
            while (point < 0)
                point++;
        }

        if (std::find(timeHeadwayAndFlowPoints.begin(), timeHeadwayAndFlowPoints.end(), point) == timeHeadwayAndFlowPoints.end())
            timeHeadwayAndFlowPoints.push_back(point);
    }

    //Initialize last timestamps and queues for all points
    for (int point : timeHeadwayAndFlowPoints)
    {
        flowAtPoints.add(point, 0);
        lastTimestamps.add(point, std::numeric_limits<unsigned long long>::max()); //No car has passed yet
        loggedTimeHeadways.add(point, LimitedQueue<unsigned long long>(queueSize));
    }
}

void Road::logTimeHeadways(unsigned long long currentTime)
{
    for (int point : timeHeadwayAndFlowPoints)
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

int Road::measureQueueSize(int trafficLightIndex, int maxSpeedThreshold = 1)
{
    int queueSize = 0;
    auto wrapIndex = [&](int index) -> int
    {
        return (index + roadSize) % roadSize;
    };

    int currentIndex = isPeriodic ? wrapIndex(trafficLightIndex - 1) : trafficLightIndex - 1;

    while (true)
    {
        const auto& section = sections[isPeriodic ? wrapIndex(currentIndex) : currentIndex];

        if (currentIndex != trafficLightIndex - 1 && section->trafficLight)
            break;

        if (section->currentCar)
        {
            if (section->currentCar->speed <= maxSpeedThreshold)
                queueSize++;
            else
                break;
        }
        else
            break;

        currentIndex--;

        if (!isPeriodic && currentIndex < 0)
            break;
    }

    if (isPeriodic)
    {
        currentIndex = trafficLightIndex;
        while (true)
        {
            const auto& section = sections[wrapIndex(currentIndex)];

            if (section->trafficLight)
                break;

            if (section->currentCar)
            {
                if (section->currentCar->speed <= maxSpeedThreshold)
                    queueSize++;
                else
                    break;
            }
            else
                break;

            currentIndex++;
            if (wrapIndex(currentIndex) == wrapIndex(trafficLightIndex - 1))
                break;
        }
    }

    return queueSize;
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
        std::vector<int> positions(roadSize);
        std::iota(positions.begin(), positions.end(), 0);
        std::shuffle(positions.begin(), positions.end(), rng.getGenerator());

        for (int i = 0; i < numCars; ++i)
        {
            int selectedPosition = positions[i];
            sections[selectedPosition]->currentCar = std::make_shared<Car>(selectedPosition, roadID);
            carsPositions.push_back(selectedPosition);
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

    std::sort(carsPositions.begin(), carsPositions.end());

    calculateGeneralDensity();
    initialDensity = generalDensity;
    calculateAverageDistanceHeadway();
}

void Road::addCarsBasedOnDensity(double density)
{
    if (density < 0.0 || density > 1.0) 
        std::cerr << "Density can not be smaller than 0.0 or bigger than 1.0." << std::endl;

    initialNumCars = static_cast<int>(std::trunc(roadSize*density));

    addCars(static_cast<int>(std::trunc(roadSize*density)));
}

void Road::moveCars()
{
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

                if (sections[newPos]->currentCar)
                    std::cout << "There is already a car in the new position" << std::endl;

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
                    calculateFlowAtPoints(i, newPos);
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
    newCarsPositions.clear();
    std::sort(carsPositions.begin(), carsPositions.end(), std::greater<int>());
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