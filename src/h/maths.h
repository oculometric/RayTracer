#ifndef MATHS_H
#define MATHS_H

#include <string>
#include <iostream>
#include <math.h>

inline float sqr (float f) { return f*f; }

struct RTVector
{
    float x;
    float y;
    float z;

    RTVector();
    RTVector(const RTVector &);
    RTVector(float, float, float);
    RTVector(float);

    float mag_sq();
    float mag();

    float norm_self();
    RTVector norm_copy();

    RTVector & operator =(const RTVector &);
    RTVector & operator +=(const RTVector &);
    RTVector & operator -=(const RTVector &);
    RTVector & operator *=(const float);
    RTVector & operator /=(const float);
    RTVector operator -() const;

    std::string describe();
};

typedef RTVector RTPoint;
typedef RTVector RTColour;

inline RTVector operator +(const RTVector & v1, const RTVector & v2) { return RTVector(v1.x+v2.x, v1.y+v2.y, v1.z+v2.z); }
inline RTVector operator -(const RTVector & v1, const RTVector & v2) { return RTVector(v1.x-v2.x, v1.y-v2.y, v1.z-v2.z); }
inline RTVector operator *(const RTVector & v1, const RTVector & v2) { return RTVector(v1.x*v2.x, v1.y*v2.y, v1.z*v2.z); }
inline RTVector operator /(const RTVector & v1, const RTVector & v2) { return RTVector(v1.x/v2.x, v1.y/v2.y, v1.z/v2.z); }
inline RTVector operator *(const RTVector & v, const float f) { return RTVector(v.x*f, v.y*f, v.z*f); }
inline RTVector operator /(const RTVector & v, const float f) { return RTVector(v.x/f, v.y/f, v.z/f); }
inline bool operator ==(const RTVector & v1, const RTVector & v2) { return (v1.x==v2.x && v1.y==v2.y && v1.z==v2.z); }

inline float operator ^(const RTVector & v1, const RTVector & v2) { return ((v1.x*v2.x)+(v1.y*v2.y)+(v1.z*v2.z)); } // Dot product
inline RTVector operator %(const RTVector & v1, const RTVector & v2) { return RTVector(((v1.y*v2.z)-(v1.z*v2.y)), ((v1.z*v2.x)-(v1.x*v2.z)), ((v1.x*v2.y)-(v1.y*v2.x))); } // Cross product

struct RTRay
{
    RTPoint origin;
    RTVector direction;

    RTRay();
    RTRay(const RTRay &);
    RTRay(RTPoint, RTVector);

    std::string describe();
};

struct RTVector2D
{
    float u;
    float v;

    RTVector2D();
    RTVector2D(const RTVector2D &);
    RTVector2D(float, float);

    std::string describe();
};

inline RTVector2D operator +(const RTVector2D & v1, const RTVector2D & v2) { return RTVector2D(v1.u+v2.u, v1.v+v2.v); }
inline RTVector2D operator -(const RTVector2D & v1, const RTVector2D & v2) { return RTVector2D(v1.u-v2.u, v1.v-v2.v); }
inline RTVector2D operator *(const RTVector2D & v1, const RTVector2D & v2) { return RTVector2D(v1.u*v2.u, v1.v*v2.v); }
inline RTVector2D operator /(const RTVector2D & v1, const RTVector2D & v2) { return RTVector2D(v1.u/v2.u, v1.v/v2.v); }
inline RTVector2D operator *(const RTVector2D & v, const float f) { return RTVector2D(v.u*f, v.v*f); }
inline RTVector2D operator /(const RTVector2D & v, const float f) { return RTVector2D(v.u/f, v.v/f); }

inline RTVector lerp(RTVector v1, RTVector v2, float f) { return (v1*(1-f))+(v2*(f)); }
inline RTVector reflect(RTVector v1, RTVector v2) { return (v1-((v2*2)*(v1^v2))).norm_copy(); }   // reflect v1 in v2
inline RTRay get_reflected(RTRay ray, float dist, RTVector norm) { return RTRay((dist*ray.direction)+ray.origin, reflect(ray.direction, norm)); }
inline RTVector disturb(const RTVector & v, const RTVector2D & d)
{
    RTVector r = RTVector(1,2,3);
    RTVector a = v % (v * r);
    RTVector b = v % a;
    return (a * d.u) + (b * d.v); // i think this works
}
inline float clamp0(float f) { return f < 0 ? 0 : f; }

inline void gamma_correct(const RTVector & c, float exp, float gam, uint8_t * o)
{
    RTVector g = c * exp;
    g = RTVector(powf(clamp0(g.x),gam), powf(clamp0(g.y),gam), powf(clamp0(g.z),gam));
    o[0] = (uint8_t)(g.x*UINT8_MAX);
    o[1] = (uint8_t)(g.y*UINT8_MAX);
    o[2] = (uint8_t)(g.z*UINT8_MAX);
}

#endif