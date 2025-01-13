#include "Simulation.h"

Simulation::Simulation(const std::string& configFilePath) : undefinedDuration(false)
{
    std::ifstream file(configFilePath);
    if (file.is_open())
    {
        file >> config;
        file.close();
    }
    else
        throw std::runtime_error("Unable to open configuration file.");
}

void Simulation::setup()
{
    if (!config.contains("simulation") || !config["simulation"].contains("roads"))
        throw std::runtime_error("Invalid configuration: missing 'simulation' or 'roads' key.");

    if (config["simulation"].contains("episodes"))
        episodes = config["simulation"]["episodes"];
    else
        undefinedDuration = true;

    if (config["simulation"].contains("queueSize"))
        queueSize = config["simulation"]["queueSize"];
    else
        queueSize = 10;

    const auto& roadsConfig = config["simulation"]["roads"];

    std::vector<double> alphas;

    for (const auto& roadConfig : roadsConfig)
    {
        int roadID = roadConfig.value("roadID", 0);
        int roadSize = roadConfig.value("roadSize", 50);

        int numCars = 0;
        if (roadConfig.contains("numCars"))
            numCars = roadConfig["numCars"];

        int maxSpeed = roadConfig.value("maxSpeed", 5);
        double brakeProb = roadConfig.value("brakeProbability", 0.1);
        bool isPeriodic = roadConfig.value("isPeriodic", true);
        double alpha = roadConfig.value("alphaWeight", 0.0);
        if (alpha > 0.0)
        {
            roadsWithAlpha.push_back(roadID);
            alphaWeights.add(roadID, alpha);
            alphas.push_back(alpha);
        }

        double beta = roadConfig.value("beta", 0.0);
        if (beta > 0.0)
            roadsWithBeta.push_back(roadID);

        auto road = std::make_shared<Road>(roadID, roadSize, isPeriodic, beta, maxSpeed, brakeProb, numCars, rng, queueSize);
        roads.emplace_back(road);
        road->setupSections();
        road->addCars(numCars);
    }

    double alphasSum = std::accumulate(alphas.begin(), alphas.end(), 0.0);
    if ((std::abs(alphasSum - 1.0) > 1e-6) && alphasSum != 0.0)
        for (auto& id : roadsWithAlpha)
            alphaWeights.add(id, (alphaWeights.get(id)/alphasSum));

    if (config["simulation"].contains("roads"))
    {
        const auto& roadsConfig = config["simulation"]["roads"];
        for (const auto& roadConfig : roadsConfig)
        {
            int roadID = roadConfig["roadID"];
            const auto& sharedSections = roadConfig["sharedSections"];
            for (const auto& sharedSection : sharedSections)
            {
                if (sharedSection.size() == 5)
                {
                    int otherRoadID = sharedSection[0];
                    int currentSite = sharedSection[1];
                    int otherSite = sharedSection[2];
                    double currentToOtherProb = sharedSection[3];
                    double otherToCurrentProb = sharedSection[4];

                    if ((roadID >= 0 && roadID < roads.size() && currentSite >= 0 && currentSite < roads[roadID]->roadSize) &&
                        (otherRoadID >= 0 && otherRoadID < roads.size() && otherSite >= 0 && otherSite < roads[otherRoadID]->sections.size()))
                    {
                        roads[roadID]->changingRoadProbs.add(currentSite, currentToOtherProb);
                        roads[otherRoadID]->changingRoadProbs.add(otherSite, otherToCurrentProb);
                        roads[roadID]->sections[currentSite]->connect(roads[otherRoadID]->sections[otherSite]);
                        roads[otherRoadID]->sections[otherSite]->connect(roads[roadID]->sections[currentSite]);
                    }
                    else
                        std::cerr << "Invalid roadID or section index in sharedSection." << std::endl;
                }
                else
                    std::cerr << "Not enough parameters on shared section specifications" << std::endl;
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
                    group = trafficLightGroups[groupID];

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
                                group = trafficLightGroups[groupID];
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
                    road->trafficLightPositions.push_back(position);
                }
                else
                    std::cerr << "Invalid position: " << position << " on roadID: " << roadID << std::endl;
            }
            else
                std::cerr << "Invalid roadID: " << roadID << std::endl;
        }
    }
    for (auto& road : roads)
        road->setupTimeHeadwayPoints(queueSize);

    for (auto& group : trafficLightGroups)
        group->initialize();

    currentDay = 0;
    currentHour = 0;

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
        road->isPeriodic ? std::cout << std::setw(20) << "Periodic boundary" <<  "\n" : std::cout << std::setw(20) << "Open boundary" <<  "\n";
        std::cout << std::setw(20) << "Max Speed" << road->maxSpeed << "\n";
        std::cout << std::setw(20) << "Brake Probability" << road->brakeProb << "\n";
        //std::cout << std::setw(20) << "Changing Road Prob" << road->changingRoadProb << "\n";
        std::cout << std::setw(20) << "Alpha" << road->alpha << "\n";
        std::cout << std::setw(20) << "Beta" << road->beta << "\n";
        std::cout << std::string(35, ':') << "\n";
    }
    std::cout << "Press any key to continue...";
    std::cin.get();
}

