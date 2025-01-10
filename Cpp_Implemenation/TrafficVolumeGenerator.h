#ifndef TRAFFIC_VOLUME_GENERATOR_H
#define TRAFFIC_VOLUME_GENERATOR_H

#include <vector>
#include <memory>
#include <cmath>
#include "Road.h"
#include "RandomNumberGenerator.h"

class TrafficVolumeGenerator
{
public:
    TrafficVolumeGenerator(const std::vector<std::shared_ptr<Road>>& roads, const std::vector<int>& roadsWithAlpha, double weekdayAmplitude, double weekdayMean, double weekdaySigma, bool weekdayBimodal, double weekdayBaseline, double weekendAmplitude, double weekendMean, double weekendSigma, bool weekendBimodal, double weekendBaseline, RandomNumberGenerator& rng, double stdDev, int updateIntervalSeconds);
    void simulateStep();
    void rebalanceAlpha(int currentHour, int currentDay);

private:
    std::vector<std::shared_ptr<Road>> roads;
    std::vector<int> roadsWithAlpha;
    double totalAlpha;
    int updateInterval; //Time interval for updates in time steps
    int elapsedTime; //Tracks elapsed time in the simulation
    RandomNumberGenerator& rng;

    struct TrafficPatternParams
    {
        double a; //Amplitude
        double mu; //Mean
        double sigma; //Standard deviation
        bool isBimodal; //Determines if the pattern has a bimodal distribution
        double baseline; //Baseline traffic volume to prevent zero volume
    };

    TrafficPatternParams weekdayPattern;
    TrafficPatternParams weekendPattern;
    double randomnessStdDev;
};

#endif