#ifndef TRAFFIC_LIGHT_GROUP_H
#define TRAFFIC_LIGHT_GROUP_H

#include <vector>
#include <memory>
#include <iostream>
#include "TrafficLight.h"

class TrafficLight;

class TrafficLightGroup : public std::enable_shared_from_this<TrafficLightGroup>
{
public:
    std::vector<std::shared_ptr<TrafficLight>> trafficLights;
    int column;
    int row;
    int gridHeight;
    int gridWidth;
    int degreeCentrality;
    double betweennessCentrality;
    double closenessCentrality;
    std::vector<short> cyclesDuration;
    
    TrafficLightGroup();

    void addTrafficLight(std::shared_ptr<TrafficLight> trafficLight);
    void initialize();
    void update();
    void setTransitionTime(int time);
    void setCoords(int column, int row);
    void setGridShape(int h, int w);
    void calculateCentralities();
    void calculateDegreeCentrality();
    void calculateClosenessCentrality();
    void calculateBetweennessCentrality();

private:
    int currentIndex;
    bool inGreenPhase;
    bool inTransitionPhase; 
    int groupTimer;
    int transitionTime;
    int totalCycleTime;
    void calculateTotalCycleTime();
};

#endif