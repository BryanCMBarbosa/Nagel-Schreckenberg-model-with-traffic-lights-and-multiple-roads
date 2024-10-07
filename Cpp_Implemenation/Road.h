#ifndef ROAD_H
#define ROAD_H

#include <vector>
#include <memory>
#include <iostream>
#include <random>
#include "RoadSection.h"
#include "RandomNumberGenerator.h"

class RoadSection;

class Road : public std::enable_shared_from_this<Road>
{
public:
    int roadID;
    int roadSize;
    std::vector<std::shared_ptr<RoadSection>> sections;
    std::vector<std::shared_ptr<Road>> connectedRoads;  //Roads that are connected through shared sections
    int maxSpeed;
    double brakeProb;
    double changingRoadProb;
    int initialNumCars;
    std::vector<int> carsPositions;
    RandomNumberGenerator& rng;

    Road(int id, int roadSize, int maxSpd, double brakeP, double changingP, int initialNumCars, RandomNumberGenerator& gen);
    Road(const Road&) = delete;
    Road& operator=(const Road&) = delete;
    void setupSections();
    void simulateStep();
    void moveCars();
    void addCars(int numCars, int position = -1);
    int calculateDistanceToNextCarOrTrafficLight(RoadSection& currentSection, int currentPosition, int distanceSharedSection);
    bool anyCarInSharedSection(RoadSection& section);
    int calculateDistanceToSharedSection(RoadSection& currentSection);
    std::pair<int, std::shared_ptr<Road>> decideTargetRoad(RoadSection& section);
    ~Road();
};

#endif
