#include "Simulation.h"

Simulation::Simulation(const std::string& configFilePath, std::string resultsPath = "./", short executionType = 0) : resultsPath(resultsPath), executionType(executionType)
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

    if (config["simulation"].contains("queueSize"))
        queueSize = config["simulation"]["queueSize"];
    else
        queueSize = 10;

    if (config["simulation"].contains("vMax"))
        vMax = config["simulation"]["vMax"];
    else
        vMax = 3;

    if (config["simulation"].contains("brakeProbability"))
        brakeProbability = config["simulation"]["brakeProbability"];
    else
        brakeProbability = 0.1;

    const auto& roadsConfig = config["simulation"]["roads"];

    std::vector<double> alphas;

    for (const auto& roadConfig : roadsConfig)
    {
        int roadID = roadConfig.value("roadID", 0);
        int roadSize = roadConfig.value("roadSize", 50);

        int numCars = 0;
        double density = 0.0;
        if (roadConfig.contains("density"))
            density = roadConfig["density"];
        if (density <= 0.0 && roadConfig.contains("numCars"))
            numCars = roadConfig["numCars"];
        
        int maxSpeed = vMax;
        if (roadConfig.contains("maxSpeed"))
            int maxSpeed = roadConfig.value("maxSpeed", 3);

        double brakeProb = brakeProbability;
        if (roadConfig.contains("brakeProbability"))
            brakeProb = roadConfig.value("brakeProbability", 0.1);
        
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

        if (numCars == 0)
        {
            auto road = std::make_shared<Road>(roadID, roadSize, isPeriodic, beta, vMax, brakeProbability, density, rng, queueSize);
            roads.emplace_back(road);
            road->setupSections();
            road->addCarsBasedOnDensity(density);
        }
        else
        {
            auto road = std::make_shared<Road>(roadID, roadSize, isPeriodic, beta, vMax, brakeProbability, numCars, rng, queueSize);
            roads.emplace_back(road);
            road->setupSections();
            road->addCars(numCars);
        }
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
            for (auto& road : roads)
                std::sort(road->sharedSectionsPositions.begin(), road->sharedSectionsPositions.end());
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

                    auto trafficLight = std::make_shared<TrafficLight>(externalControl, timeOpen, timeClosed, roads[roadID], position);

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
    {
        road->setupTimeHeadwayAndFlowPoints(queueSize);

        std::sort(road->trafficLightPositions.begin(), road->trafficLightPositions.end());
        for (auto& TLPosition : road->trafficLightPositions)
            road->sections[TLPosition]->trafficLight->calculateDistanceToPreviousTrafficLight();
    }

    if (config["simulation"].contains("controllerType") && !trafficLightGroups.empty())
    {
        std::string controllerType = config["simulation"]["controllerType"].get<std::string>();
        if (controllerType == "synchronized")
            trafficLightController = std::make_shared<SyncController>();
        else if (controllerType == "green_wave")
        {
            double vMax = config["simulation"].value("vMax", 3.0);
            double brakeProbability = config["simulation"].value("brakeProbability", 0.2);
            size_t numberOfColumns = config["simulation"].value("numberOfColumns", 4);
            trafficLightController = std::make_shared<GreenWaveController>(60, vMax, brakeProbability, numberOfColumns);
        }
        else if (controllerType == "random_offset")
            trafficLightController = std::make_shared<RandomOffsetController>(rng);
        else
            throw std::invalid_argument("Unknown controller type in configuration.");
            
        for(auto& TLGroup : trafficLightGroups)
            trafficLightController->addTrafficLightGroup(TLGroup);
    }

    if (!trafficLightGroups.empty())
        trafficLightController->initialize();

    currentDay = 0;
    currentHour = 0;

    //printSimulationSettings();
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
    switch (executionType)
    {
    case 0: //Basic execution; no printing; no real-time plotting.
        execute();
        break;
    
    case 1:
        break;

    default:
        break;
    }
}


