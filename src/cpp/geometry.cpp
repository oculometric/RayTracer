#include "../h/geometry.h"

#include <iostream>
#include <fstream>
using namespace std;

RTTri::RTTri()
{
    this->v1 = RTPoint(0,0,0);
    this->v2 = RTPoint(1,0,0);
    this->v3 = RTPoint(0,1,0);
    this->material = new RTMaterial();

    this->calc_vecs();
    this->calc_norm();
}

RTTri::RTTri(const RTTri & t)
{
    this->v1 = RTPoint(t.v1);
    this->v2 = RTPoint(t.v2);
    this->v3 = RTPoint(t.v3);

    this->vec_a = RTVector(t.vec_a);
    this->vec_b = RTVector(t.vec_b);

    this->norm = RTVector(t.norm);
}

RTTri::RTTri(RTPoint _v1, RTPoint _v2, RTPoint _v3, RTMaterial * _mat)
{
    this->v1 = _v1;
    this->v2 = _v2;
    this->v3 = _v3;
    this->material = _mat;

    this->calc_vecs();
    this->calc_norm();
}

void RTTri::calc_vecs()
{
    this->vec_a = this->v2 - this->v1;
    this->vec_b = this->v3 - this->v1;
}

void RTTri::calc_norm()
{
    this->norm = (this->vec_a % this->vec_b).norm_copy();
}

RTTri & RTTri::operator =(const RTTri & t)
{
    this->v1 = RTPoint(t.v1);
    this->v2 = RTPoint(t.v2);
    this->v3 = RTPoint(t.v3);
    this->material = t.material;

    this->vec_a = RTVector(t.vec_a);
    this->vec_b = RTVector(t.vec_b);

    this->norm = RTVector(t.norm);
    return *this;
}

std::string RTTri::describe()
{
    std::string s = "";
    s += "[";
    s += this->v1.describe();
    s += ":";
    s += this->v2.describe();
    s += ":";
    s += this->v3.describe();
    s += ":";
    s += this->norm.describe();
    s += "]";

    return s;
}

bool box_intersection(RTVector box_min, RTVector box_max, RTVector box_center, RTRay ray)
{
    // float t = (ray.direction ^ (box_center - ray.origin)) / (ray.direction ^ ray.direction);
    // if (t < -1) return false;
    // RTVector p = (ray.direction * t) + ray.origin;

    // if (p.x > box_max.x) return false;
    // if (p.x < box_min.x) return false;
    
    // if (p.y > box_max.y) return false;
    // if (p.y < box_min.y) return false;
    
    // if (p.z > box_max.z) return false;
    // if (p.z < box_min.z) return false;

    float ray_inv_x = 1.0 / ray.direction.x;
    float t0x = (box_min.x - ray.origin.x) * ray_inv_x;
    float t1x = (box_max.x - ray.origin.x) * ray_inv_x;
    float tminx = min(t0x, t1x); float tmaxx = max(t0x,t1x);
    if (tminx > tmaxx) return false;

    float ray_inv_y = 1.0 / ray.direction.y;
    float t0y = (box_min.y - ray.origin.y) * ray_inv_y;
    float t1y = (box_max.y - ray.origin.y) * ray_inv_y;
    float tminy = min(t0y, t1y); float tmaxy = max(t0y,t1y);
    if (tminy > tmaxy) return false;

    float ray_inv_z = 1.0 / ray.direction.z;
    float t0z = (box_min.z - ray.origin.z) * ray_inv_z;
    float t1z = (box_max.z - ray.origin.z) * ray_inv_z;
    float tminz = min(t0z, t1z); float tmaxz = max(t0z,t1z);
    if (tminz > tmaxz) return false;

    if (max(tminx, max(tminy, tminz)) > min(tmaxx, min(tmaxy, tmaxz))) return false;
    
    return true;
}

