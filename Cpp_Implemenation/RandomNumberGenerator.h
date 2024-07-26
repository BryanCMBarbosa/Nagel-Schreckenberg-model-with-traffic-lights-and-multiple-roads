#ifndef RANDOM_NUMBER_GENERATOR_H
#define RANDOM_NUMBER_GENERATOR_H

#include <random>

class Simulation;  // Forward declaration

class RandomNumberGenerator
{
public:
    int getRandomInt(int min, int max);
    double getRandomDouble();

protected:
    std::mt19937 generator;  // Mersenne Twister RNG

    // Protected constructor to allow instantiation by friend classes
    RandomNumberGenerator();

    // Friend class declaration
    friend class Simulation;

    // Delete copy constructor and assignment operator to prevent copying
    RandomNumberGenerator(const RandomNumberGenerator&) = delete;
    RandomNumberGenerator& operator=(const RandomNumberGenerator&) = delete;
};

#endif // RANDOM_NUMBER_GENERATOR_H
