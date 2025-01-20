#ifndef ROAD_H
#define ROAD_H

#include <vector>
#include <memory>
#include <iostream>
#include <random>
#include <limits>
#include "RoadSection.h"
#include "TrafficLight.h"
#include "RandomNumberGenerator.h"
#include "LimitedQueue.h"
#include "Dictionary.h"

class RoadSection;
class TrafficLight;

class Road : public std::enable_shared_from_this<Road>
{
public:
    int roadID;
    int roadSize;
    bool isPeriodic;
    double generalDensity;
    double averageDistanceHeadway;
    double averageSpeed;
    std::vector<std::shared_ptr<RoadSection>> sections;
    std::vector<std::shared_ptr<Road>> connectedRoads;
    double alpha;
    double beta;
    bool newCarInserted;
    int maxSpeed;
    double brakeProb;
    Dictionary<int, double> changingRoadProbs;
    int initialNumCars;
    double initialDensity;
    std::vector<int> carsPositions;
    std::vector<int> newCarsPositions;
    std::vector<int> trafficLightPositions;
    std::vector<int> sharedSectionsPositions;
    LimitedQueue<int> residenceTimes;
    LimitedQueue<int> travelTimes;
    LimitedQueue<double> averageTravelTimes;
    Dictionary<int, int> flowAtPoints;
    std::vector<int> timeHeadwayAndFlowPoints; //Indices of time headway points
    Dictionary<int, unsigned long long> lastTimestamps; //Last timestamp a car passed each point
    Dictionary<int, LimitedQueue<unsigned long long>> loggedTimeHeadways; //Logged time headways for each point
    std::vector<std::shared_ptr<TrafficLight>> trafficLights;
    RandomNumberGenerator& rng;

    Road(int id, int roadSize, bool isPeriodic, double beta, int maxSpd, double brakeP, int initialNumCars, RandomNumberGenerator& gen, int queueSize);
    Road(int id, int roadSize, bool isPeriodic, double beta, int maxSpd, double brakeP, double initialDensity, RandomNumberGenerator& gen, int queueSize);
    Road(const Road&) = delete;
    Road& operator=(const Road&) = delete;
    void setupSections();
    void simulateStep(unsigned long long currentTime);
    void moveCars();
    void calculateAverageTravelTime();
    void calculateGeneralDensity();
    double calculateRegionalDensity(int leftBoundary, int rightBoundary);
    bool didCarCrossPoint(int position, int newPosition, int measurementPoint);
    void calculateFlowAtPoints(int position, int newPosition);
    int calculateDistanceHeadwayBetweenTwoCars(int carIndex1, int carIndex2);
    void calculateAverageDistanceHeadway();
    void calculateAverageSpeed();
    void setupTimeHeadwayAndFlowPoints(int queueSize);
    void logTimeHeadways(unsigned long long currentTime);
    int measureQueueSize(int trafficLightIndex, int maxSpeedThreshold);
    const Dictionary<int, LimitedQueue<unsigned long long>>& getLoggedTimeHeadways() const;
    std::vector<int> getRoadRepresentation() const;
    std::vector<std::pair<int, int>> detectJams();
    void addCars(int numCars, int position = -1);
    void addCarsBasedOnDensity(double density);
    int calculateDistanceToNextCarOrTrafficLight(RoadSection& currentSection, int currentPosition, int distanceSharedSection);
    bool anyCarInSharedSection(RoadSection& section);
    int calculateDistanceToSharedSection(RoadSection& currentSection);
    std::pair<int, std::shared_ptr<Road>> decideTargetRoad(RoadSection& section);
    ~Road();
};

#endif
