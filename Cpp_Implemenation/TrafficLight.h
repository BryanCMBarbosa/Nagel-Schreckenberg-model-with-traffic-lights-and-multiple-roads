#ifndef TRAFFICLIGHT_H
#define TRAFFICLIGHT_H

#include <map>
#include "Road.h"

using namespace std;

class TrafficLight
{
    public:
        int position;
        int timeOpen;
        int timeClosed; //Ignored if paired with another Traffic Light
        int nextToggle;
        bool state;  //true for green, false for red
        Road* road;
        TrafficLight* pair;

        TrafficLight(int pos, Road* r, TrafficLight* p);
        void toggleState();
        bool updateLight(int time);
};

#endif