#include "Simulation.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>

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
    std::cout << "JSON FILE: " << std::endl;
    std::cout << ":::::::::::::::::::::::::::::::::::::::::::::::::" << std::endl;
    std::cout << config << std::endl;
    std::cout << ":::::::::::::::::::::::::::::::::::::::::::::::::" << std::endl << std::endl << std::endl << std::endl;
    setup();
}

void Simulation::setup()
{
    if (!config.contains("simulation") || !config["simulation"].contains("roads"))
    {
        throw std::runtime_error("Invalid configuration: missing 'simulation' or 'roads' key.");
    }

    const auto& roadsConfig = config["simulation"]["roads"];
    for (const auto& roadConfig : roadsConfig)
    {
        int roadID = roadConfig.value("roadID", 0);
        int roadSize = roadConfig.value("roadSize", 0);

        roadSectionsMap[roadID].resize(roadSize, nullptr);
        for (int i = 0; i < roadSize; ++i)
        {
            roadSectionsMap[roadID][i] = std::make_shared<RoadSection>();
        }
    }

    for (const auto& roadConfig : roadsConfig)
    {
        if (!roadConfig.contains("sections"))
            continue;

        int roadID = roadConfig["roadID"];

        for (const auto& sectionConfig : roadConfig["sections"])
        {
            if (!sectionConfig.contains("index") || !sectionConfig.contains("sharedWith")) continue;
            int index = sectionConfig["index"];

            auto& section = roadSectionsMap[roadID][index];
            for (const auto& shared : sectionConfig["sharedWith"])
            {
                int sharedRoadID = shared.value("roadID", 0);
                int sharedIndex = shared.value("sectionIndex", 0);

                auto& sharedSection = roadSectionsMap[sharedRoadID][sharedIndex];
                section->connect(sharedSection);
            }
        }
    }

    for (const auto& roadConfig : roadsConfig)
    {
        int roadID = roadConfig["roadID"];
        std::vector<std::shared_ptr<RoadSection>> sections = roadSectionsMap[roadID];

        if (sections.empty())
            continue;

        int maxSpeed = roadConfig.value("maxSpeed", 5);
        double brakeProb = roadConfig.value("brakeProbability", 0.1);
        double changeProb = roadConfig.value("changingRoadProbability", 0.15);

        Road newRoad(roadID, sections, maxSpeed, brakeProb, changeProb, rng);
        roads.push_back(std::move(newRoad));
    }
    printSimulationSettings();
}

void Simulation::printSimulationSettings() const
{
    std::cout << "Simulation Settings:" << std::endl;
    std::cout << std::left << std::setw(20) << "Parameter" << std::setw(15) << "Value" << std::endl;
    std::cout << std::string(35, '-') << std::endl;

    if (!config.contains("simulation") || !config["simulation"].contains("roads"))
    {
        std::cerr << "Configuration error: missing 'simulation' or 'roads' key." << std::endl;
        return;
    }

    const auto& roadsConfig = config["simulation"]["roads"];
    for (const auto& roadConfig : roadsConfig)
    {
        if (roadConfig.contains("roadID"))
        {
            std::cout << std::setw(20) << "Road ID" << roadConfig["roadID"] << std::endl;
        }
        if (roadConfig.contains("roadSize"))
        {
            std::cout << std::setw(20) << "Road Size" << roadConfig["roadSize"] << std::endl;
        }
        if (roadConfig.contains("maxSpeed"))
        {
            std::cout << std::setw(20) << "Max Speed" << roadConfig["maxSpeed"] << std::endl;
        }
        if (roadConfig.contains("brakeProbability"))
        {
            std::cout << std::setw(20) << "Brake Probability" << roadConfig["brakeProbability"] << std::endl;
        }
        if (roadConfig.contains("changingRoadProbability"))
        {
            std::cout << std::setw(20) << "Changing Road Probability" << roadConfig["changingRoadProbability"] << std::endl;
        }
    }
}


void Simulation::run()
{
    std::cout << "Running simulation..." << std::endl;
    for (auto& road : roads)
    {
        road.simulateStep();
        //printRoadStates();
    }
}

void Simulation::printRoadStates() const
{
    std::cout << "Road States at Current Simulation Step:\n";
    for (const auto& road : roads)
    {
        std::cout << "Road " << road.roadID << ":\n";
        std::stringstream tlLine;
        std::stringstream carLine;

        for (const auto& section : road.sections)
        {
            if (section)
            {
                if (section->trafficLight)
                {
                    tlLine << (section->trafficLight->state ? "[O]" : "[C]");
                }
                else
                {
                    tlLine << "   ";
                }

                if (section->currentCar)
                {
                    carLine << " 1 ";
                }
                else
                {
                    carLine << " _ ";
                }
            }
            else
            {
                tlLine << "   ";
                carLine << " _ ";
            }
        }

        std::cout << std::setw(6) << tlLine.str() << "\n";
        std::cout << std::setw(6) << carLine.str() << "\n\n";
    }
}

