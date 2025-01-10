#ifndef RANDOM_NUMBER_GENERATOR_H
#define RANDOM_NUMBER_GENERATOR_H

#include <random>

class Simulation;

class RandomNumberGenerator
{
public:
    int getRandomInt(int min, int max);
    double getRandomDouble();
    double getRandomGaussian(double mean, double stddev);
    std::mt19937& getGenerator();

protected:
    std::mt19937 generator;

    RandomNumberGenerator();

    friend class Simulation;

    RandomNumberGenerator(const RandomNumberGenerator&) = delete;
    RandomNumberGenerator& operator=(const RandomNumberGenerator&) = delete;
};

#endif