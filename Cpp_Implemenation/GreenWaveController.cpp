#include "GreenWaveController.h"

GreenWaveController::GreenWaveController(unsigned int cycleTime, double vMax, double p, size_t numberOfColumns)
    : TrafficLightController(cycleTime), v_max(vMax), p(p), numberOfColumns(numberOfColumns)
{
    if (v_max <= 0 || p < 0 || p > 1)
        throw std::invalid_argument("Invalid v_max or p values.");
    if (numberOfColumns == 0)
        throw std::invalid_argument("numberOfColumns must be greater than 0.");

    v_free = v_max - p; //Calculate free-flow speed dynamically
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
            //Calculate offset for the current light based on grid position
            unsigned int offset = calculateOffset(rowIndex, colIndex, distanceBetweenIntersections, v_free);
            bool isGreen = ((currentTime + offset) / (cycleTime / 2)) % 2 == 0;
            group->trafficLights[j]->state = isGreen;
        }
    }
}

void GreenWaveController::calculateCycleTime(double distanceBetweenIntersections)
{
    if (distanceBetweenIntersections <= 0)
        throw std::invalid_argument("Invalid parameter for cycle time calculation.");

    double v_free = v_max - p;
    double T_free = distanceBetweenIntersections / v_free;

    cycleTime = static_cast<unsigned int>(std::round(T_free * 2));

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