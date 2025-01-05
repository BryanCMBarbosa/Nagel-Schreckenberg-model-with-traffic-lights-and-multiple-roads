#include "Simulation.h"

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

    if (config["simulation"].contains("episodes"))
        episodes = config["simulation"]["episodes"];
    else
        undefinedDuration = true;

    if (config["simulation"].contains("flowQueueSize"))
        flowQueueSize = config["simulation"]["flowQueueSize"];
    else
        flowQueueSize = 10;

    const auto& roadsConfig = config["simulation"]["roads"];

    for (const auto& roadConfig : roadsConfig)
    {
        int roadID = roadConfig.value("roadID", 0);
        int roadSize = roadConfig.value("roadSize", 50);

        int numCars = 0;
        if (roadConfig.contains("numCars"))
        {
            numCars = roadConfig["numCars"];
        }

        int maxSpeed = roadConfig.value("maxSpeed", 5);
        double brakeProb = roadConfig.value("brakeProbability", 0.1);
        double changeProb = roadConfig.value("changingRoadProbability", 0.15);

        auto road = std::make_shared<Road>(roadID, roadSize, maxSpeed, brakeProb, changeProb, numCars, rng, flowQueueSize);
        roads.emplace_back(road);
        road->setupSections();
        road->addCars(numCars);
        road->calculateGeneralDensity();
    }

    if (config["simulation"].contains("sharedSections"))
    {
        const auto& sharedSections = config["simulation"]["sharedSections"];
        for (const auto& sharedSection : sharedSections)
        {
            int roadID = sharedSection["roadID"];
            int index = sharedSection["index"];
            std::vector<std::pair<int, std::shared_ptr<Road>>> connectedRoads;

            if (roadID >= 0 && roadID < roads.size() && index >= 0 && index < roads[roadID]->sections.size())
            {
                connectedRoads.push_back(std::make_pair(index, roads[roadID]));
                const auto& otherConnectedRoads = sharedSection["sharedWith"];

                for (const auto& otherConnectedRoad : otherConnectedRoads)
                {
                    int otherConnRoadID = otherConnectedRoad["roadID"];
                    int otherConnRoadSection = otherConnectedRoad["index"];

                    if (otherConnRoadID >= 0 && otherConnRoadID < roads.size() &&
                        otherConnRoadSection >= 0 && otherConnRoadSection < roads[otherConnRoadID]->sections.size())
                    {
                        connectedRoads.push_back(std::make_pair(otherConnRoadSection, roads[otherConnRoadID]));
                    }
                    else
                    {
                        std::cerr << "Invalid other roadID or section index: roadID="
                                  << otherConnRoadID << ", section=" << otherConnRoadSection << std::endl;
                    }
                }

                if (!connectedRoads.empty())
                {
                    for (int i = 0; i < connectedRoads.size(); i++)
                    {
                        connectedRoads[i].second->sections[connectedRoads[i].first]->connect(connectedRoads);
                    }
                }
            }
            else
            {
                std::cerr << "Invalid roadID or section index in sharedSection: roadID="
                          << roadID << ", section=" << index << std::endl;
            }
        }
    }
    if (config["simulation"].contains("trafficLightGroups"))
    {
        const auto& trafficLightGroupsConfig = config["simulation"]["trafficLightGroups"];
        for (const auto& groupConfig : trafficLightGroupsConfig)
        {
            int groupID = groupConfig.value("groupID", -1);
            int transitionTime = groupConfig.value("transitionTime", 0);

            if (groupID >= 0)
            {
                if (groupID >= trafficLightGroups.size())
                    trafficLightGroups.resize(groupID + 1);

                std::shared_ptr<TrafficLightGroup> group;
                if (!trafficLightGroups[groupID])
                {
                    group = std::make_shared<TrafficLightGroup>();
                    trafficLightGroups[groupID] = group;
                }
                else
                {
                    group = trafficLightGroups[groupID];
                }

                group->setTransitionTime(transitionTime);
            }
        }
    }
    if (config["simulation"].contains("trafficLights"))
    {
        const auto& trafficLightsConfig = config["simulation"]["trafficLights"];
        for (const auto& trafficLightConfig : trafficLightsConfig)
        {
            int roadID = trafficLightConfig["roadID"];
            if (roadID >= 0 && roadID < roads.size())
            {
                auto road = roads[roadID];
                int position = trafficLightConfig["position"];

                if (position >= 0 && position < road->sections.size())
                {
                    bool externalControl = trafficLightConfig["externalControl"];
                    int timeOpen = trafficLightConfig.value("timeOpen", 10);
                    int timeClosed = trafficLightConfig.value("timeClosed", 10);
                    bool paired = trafficLightConfig.value("paired", false);

                    auto trafficLight = std::make_shared<TrafficLight>(externalControl, timeOpen, timeClosed);

                    if (paired)
                    {
                        std::shared_ptr<TrafficLightGroup> group;

                        if (trafficLightConfig.contains("groupID"))
                        {
                            int groupID = trafficLightConfig["groupID"];
                            if (groupID >= 0 && groupID < trafficLightGroups.size())
                            {
                                group = trafficLightGroups[groupID];
                            }
                            else
                            {
                                group = std::make_shared<TrafficLightGroup>();
                                trafficLightGroups.push_back(group);
                            }
                        }
                        else
                        {
                            group = std::make_shared<TrafficLightGroup>();
                            trafficLightGroups.push_back(group);
                        }

                        group->addTrafficLight(trafficLight);
                    }

                    road->sections[position]->trafficLight = trafficLight;
                    road->trafficLights.push_back(trafficLight);
                }
                else
                {
                    std::cerr << "Invalid position: " << position << " on roadID: " << roadID << std::endl;
                }
            }
            else
            {
                std::cerr << "Invalid roadID: " << roadID << std::endl;
            }
        }
    }

    for (auto& group : trafficLightGroups)
    {
        group->initialize();
    }

    printSimulationSettings();
}


