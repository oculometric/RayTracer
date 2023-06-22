#include "../h/raycast.h"

#include <iostream>
using namespace std;

RTRayCast ray_cast(RTRay ray, RTTri * tri)
{
    //cout << "casting against tri: " << tri->describe() << endl;
    float dot = (ray.direction ^ tri->norm);
    if (dot == 0) return RTRayCast(); // parallel to plane
    float backfacing = dot > 0 ? 1 : -1;

    float dist = ((tri->v1 - ray.origin) ^ tri->norm) / dot; // distance along ray
    if (dist <= 0) return RTRayCast();
    //cout << "triangle is in front of us " << to_string(dist) << endl;

    RTVector v1 = (tri->v1 - ray.origin); // vector from ray origin to each vertex
    RTVector v2 = (tri->v2 - ray.origin);
    RTVector v3 = (tri->v3 - ray.origin);

    // these three triangles along with the main triangle being checked form a triangular based pyramid
    RTVector in1 = (v1 % v2) * backfacing; // form a triangle between v1, v2, and the ray origin, and find a normal vector pointing inwards (towards the centre of the main triangle)
    if ((in1 ^ ray.direction) <= 0) return RTRayCast();
    RTVector in2 = (v2 % v3) * backfacing;
    if ((in2 ^ ray.direction) <= 0) return RTRayCast();
    RTVector in3 = (v3 % v1) * backfacing;
    if ((in3 ^ ray.direction) <= 0) return RTRayCast();

    //cout << "we hit!" << endl;

    // if none of those failed, then the ray exists within a triangular based pyramid, starting from its tip, and thus must touch the triangle

    RTRayCast rc;
    rc.distance = dist;
    rc.intersected = true;
    rc.triangle = tri;
    rc.facing = (int)backfacing;

    return rc;
}

RTRayCast ray_cast(RTRay ray, RTGeometryBuffer * buf)
{
    std::vector<RTTri *> tris = buf->triangles_for_ray(ray);
    //if (tris.size() == 0) return RTRayCast();
    // TODO: implement a distance pre-sorting algorithm?
    RTRayCast closest_hit = RTRayCast();
    for (RTTri * tri : tris)
    {
        RTRayCast rc = ray_cast(ray, tri);
        if (!rc.intersected) continue;
        if (rc.distance < closest_hit.distance || closest_hit.distance == 0)
        {
            closest_hit = rc;
        }
    }
    return closest_hit;
}

RTRayCast::RTRayCast()
{
    this->intersected = false;
    this->distance = 0;
    this->triangle = NULL;
    this->facing = 0.5;
}
