#include "Simulation.h"
#include <iostream>
#include <string>

int main()
{
    std::string configFilePath = "./simulation_settings.json";

    try
    {
        Simulation sim(configFilePath);
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