int Simulation::countTotalCars() const
{
    int totalCars = 0;
    for (const auto& road : roads)
    {
        totalCars += road->carsPositions.size();
    }
    return totalCars;
}

void Simulation::printSimulationSettings() const
{
    std::cout << "Simulation Settings:\n";
    std::cout << std::left << std::setw(20) << "Parameter" << std::setw(15) << "Value\n";
    std::cout << std::string(35, '-') << "\n";

    for (const auto& road : roads)
    {
        std::cout << std::setw(20) << "Road ID" << road->roadID << "\n";
        std::cout << std::setw(20) << "Road Size" << road->sections.size() << "\n";
        std::cout << std::setw(20) << "Max Speed" << road->maxSpeed << "\n";
        std::cout << std::setw(20) << "Brake Probability" << road->brakeProb << "\n";
        std::cout << std::setw(20) << "Changing Road Prob" << road->changingRoadProb << "\n";
        std::cout << std::setw(20) << "Connected Roads";
        for (auto connectedRoad : road->connectedRoads)
        {
            std::cout << connectedRoad->roadID << " ";
        }
        std::cout << "\n";
    }
}

void Simulation::run()
{
    std::cout << "Running simulation...\n";
    int numberRoads = roads.size();
    if (undefinedDuration)
    {
    }
    else
    {
        printRoadStates();
        for (unsigned long long episode = 0; episode < episodes; episode++)
        {
            for (auto& group : trafficLightGroups)
            {
                group->update();
            }

            for (int roadIndex = 0; roadIndex < numberRoads; roadIndex++)
            {
                roads[roadIndex]->simulateStep();
            }
            printRoadStates();
            std::this_thread::sleep_for(std::chrono::milliseconds(150));
        }
    }
}

void Simulation::clearScreen() const
{
    #ifdef _WIN32
        system("CLS");
    #else
        system("clear");
    #endif
}

void Simulation::printRoadStates() const
{
    clearScreen();  
    int numberOfRoads = roads.size();
    for (int roadIndex = 0; roadIndex < numberOfRoads; roadIndex++)
    {
        std::stringstream tlLine;
        std::stringstream carLine;

        int roadSize = roads[roadIndex]->sections.size();
        for (int sectionIndex = 0; sectionIndex < roadSize; sectionIndex++)
        {
            if (roads[roadIndex]->sections[sectionIndex]->trafficLight)
            {
                tlLine << (roads[roadIndex]->sections[sectionIndex]->trafficLight->state ? "[O]" : "[C]");
            }
            else
            {
                tlLine << "   ";
            }

            if (roads[roadIndex]->sections[sectionIndex]->currentCar)
            {
                carLine << " " << roads[roadIndex]->sections[sectionIndex]->currentCar->originalRoadID << " ";
            }
            else
            {
                carLine << " _ ";
            }
        }

        std::cout << std::setw(6) << tlLine.str() << "\n";
        std::cout << std::setw(6) << carLine.str() << "\n\n";
    }
}

