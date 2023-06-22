#include "../h/material.h"

#define _USE_MATH_DEFINES
#include <math.h>

RTColour RTSky::sample(RTVector v)
{
    float mixer = (asin(v.z)/M_PI)*2;
    if (mixer > 0) return lerp (this->horizon, this->upper, mixer);
    if (mixer < 0) return lerp (this->horizon, this->lower, -mixer);
    return this->horizon;
}