#include "Simulation.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <random>

Simulation::Simulation(const std::string& configFilePath)
{
    std::ifstream file(configFilePath);
    if (file.is_open())
    {
        file >> config;
        file.close();
    }
    else
    {
        throw std::runtime_error("Unable to open configuration file.");
    }
}

void Simulation::setup()
{
    if (!config.contains("simulation") || !config["simulation"].contains("roads"))
    {
        throw std::runtime_error("Invalid configuration: missing 'simulation' or 'roads' key.");
    }

    const auto& roadsConfig = config["simulation"]["roads"];
    std::cout << "roadsConfig Size: " << roadsConfig.size() << std::endl;

    for (const auto& roadConfig : roadsConfig)
    {
        int roadID = roadConfig.value("roadID", 0);
        int roadSize = roadConfig.value("roadSize", 0);

        int numCars = 0;
        if (roadConfig.contains("numCars"))
        {
            numCars = roadConfig["numCars"];
        }

        int maxSpeed = roadConfig.value("maxSpeed", 5);
        double brakeProb = roadConfig.value("brakeProbability", 0.1);
        double changeProb = roadConfig.value("changingRoadProbability", 0.15);

        roads.emplace_back(roadID, roadSize, maxSpeed, brakeProb, changeProb, numCars, rng);
    }

    printSimulationSettings();
}

void Simulation::printSimulationSettings() const
{
    std::cout << "Simulation Settings:\n";
    std::cout << std::left << std::setw(20) << "Parameter" << std::setw(15) << "Value\n";
    std::cout << std::string(35, '-') << "\n";

    for (const auto& road : roads)
    {
        std::cout << std::setw(20) << "Road ID" << road.roadID << "\n";
        std::cout << std::setw(20) << "Road Size" << road.sections.size() << "\n";
        std::cout << std::setw(20) << "Max Speed" << road.maxSpeed << "\n";
        std::cout << std::setw(20) << "Brake Probability" << road.brakeProb << "\n";
        std::cout << std::setw(20) << "Changing Road Prob" << road.changingRoadProb << "\n";
        std::cout << std::setw(20) << "Connected Roads";
        for (auto connectedRoad : road.connectedRoads)
        {
            std::cout << connectedRoad->roadID << " ";
        }
        std::cout << "\n";
    }
}

void Simulation::run()
{
    std::cout << "Running simulation...\n";
    for (auto& road : roads)
    {
        printRoadStates();
        road.simulateStep();
    }
}

void Simulation::printRoadStates() const
{
    int numberOfRoads = roads.size();
    std::cout << "Road States at Current Simulation Step:\n";
    for (int roadIndex = 0; roadIndex < numberOfRoads; roadIndex++)
    {
        std::cout << "Road ID: " << roads[roadIndex].roadID << ", Size: " << roads[roadIndex].sections.size() << "\n";

        std::stringstream tlLine;  //Line for traffic lights
        std::stringstream carLine; //Line for cars

        int roadSize = roads[roadIndex].sections.size();
        for (int sectionIndex = 0; sectionIndex < roadSize; sectionIndex++)
        {
            if (roads[roadIndex].sections[sectionIndex].trafficLight)
            {
                tlLine << (roads[roadIndex].sections[sectionIndex].trafficLight->state ? "[O]" : "[C]");
            }
            else
            {
                tlLine << "   ";  //No traffic light in this section
            }

            // Car status
            if (roads[roadIndex].sections[sectionIndex].currentCar)
            {
                carLine << " 1 ";  //Car is present
            }
            else
            {
                carLine << " _ ";  //No car in this section
            }
        }

        std::cout << std::setw(6) << tlLine.str() << "\n";
        std::cout << std::setw(6) << carLine.str() << "\n\n";
    }
}

