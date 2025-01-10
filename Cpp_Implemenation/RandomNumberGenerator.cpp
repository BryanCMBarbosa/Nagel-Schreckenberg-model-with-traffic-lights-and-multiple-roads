#include "RandomNumberGenerator.h"

RandomNumberGenerator::RandomNumberGenerator() : generator(std::random_device{}())
{
}

int RandomNumberGenerator::getRandomInt(int min, int max)
{
    std::uniform_int_distribution<> distrib(min, max);
    return distrib(generator);
}

double RandomNumberGenerator::getRandomDouble()
{
    std::uniform_real_distribution<double> distrib(0.0, 1.0);
    return distrib(generator);
}

double RandomNumberGenerator::getRandomGaussian(double mean, double stddev)
{
    std::normal_distribution<double> distrib(mean, stddev);
    return distrib(generator);
}

std::mt19937& RandomNumberGenerator::getGenerator()
{
    return generator;
}
