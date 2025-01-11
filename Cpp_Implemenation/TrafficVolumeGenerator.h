#ifndef TRAFFIC_VOLUME_GENERATOR_H
#define TRAFFIC_VOLUME_GENERATOR_H

#include <vector>
#include <memory>
#include <cmath>
#include "Road.h"
#include "RandomNumberGenerator.h"
#include "Dictionary.h"

class TrafficVolumeGenerator
{
public:
    TrafficVolumeGenerator(const std::vector<std::shared_ptr<Road>>& roads, const std::vector<int>& roadsWithAlpha, Dictionary<int, double>& alphaWeights, RandomNumberGenerator& rng, double stdDev, int updateIntervalSeconds);
    void update(unsigned long long timeStep, int currentDay);
    void rebalanceAlpha(unsigned long long timeStep, int currentDay);
    double weekdayPattern(unsigned long long timeStep, double peak1Time, double peak2Time);
    double weekendPattern(unsigned long long timeStep);

private:
    std::vector<std::shared_ptr<Road>> roads;
    std::vector<int> roadsWithAlpha;
    Dictionary<int, double> alphaWeights;
    int updateInterval; //Time interval for updates in time steps
    RandomNumberGenerator& rng;
    double randomnessStdDev;
};

#endif