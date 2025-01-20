#include "Simulation.h"
#include <iostream>
#include <string>
#include <stdexcept>

int main(int argc, char* argv[])
{
    if (argc < 4)
    {
        std::cerr << "Usage: " << argv[0] 
                  << " <configFilePath> <resultsPath> <executionType>"
                  << std::endl;
        return 1;
    }

    std::string configFilePath = argv[1];
    std::string resultsPath = argv[2];

    short executionType = 0;
    try
    {
        executionType = static_cast<short>(std::stoi(argv[3]));
    }
    catch(const std::exception& e)
    {
        std::cerr << "Invalid executionType argument. "
                  << "Please provide a valid integer for executionType.\n"
                  << "Error: " << e.what() << std::endl;
        return 1;
    }

    try
    {
        Simulation sim(configFilePath, resultsPath, executionType);
        sim.setup();
        sim.run();
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error during simulation: " << e.what() << std::endl;
        return 1;
    }

    std::cout << "Simulation completed successfully." << std::endl;
    return 0;
}
