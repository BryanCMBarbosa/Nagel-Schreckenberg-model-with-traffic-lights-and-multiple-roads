#include "TrafficLight.h"

TrafficLight::TrafficLight(int pos, Road* r, TrafficLight* p)
    : position(pos), road(r), pair(p), timeOpen(0), timeClosed(0), nextToggle(0), state(false)
    {}