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
#include <iomanip>

class Simulation
{
private:
    nlohmann::json config;
    std::vector<Road> roads;
    RandomNumberGenerator rng;

public:
    Simulation(const std::string& configFilePath);
    void setup();
    void printSimulationSettings() const;
    void run();
    void printRoadStates() const; 
};

#endif
