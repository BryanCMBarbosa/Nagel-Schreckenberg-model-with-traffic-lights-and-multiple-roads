#include "TrafficVolumeGenerator.h"

TrafficVolumeGenerator::TrafficVolumeGenerator(const std::vector<std::shared_ptr<Road>>& roads, const std::vector<int>& roadsWithAlpha,
                           double weekdayAmplitude, double weekdayMean, double weekdaySigma, bool weekdayBimodal, double weekdayBaseline,
                           double weekendAmplitude, double weekendMean, double weekendSigma, bool weekendBimodal, double weekendBaseline,
                           RandomNumberGenerator& rng,
                           double stdDev = 0.05,
                           int updateIntervalSeconds = 300)
        : roads(roads),
          roadsWithAlpha(roadsWithAlpha),
          totalAlpha(0.0),
          updateInterval(updateIntervalSeconds),
          elapsedTime(0),
          rng(rng),
          randomnessStdDev(stdDev),
          weekdayPattern{weekdayAmplitude, weekdayMean, weekdaySigma, weekdayBimodal, weekdayBaseline},
          weekendPattern{weekendAmplitude, weekendMean, weekendSigma, weekendBimodal, weekendBaseline}
    {
        for (const auto& road : roads)
        {
            totalAlpha += road->alpha;
        }
    }

void TrafficVolumeGenerator::simulateStep()
{
    elapsedTime++;
    if (elapsedTime % updateInterval == 0)
    {
        int currentHour = (elapsedTime / 3600) % 24; //Calculate current hour based on elapsed time
        int currentDay = (elapsedTime / 86400) % 7;  //Calculate current day of the week (0=Sunday, 6=Saturday)
        rebalanceAlpha(currentHour, currentDay);
    }
}

void TrafficVolumeGenerator::rebalanceAlpha(int currentHour, int currentDay)
{
    const TrafficPatternParams* pattern;

    if (currentDay == 0 || currentDay == 6) //Weekend (Sunday=0, Saturday=6)
        pattern = &weekendPattern;
    else
        pattern = &weekdayPattern;

    double timeMultiplier = 0.0;

    if (pattern->isBimodal)
    {
        //Bimodal pattern (e.g. weekday with morning and afternoon peaks)
        double morningComponent = 0.4 * pattern->a * std::exp(-std::pow(currentHour - pattern->mu, 2) / (2 * std::pow(pattern->sigma, 2)));
        double afternoonComponent = 0.4 * pattern->a * std::exp(-std::pow(currentHour - (pattern->mu + 8), 2) / (2 * std::pow(pattern->sigma, 2))); //Offset by 8 hours for the second peak
        double valleyBaseline = 0.01; //Midday valley baseline
        timeMultiplier = morningComponent + afternoonComponent + valleyBaseline;
    }
    else
    {
        double peakComponent = 0.35 * pattern->a * std::exp(-std::pow(currentHour - pattern->mu, 2) / (2 * std::pow(pattern->sigma, 2)));
        double secondaryComponent = 0.2 * 0.35 * pattern->a * std::exp(-std::pow(currentHour - (pattern->mu + 6), 2) / (2 * std::pow(pattern->sigma * 1.5, 2)));
        double smoothingFactor = std::exp(-std::pow(currentHour - (pattern->mu + 6), 2) / (2 * std::pow(pattern->sigma * 2, 2)));
        timeMultiplier = (peakComponent + secondaryComponent) * smoothingFactor + pattern->baseline * (1 - smoothingFactor);
    }

    //Add randomness to the timeMultiplier using Gaussian sampling
    timeMultiplier = rng.getRandomGaussian(timeMultiplier, randomnessStdDev);

    //Ensure timeMultiplier remains within reasonable bounds
    if (timeMultiplier < 0.0)
        timeMultiplier = 0.0;

    //Rebalance alpha for each road based on the current time multiplier
    for (auto& index : roadsWithAlpha)
    {
        double normalizedAlpha = roads[index]->alpha / totalAlpha;
        roads[index]->alpha = normalizedAlpha * timeMultiplier * totalAlpha;
    }
}