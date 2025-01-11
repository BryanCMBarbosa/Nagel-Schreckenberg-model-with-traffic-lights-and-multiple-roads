#include "TrafficVolumeGenerator.h"

TrafficVolumeGenerator::TrafficVolumeGenerator(const std::vector<std::shared_ptr<Road>>& roads, const std::vector<int>& roadsWithAlpha, Dictionary<int, double>& alphaWeights, RandomNumberGenerator& rng, double stdDev = 0.025, int updateIntervalSeconds = 300)
        : roads(roads),
          roadsWithAlpha(roadsWithAlpha),
          alphaWeights(alphaWeights),
          rng(rng),
          updateInterval(updateIntervalSeconds),
          randomnessStdDev(stdDev)
{
}

void TrafficVolumeGenerator::update(unsigned long long timeStep, int currentDay)
{
    if (timeStep % updateInterval == 0)
        rebalanceAlpha(timeStep, currentDay);
}

double TrafficVolumeGenerator::weekdayPattern(unsigned long long timeStep, double peak1Time = 25200, double peak2Time = 54000)
{
    double peak1 = 0.2 * exp(-pow(timeStep - peak1Time, 2) / (2 * pow(5000, 2)));
    double peak2 = 0.2 * exp(-pow(timeStep - peak2Time, 2) / (2 * pow(8000, 2)));
    double valley = -0.15 * exp(-pow(timeStep - 10800, 2) / (2 * pow(4000, 2)));
    double baseline = 0.16;
    double betweenPeaks = 0.09 * exp(-pow(timeStep - 39600, 2) / (2 * pow(5000, 2)));

    return peak1 + peak2 + valley + baseline + betweenPeaks;
    
    //Combination of all components
    return peak1 + peak2 + valley + baseline + betweenPeaks;
}

double TrafficVolumeGenerator::weekendPattern(unsigned long long timeStep)
{
    double peak = 0.13 * exp(-pow(timeStep - 53100, 2) / (2 * pow(25000, 2))); //Peak at 53100
    double valley = -0.14 * exp(-pow(timeStep - 10800, 2) / (2 * pow(8000, 2))); //Valley at 10800
    double baseline = 0.15; //Baseline offset

    //Combination of components
    return peak + valley + baseline;
}

void TrafficVolumeGenerator::rebalanceAlpha(unsigned long long timeStep, int currentDay)
{
    double meanProbOnDayTime = 0.0;
    if (currentDay == 0 || currentDay == 6) //(0=Sunday, 6=Saturday)
        meanProbOnDayTime = weekendPattern(timeStep);
    else
        meanProbOnDayTime = weekdayPattern(timeStep);
    
    double alphaProb = std::clamp(rng.getRandomGaussian(meanProbOnDayTime, randomnessStdDev), 0.005, 1.0);

    for (auto& index : roadsWithAlpha)
        roads[index]->alpha = alphaWeights.get(index) * alphaProb;
}