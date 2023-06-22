#include "../h/maths.h"

#include <cmath>

RTVector::RTVector()
{
    this->x = 0;
    this->y = 0;
    this->z = 1;
}

RTVector::RTVector(const RTVector & v)
{
    this->x = v.x;
    this->y = v.y;
    this->z = v.z;
}

RTVector::RTVector(float _x, float _y, float _z)
{
    this->x = _x;
    this->y = _y;
    this->z = _z;
}

RTVector::RTVector(float f)
{
    this->x = f;
    this->y = f;
    this->z = f;
}

float RTVector::mag_sq()
{
    return (sqr(this->x) + sqr(this->y) + sqr(this->z));
}

float RTVector::mag()
{
    return std::sqrt(this->mag_sq());
}

float RTVector::norm_self()
{
    float _mag = this->mag();
    if (_mag == 0) return 0;
    this->x /= _mag;
    this->y /= _mag;
    this->z /= _mag;
    return _mag;
}

RTVector RTVector::norm_copy()
{
    RTVector v (*this);
    v.norm_self();
    return v;
}

RTVector & RTVector::operator =(const RTVector & v)
{
    this->x = v.x;
    this->y = v.y;
    this->z = v.z;
    return *this;
}

RTVector & RTVector::operator +=(const RTVector & v)
{
    this->x += v.x;
    this->y += v.y;
    this->z += v.z;
    return *this;
}

RTVector & RTVector::operator -=(const RTVector & v)
{
    this->x -= v.x;
    this->y -= v.y;
    this->z -= v.z;
    return *this;
}

RTVector & RTVector::operator *=(const float f)
{
    this->x *= f;
    this->y *= f;
    this->z *= f;
    return *this;
}

RTVector & RTVector::operator /=(const float f)
{
    this->x /= f;
    this->y /= f;
    this->z /= f;
    return *this;
}

RTVector RTVector::operator -() const
{
    return RTVector(-this->x, -this->y, -this->z);
}

std::string RTVector::describe()
{
    std::string s = "";
    s += "(";
    s += std::to_string(this->x);
    s += ",";
    s += std::to_string(this->y);
    s += ",";
    s += std::to_string(this->z);
    s += ")";
    return s;
}

RTRay::RTRay()
{
    this->origin = RTPoint();
    this->direction = RTVector();
}

RTRay::RTRay(const RTRay & r)
{
    this->origin = RTPoint(r.origin);
    this->direction = RTVector(r.direction);
}

RTRay::RTRay(RTPoint p, RTVector v)
{
    this->origin = p;
    this->direction = v.norm_copy();
}

std::string RTRay::describe()
{
    std::string s = "";
    s += "[";
    s += this->origin.describe();
    s += ":";
    s += this->direction.describe();
    s += "]";
    return s;
}

RTVector2D::RTVector2D()
{
    this->u = 0;
    this->v = 0;
}

RTVector2D::RTVector2D(const RTVector2D & vec)
{
    this->u = vec.u;
    this->v = vec.v;
}

RTVector2D::RTVector2D(float _u, float _v)
{
    this->u = _u;
    this->v = _v;
}

std::string RTVector2D::describe()
{
    std::string s = "";
    s += "(";
    s += std::to_string(this->u);
    s += ",";
    s += std::to_string(this->v);
    s += ")";
    return s;
}
