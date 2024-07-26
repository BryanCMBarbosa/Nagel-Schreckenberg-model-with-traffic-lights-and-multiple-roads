#include "Lane.h"

Lane::Lane(int size, int roadID, int laneID) : sections(size, nullptr), roadID(roadID), laneID(laneID)
{
}
