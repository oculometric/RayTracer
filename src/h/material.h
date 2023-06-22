#ifndef MATERIAL_H
#define MATERIAL_H

#include "maths.h"

struct RTMaterial
{
    float roughness = 0;
    RTColour colour = RTColour(1,0,1);
};

struct RTSky
{
    RTColour upper;
    RTColour horizon;
    RTColour lower;

    RTColour sample (RTVector);
};

#endif