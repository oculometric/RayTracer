#ifndef GEOMETRY_H
#define GEOMETRY_H

struct RTMaterial;
struct RTSky;

#include "math.h"
#include "material.h"
#include <vector>
#include <string>

#define BVH_DIVISION_TOPLEVEL_RADIUS 1
#define BVH_DIVISION_MAX 3

struct RTTri
{
    // Triangle vertices are indexed clockwise, when viewed with the normal facing the camera
    RTPoint v1;
    RTPoint v2;
    RTPoint v3;

    RTMaterial * material;

    RTVector vec_a;
    RTVector vec_b;

    RTVector norm;

    RTTri();
    RTTri(const RTTri &);
    RTTri(RTPoint, RTPoint, RTPoint, RTMaterial *);

    void calc_vecs();
    void calc_norm();

    RTTri & operator =(const RTTri &);

    std::string describe();
};

struct RTBVHNode
{
    RTBVHNode * children[8];

    std::vector<RTTri *> triangles;

    RTVector center;
};

struct RTGeometryBuffer
{
    std::vector<RTTri *> triangles;

    std::vector<RTBoundBox *> triangle_bounding_boxes;

    std::vector<RTBVHNode *> bvh_toplevel;

    RTBVHNode * add_toplevel_block(RTVector);
    void insert_triangle_child(RTTri *, int, RTBVHNode *);

    std::vector<RTTri *> triangles_for_ray(RTRay); // fetch triangles which touch relevant BVH segments. we'll implement that later
    void insert_triangle(RTTri *); // insert a triangle into the BVH structure

    bool load_obj(std::string, RTVector, RTMaterial *);
};

std::vector<std::string> split_str(std::string, char);

#endif