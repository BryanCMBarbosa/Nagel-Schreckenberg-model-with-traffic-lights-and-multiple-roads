    #include "Road.h"
    #include <iostream>

    Road::Road(int id, const std::vector<std::shared_ptr<RoadSection>>& secs, int maxSpd, double brakeP, double changingP, RandomNumberGenerator& gen) 
        : roadID(id), sections(secs), maxSpeed(maxSpd), brakeProb(brakeP), changingRoadProb(changingP), rng(gen)
    {
    }

    void Road::setupSections()
    {
        for (auto& section : sections)
        {
            section->road = this;
        }
    }

    int calculateDistanceOnOtherRoad(const std::shared_ptr<RoadSection>& section, const std::vector<std::shared_ptr<RoadSection>>& otherSections)
    {
        for (int i = 1; i < otherSections.size(); ++i)
        {
            if (otherSections[i]->currentCar || !otherSections[i]->connectedSections.empty())
            {
                return i;
            }
        }
        return otherSections.size();
    }

    void Road::simulateStep() {
        for (int i = 0; i < sections.size(); ++i)
        {
            if (sections[i]->currentCar)
            {
                if (sections[i]->currentCar->speed < maxSpeed)
                {
                    sections[i]->currentCar->speed++;
                }

                if (sections[i]->isSharedSection && !sections[i]->currentCar->roadChangeDecisionMade)
                {
                    int distanceToShared = calculateDistanceToNextCarOrTrafficLight(sections[i], i);
                    if (distanceToShared <= sections[i]->currentCar->speed)
                    {
                        if (rng.getRandomDouble() < changingRoadProb)
                        {
                            sections[i]->currentCar->willChangeRoad = true;
                            sections[i]->currentCar->roadChangeDecisionMade = true;
                            sections[i]->currentCar->targetRoad = decideTargetRoad(sections[i]);
                        }
                    }
                }

                int distanceToNextCar = calculateDistanceToNextCarOrTrafficLight(sections[i], i);
                if (distanceToNextCar < sections[i]->currentCar->speed)
                {
                    sections[i]->currentCar->speed = distanceToNextCar;
                }

                if (rng.getRandomDouble() < brakeProb)
                {
                    sections[i]->currentCar->speed = std::max(0, sections[i]->currentCar->speed - 1);
                }
            }
        }

        moveCars();
    }


    void Road::moveCars()
    {
        std::vector<std::shared_ptr<RoadSection>> newPositions(sections.size(), nullptr);
        for (int i = 0; i < sections.size(); ++i)
        {
            if (sections[i]->currentCar && sections[i]->currentCar->speed > 0)
            {
                int newPos = (i + sections[i]->currentCar->speed) % sections.size();
                if (!newPositions[newPos])
                {
                    newPositions[newPos] = sections[i];
                    sections[i] = nullptr;
                }
            }
        }
        sections = newPositions;
    }

    Road* Road::decideTargetRoad(const std::shared_ptr<RoadSection>& section)
    {
        std::vector<Road*> possibleRoads;
        for (auto& connectedSection : section->connectedSections)
        {
            if (connectedSection->road != this)
            {
                possibleRoads.push_back(connectedSection->road);
            }
        }

        if (possibleRoads.empty())
        {
            return nullptr;
        }

        std::random_device rd;
        std::mt19937 eng(rd());
        std::uniform_int_distribution<> distr(0, possibleRoads.size() - 1);
        return possibleRoads[distr(eng)];
    }

    int Road::calculateDistanceToNextCarOrTrafficLight(const std::shared_ptr<RoadSection>& currentSection, int currentPosition)
    {
        int distance = 0;
        int count = sections.size();

        for (int i = 1; i < count; ++i)
        {
            int index = (currentPosition + i) % count;
            if (sections[index]->currentCar || (sections[index]->trafficLight && !sections[index]->trafficLight->state))
            {
                return i;
            }
            
            if (sections[index]->isSharedSection && sections[index]->currentCar && sections[index]->currentCar->willChangeRoad)
            {
                Road* targetRoad = sections[index]->currentCar->targetRoad;
                if (targetRoad)
                {
                    int targetRoadDistance = targetRoad->calculateDistanceToNextCarOrTrafficLightFromStart(targetRoad->sections[0], 0);
                    return i + targetRoadDistance;
                }
            }
        }

        return count;
    }

    int Road::calculateDistanceToNextCarOrTrafficLightFromStart(const std::shared_ptr<RoadSection>& section, int start)
    {
        for (int i = start; i < sections.size(); ++i)
        {
            if (sections[i]->currentCar || (sections[i]->trafficLight && !sections[i]->trafficLight->state))
            {
                return i - start;
            }
        }
        return sections.size();
    }
