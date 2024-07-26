#ifndef LANE_H
#define LANE_H

#include <vector>
#include "RoadSection.h"

class Lane
{
public:
    std::vector<RoadSection*> sections;
    int roadID;
    int laneID;

    Lane(int size, int roadID, int laneID);
};

#endif
