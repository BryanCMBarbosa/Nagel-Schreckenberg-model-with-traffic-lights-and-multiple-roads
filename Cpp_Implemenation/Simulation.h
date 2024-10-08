#ifndef SIMULATION_H
#define SIMULATION_H

#include <fstream>
#include <nlohmann/json.hpp>
#include "RandomNumberGenerator.h"
#include "Road.h"
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

class Simulation
{
private:
    nlohmann::json config;
    std::vector<std::shared_ptr<Road>> roads;
    RandomNumberGenerator rng;
    unsigned long long episodes;
    bool undefinedDuration;

public:
    Simulation(const std::string& configFilePath);
    void setup();
    int countTotalCars() const;
    void printSimulationSettings() const;
    void run();
    void clearScreen() const;
    void printRoadStates() const; 
};

#endif
