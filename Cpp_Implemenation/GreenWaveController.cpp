#include "GreenWaveController.h"

GreenWaveController::GreenWaveController(unsigned int cycleTime, double vMax, double p, int columns)
    : TrafficLightController(cycleTime), vMax(vMax), p(p)
{
    if (vMax <= 0 || p < 0 || p > 1)
        throw std::invalid_argument("Invalid v_max or p values.");
    if (columns == 0)
        throw std::invalid_argument("numberOfColumns must be greater than 0.");
    else
        numberOfColumns = columns;
    
    vFree = vMax - p; //Calculate free-flow speed dynamically
}

void GreenWaveController::initialize()
{
    for (size_t i = 0; i < trafficLightGroups.size(); ++i)
    {
        auto& group = trafficLightGroups[i];
        group->initialize();
    }
}

void GreenWaveController::update(unsigned long long currentTime)
{
    for (size_t groupIndex = 0; groupIndex < trafficLightGroups.size(); ++groupIndex)
    {
        size_t rowIndex = groupIndex / numberOfColumns;
        size_t colIndex = groupIndex % numberOfColumns;

        auto& group = trafficLightGroups[groupIndex];

        for (size_t j = 0; j < group->trafficLights.size(); ++j)
        {
            updateParametersBasedOnRoad(group->trafficLights[j]->distanceToPreviousTrafficLight, group->trafficLights[j]->getRoadSpeed(), group->trafficLights[j]->getBrakeProb());
            
            //Calculate offset for the current light based on grid position
            unsigned int offset = calculateOffset(rowIndex, colIndex, group->trafficLights[j]->distanceToPreviousTrafficLight, vFree);
            bool isGreen = ((currentTime + offset) / (cycleTime / 2)) % 2 == 0;
            group->trafficLights[j]->state = isGreen;
        }
    }
}

void GreenWaveController::updateParametersBasedOnRoad(double distanceBetweenIntersections, int roadVMax, double roadP)
{
    if (distanceBetweenIntersections <= 0)
        throw std::invalid_argument("Invalid parameter for cycle time calculation.");

    if (roadVMax > 0)
        vMax = roadVMax;
    
    if (roadP <= 1.0)
        p = roadP;

    vFree = vMax - p;
    double TFree = distanceBetweenIntersections / vFree;

    cycleTime = static_cast<unsigned int>(std::round(TFree * 2));

    if (cycleTime < 1)
        throw std::runtime_error("Calculated cycle time is too short to be realistic.");
}

unsigned int GreenWaveController::calculateOffset(size_t rowIndex, size_t colIndex, double distanceBetweenIntersections, double v_free)
{
    if (distanceBetweenIntersections <= 0 || v_free <= 0)
        throw std::invalid_argument("Invalid parameters for offset calculation.");

    //Correct calculation of T_delay
    double T_delay = distanceBetweenIntersections / v_free;

    //Offset calculation for the current intersection
    return static_cast<unsigned int>(std::round((rowIndex + colIndex) * T_delay)) % cycleTime;
}

GreenWaveController::~GreenWaveController() {}