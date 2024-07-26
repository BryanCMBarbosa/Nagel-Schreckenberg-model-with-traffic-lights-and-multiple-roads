#ifndef ROAD_H
#define ROAD_H

#include <vector>
#include <memory>
#include "RoadSection.h"
#include "RandomNumberGenerator.h"

class RoadSection;

class Road
{
public:
    int roadID;
    std::vector<std::shared_ptr<RoadSection>> sections;
    std::vector<Road*> connectedRoads;
    int maxSpeed;
    double brakeProb;
    double changingRoadProb;
    RandomNumberGenerator& rng;

    Road(int id, const std::vector<std::shared_ptr<RoadSection>>& sections, int maxSpd, double brakeP, double changingP, RandomNumberGenerator& generator);
    void setupSections();
    void simulateStep();
    void moveCars();
    int calculateDistanceToNextCarOrTrafficLight(const std::shared_ptr<RoadSection>& currentSection, int currentPosition);
    int calculateDistanceToNextCarOrTrafficLightFromStart(const std::shared_ptr<RoadSection>& section, int start);
    Road* decideTargetRoad(const std::shared_ptr<RoadSection>& section);
};

#endif
