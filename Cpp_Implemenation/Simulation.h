#ifndef SIMULATION_H
#define SIMULATION_H

#include <fstream>
#include <vector>
#include <set>
#include <unordered_map>
#include <memory>
#include <iostream>
#include <sstream>
#include <fstream>
#include <iomanip>
#include <random>
#include <chrono>
#include <thread>
#include <ctime>
#include <iomanip>
#include <nlohmann/json.hpp>
#include "Dictionary.h"
#include "RandomNumberGenerator.h"
#include "Road.h"
#include "TrafficLightGroup.h"
#include "TrafficLightController.h"
#include "SyncController.h"
#include "GreenWaveController.h"
#include "RandomOffsetController.h"
#include "TrafficVolumeGenerator.h"

class TrafficLightGroup;

class Simulation
{
private:
    nlohmann::json config;
    std::vector<std::shared_ptr<Road>> roads;
    std::vector<std::shared_ptr<TrafficLightGroup>> trafficLightGroups;
    RandomNumberGenerator rng;
    unsigned long long episodes;
    int currentHour;
    int currentDay;
    int currentMinute;
    int queueSize;
    int vMax;
    double brakeProbability;
    nlohmann::json simulationResults;
    std::vector<int> roadsWithAlpha;
    std::vector<int> roadsWithBeta;
    Dictionary<int, double> alphaWeights;
    std::shared_ptr<TrafficLightController> trafficLightController;
    short executionType;
    std::string resultsPath;

public:
    Simulation(const std::string& configFilePath, std::string resultsPath, short executionType);
    void setup();
    int countTotalCars() const;
    void printSimulationSettings() const;
    void run();
    void execute();
    void clearScreen() const;
    void printRoadStates() const;
    void createHeader();
    void collectMetrics(unsigned long long episode);
    void serializeResults(const std::string& filename) const; 
};

#endif