void Simulation::execute()
{
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
    auto now_time_t = std::chrono::system_clock::to_time_t(now);
    auto now_ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;

    std::ostringstream simInfoStream;
    simInfoStream << std::put_time(std::localtime(&now_time_t), "%Y-%m-%d_%H-%M-%S");
    simInfoStream << "." << std::setfill('0') << std::setw(3) << now_ms.count();
    simInfoStream << "_eps_" << episodes
                  << "_roads_" << numberRoads;

    std::string filename = "sim_results_" + simInfoStream.str() + ".json";
    for (unsigned long long episode = 0; episode < episodes; episode++)
    {
        currentMinute = (episode / 60) % 60;
        currentHour = (episode / 3600) % 24; //Calculate current hour based on elapsed time
        currentDay = (episode / 86400) % 7;  //Calculate current day of the week (0=Sunday, 6=Saturday)
        trafficGen.update(episode, currentDay);

        if (trafficLightController)
            trafficLightController->update(episode);

        for (int roadIndex = 0; roadIndex < numberRoads; roadIndex++)
            roads[roadIndex]->simulateStep(episode);

        collectMetrics(episode);
    }

    serializeResults(filename);
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

void Simulation::createHeader()
{
    nlohmann::json headerData;

    headerData["simulationConfig"]["episodes"] = episodes;
    headerData["simulationConfig"]["queueSize"] = queueSize;
    headerData["simulationConfig"]["vMax"] = vMax;
    headerData["simulationConfig"]["brakeProbability"] = brakeProbability;

    for (const auto& road : roads)
    {
        nlohmann::json roadInfo;
        roadInfo["roadID"] = road->roadID;
        roadInfo["roadSize"] = road->roadSize;
        roadInfo["isPeriodic"] = road->isPeriodic;
        roadInfo["alpha"] = road->alpha;
        roadInfo["beta"] = road->beta;
        roadInfo["maxSpeed"] = road->maxSpeed;
        roadInfo["brakeProb"] = road->brakeProb;
        roadInfo["initialNumCars"] = road->initialNumCars;
        roadInfo["initialDensity"] = road->initialDensity;

        nlohmann::json tlArray;
        for (auto& tl : road->trafficLights)
        {
            nlohmann::json tlInfo;
            tlInfo["timeOpen"]     = tl->timeOpen;
            tlInfo["timeClosed"]   = tl->timeClosed;
            tlInfo["roadPosition"] = tl->roadPosition;
            tlInfo["externalControl"] = tl->externalControl;
            tlArray.push_back(tlInfo);
        }
        roadInfo["trafficLights"] = tlArray;

        headerData["roads"].push_back(roadInfo);
    }

    nlohmann::json tlGroupsArray = nlohmann::json::array();
    for (const auto& group : trafficLightGroups)
    {
        nlohmann::json groupData;
        groupData["degreeCentrality"]      = group->degreeCentrality;
        groupData["betweennessCentrality"] = group->betweennessCentrality;
        groupData["closenessCentrality"]   = group->closenessCentrality;
 
        nlohmann::json groupTLs = nlohmann::json::array();
        for (auto& tl : group->trafficLights)
        {
            nlohmann::json tlInGroup;
            tlInGroup["roadPosition"] = tl->roadPosition;
            groupTLs.push_back(tlInGroup);
        }
        groupData["trafficLights"] = groupTLs;

        tlGroupsArray.push_back(groupData);
    }
    headerData["trafficLightGroups"] = tlGroupsArray;

    simulationResults["header"] = headerData;
}


void Simulation::collectMetrics(unsigned long long episode)
{
    nlohmann::json episodeData;
    episodeData["episode"]       = episode;
    episodeData["currentDay"]    = currentDay;
    episodeData["currentHour"]   = currentHour;
    episodeData["currentMinute"] = currentMinute;

    for (const auto& road : roads)
    {
        nlohmann::json roadData;
        roadData["roadID"] = road->roadID;
        roadData["generalDensity"] = road->generalDensity;
        roadData["averageDistanceHeadway"] = road->averageDistanceHeadway;
        roadData["averageSpeed"] = road->averageSpeed;
        roadData["alpha"] = road->alpha;
        roadData["beta"] = road->beta;
        roadData["numCars"] = road->carsPositions.size();
        //roadData["roadRepresentation"] = road->getRoadRepresentation();

        nlohmann::json timeHeadwaysData = nlohmann::json::array();
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

        nlohmann::json flowData = nlohmann::json::array();
        const auto& flow = road->flowAtPoints;
        for (const auto& [point, value] : flow)
        {
            nlohmann::json pointData;
            pointData["pointIndex"] = point;
            pointData["flow"] = value;
            flowData.push_back(pointData);
        }
        roadData["flow"] = flowData;

        std::vector<int> resTimesVec(road->residenceTimes.begin(), road->residenceTimes.end());
        roadData["residenceTimes"] = resTimesVec;
        std::vector<int> travelTimesVec(road->travelTimes.begin(), road->travelTimes.end());
        roadData["travelTimes"] = travelTimesVec;
        std::vector<double> avgTravelVec(road->averageTravelTimes.begin(), road->averageTravelTimes.end());
        roadData["averageTravelTimes"] = avgTravelVec;

        roadData["newCarInserted"] = road->newCarInserted;

        nlohmann::json trafficLightsArray = nlohmann::json::array();
        for (auto& tl : road->trafficLights)
        {
            nlohmann::json tlData;
            tlData["isGreen"] = tl->isGreen();
            tlData["timer"]   = tl->timer;
            trafficLightsArray.push_back(tlData);
        }
        roadData["trafficLights"] = trafficLightsArray;

        episodeData["roads"].push_back(roadData);
    }

    nlohmann::json tlGroupsData = nlohmann::json::array();
    for (const auto& group : trafficLightGroups)
    {
        nlohmann::json groupData;
        tlGroupsData.push_back(groupData);
    }
    episodeData["trafficLightGroups"] = tlGroupsData;

    simulationResults["episodes"].push_back(episodeData);
}

void Simulation::serializeResults(const std::string& filename) const
{
    std::string fullPath = resultsPath + "/" + filename;
    std::string modifiedPath = fullPath;
    int counter = 1;

    while (std::filesystem::exists(modifiedPath))
    {
        size_t dotPos = fullPath.find_last_of('.');
        if (dotPos == std::string::npos)
            modifiedPath = fullPath + "_" + std::to_string(++counter);
        else
            modifiedPath = fullPath.substr(0, dotPos) + "_" + std::to_string(++counter) + fullPath.substr(dotPos);
    }

    std::ofstream file(modifiedPath);
    if (file.is_open())
    {
        file << std::setw(4) << simulationResults << std::endl;
        file.close();
        std::cout << "Results serialized to " << modifiedPath << std::endl;
    }
    else
        std::cerr << "Unable to open file for serialization: " << modifiedPath << std::endl;
}