void Simulation::run()
{
    std::cout << "Running simulation...\n";
    int numberRoads = roads.size();

    TrafficVolumeGenerator trafficGen(
        roads,
        roadsWithAlpha,
        alphaWeights,
        rng,                  
        0.05,                 
        300                   
    );

    simulationResults["episodes"] = nlohmann::json::array();

    auto now = std::chrono::system_clock::now();
    std::time_t now_time = std::chrono::system_clock::to_time_t(now);
    std::ostringstream simInfoStream;
    simInfoStream << std::put_time(std::localtime(&now_time), "%Y-%m-%d_%H-%M-%S");
    simInfoStream << "eps_" << episodes
                  << "_roads_" << numberRoads;

    std::string filename = "sim_results_" + simInfoStream.str() + ".json";

    if (undefinedDuration)
    {
        //Handle undefined duration if applicable
    }
    else
    {
        printRoadStates();
        for (unsigned long long episode = 0; episode < episodes; episode++)
        {
            currentHour = (episode / 3600) % 24; //Calculate current hour based on elapsed time
            currentDay = (episode / 86400) % 7;  //Calculate current day of the week (0=Sunday, 6=Saturday)
            trafficGen.update(episode, currentDay);

            for (auto& group : trafficLightGroups)
                group->update();

            for (int roadIndex = 0; roadIndex < numberRoads; roadIndex++)
                roads[roadIndex]->simulateStep(episode);

            collectMetrics(episode);

            printRoadStates();
            std::this_thread::sleep_for(std::chrono::milliseconds(150));
        }

        serializeResults("simulation_results.json");
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

void Simulation::collectMetrics(unsigned long long episode)
{
    nlohmann::json episodeData;
    episodeData["episode"] = episode;
    episodeData["currentDay"] = currentDay;
    episodeData["currentHour"] = currentHour;

    for (const auto& road : roads)
    {
        nlohmann::json roadData;
        roadData["roadID"] = road->roadID;
        roadData["generalDensity"] = road->generalDensity;
        roadData["cumulativeTimeSpaceAveragedFlow"] = road->cumulativeTimeSpaceAveragedFlow;
        roadData["averageDistanceHeadway"] = road->averageDistanceHeadway;
        roadData["averageTimeHeadway"] = road->averageTimeHeadway;
        roadData["averageSpeed"] = road->averageSpeed;
        roadData["alpha"] = road->alpha;
        roadData["beta"] = road->beta;
        roadData["numCars"] = road->carsPositions.size();
        roadData["roadRepresentation"] = road->getRoadRepresentation();

        nlohmann::json timeHeadwaysData;
        const auto& timeHeadways = road->getLoggedTimeHeadways();
        for (const auto& [point, queue] : timeHeadways)
        {
            nlohmann::json pointData;
            pointData["pointIndex"] = point;

            std::vector<unsigned long long> timeHeadwayValues(queue.begin(), queue.end());
            pointData["timeHeadways"] = timeHeadwayValues;

            timeHeadwaysData.push_back(pointData);
        }
        roadData["timeHeadways"] = timeHeadwaysData;
        episodeData["roads"].push_back(roadData);
    }

    simulationResults["episodes"].push_back(episodeData);
}


void Simulation::serializeResults(const std::string& filename) const
{
    std::ofstream file(filename);
    if (file.is_open())
    {
        file << std::setw(4) << simulationResults << std::endl;
        file.close();
        std::cout << "Results serialized to " << filename << std::endl;
    }
    else
    {
        std::cerr << "Unable to open file for serialization: " << filename << std::endl;
    }
}