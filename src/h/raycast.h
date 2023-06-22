#ifndef RAYCAST_H
#define RAYCAST_H

#include "maths.h"
#include "geometry.h"

// TODO: lots of things may want to be converted to references, or pointers, instead of values. think about this

struct RTRayCast
{
    RTTri * triangle;
    float distance;
    bool intersected;
    int8_t facing;

    RTRayCast();
};

RTRayCast ray_cast(RTRay, RTTri *);

RTRayCast ray_cast(RTRay, RTGeometryBuffer *);

#endif