std::vector<RTTri *> RTGeometryBuffer::triangles_for_ray(RTRay ray)
{
    vector<RTTri *> triangles_to_return;

    for (int i = 0; i < this->triangles.size(); i++)
    {
        if (box_intersection(this->triangle_bounding_boxes[i]->box_min, this->triangle_bounding_boxes[i]->box_max, this->triangle_bounding_boxes[i]->box_center, ray))
            triangles_to_return.push_back(this->triangles[i]);
    }

    //cout << "triangles given to ray: " << triangles_to_return.size() << endl;
    return triangles_to_return;
}

void RTGeometryBuffer::insert_triangle(RTTri * tri)
{
    //cout << "inserting triangle." << endl;
    this->triangles.push_back(tri);
    //cout << "it touched " << toplevel_blocks_t.size() << " top level blocks" << endl;
    
    RTVector bound_min = RTVector(0);
    RTVector bound_max = RTVector(0);
    // find a bounding box, rounded outwards (from zero) to the nearest block containing the triangle.
    // test intersection with every box between those points. simple!

    // tight bounds
    bound_min.x = min(min(tri->v1.x, tri->v2.x), tri->v3.x);
    bound_max.x = max(max(tri->v1.x, tri->v2.x), tri->v3.x);

    bound_min.y = min(min(tri->v1.y, tri->v2.y), tri->v3.y);
    bound_max.y = max(max(tri->v1.y, tri->v2.y), tri->v3.y);

    bound_min.z = min(min(tri->v1.z, tri->v2.z), tri->v3.z);
    bound_max.z = max(max(tri->v1.z, tri->v2.z), tri->v3.z);

    RTVector bound_center = (bound_min + bound_max) / 2;
    
    RTBoundBox * box = new RTBoundBox();
    box->box_max = bound_max;
    box->box_min = bound_min;
    box->box_center = bound_center;

    this->triangle_bounding_boxes.push_back(box);
    cout << "triangle " << tri->describe() << " loaded with bounds " << bound_min.describe() << ":" << bound_max.describe() << endl;
}

vector<string> split_str(string s, char d)
{
    vector<string> out;
    string current;
    int i = 0;
    while (i < s.size())
    {
        if (s[i] == d)
        {
            out.push_back(current);
            current = "";
        }
        else
        {
            current.push_back(s[i]);
        }
        i++;
    }
    out.push_back(current);
    return out;
}

bool RTGeometryBuffer::load_obj(string path, RTVector offset, RTMaterial * mat)
{
    ifstream file_stream;
    cout << "Loading obj " << path << endl;
    file_stream.open(path, ios::in);
    if (!file_stream.is_open()) { cout << "Couldnt open it :(" << endl; return false; }
    
    vector<RTPoint> verts;
    vector<RTTri *> tris;

    string line;
    while(getline(file_stream, line))
    {
        vector<string> parts = split_str(line, ' ');
        if (parts[0] == "v")
        {
            RTPoint imported_vert = RTPoint(-stof(parts[1]), stof(parts[2]), stof(parts[3]));
            verts.push_back(imported_vert + offset);
            //cout << "imported a vertex at " << imported_vert.describe() << endl;
        }
        if (parts[0] == "f")
        {
            vector<int> indices;
            for (int i = 1; i < parts.size(); i++)
            {
                int ind = stoi(parts[i].substr(0,parts[i].find('/')));
                //cout << "captured index " << ind << endl;;
                indices.push_back(ind-1);
            }
            for (int i = 2; i < indices.size(); i++)
            {
                RTTri * tri = new RTTri(verts[indices[0]], verts[indices[i-1]], verts[indices[i]], mat);
                tris.push_back(tri);
                //cout << "made a new triangle: " << tri->describe() << endl;
            }
        }
    }
    file_stream.close();
    for (RTTri * t : tris)
    {
        this->insert_triangle(t);
    }
    cout << "Loaded " << tris.size() << " tris" << endl;
    
    return true;
}
    