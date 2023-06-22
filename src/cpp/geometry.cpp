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

bool block_intersection(RTVector block_center, float block_radius, RTRay ray)
{
    //cout << ray.direction.describe() << endl;
    //RTVector ray_direction_inv = RTVector(1)/ray.direction;
    //cout << ray_direction_inv.describe() << endl;

    // float tk1 = ((block_center.x-block_radius) - ray.origin.x)/ray_direction_inv.x;
    // float tk2 = ((block_center.x+block_radius) - ray.origin.x)/ray_direction_inv.x;

    // float tmin = max(tmin, min(tk1, tk2));
    // float tmax = min(tmax, max(tk1, tk2));

    // tk1 = ((block_center.y-block_radius) - ray.origin.y)/ray_direction_inv.y;
    // tk2 = ((block_center.y+block_radius) - ray.origin.y)/ray_direction_inv.y;

    // tmin = max(tmin, min(tk1, tk2));
    // tmax = min(tmax, max(tk1, tk2));

    // tk1 = ((block_center.z-block_radius) - ray.origin.z)/ray_direction_inv.z;
    // tk2 = ((block_center.z+block_radius) - ray.origin.z)/ray_direction_inv.z;

    // tmin = max(tmin, min(tk1, tk2));
    // tmax = min(tmax, max(tk1, tk2));

    float t = (ray.direction ^ (block_center - ray.origin)) / (ray.direction ^ ray.direction);
    if (t < -1) return false;
    RTVector p = (ray.direction * t) + ray.origin;

    if (p.x > block_center.x + block_radius) return false;
    if (p.x < block_center.x - block_radius) return false;
    
    if (p.y > block_center.y + block_radius) return false;
    if (p.y < block_center.y - block_radius) return false;
    
    if (p.z > block_center.z + block_radius) return false;
    if (p.z < block_center.z - block_radius) return false;
    
    return true;
}

bool block_intersection(RTVector block_center, float block_radius, RTTri * tri, RTVector vec_c)
{
    RTRay ray1 = RTRay(tri->v1, tri->vec_a);
    RTRay ray2 = RTRay(tri->v1, tri->vec_b);
    RTRay ray3 = RTRay(tri->v2, vec_c);
    return block_intersection(block_center, block_radius, ray1) || block_intersection(block_center, block_radius, ray2) || block_intersection(block_center, block_radius, ray3);
}

int contains_bvh_child(vector<RTBVHNode *> v, RTVector o)
{
    int ind = -1;
    for (RTBVHNode * c : v)
    {
        ind++;
        if (c->center == o) return ind;
    }
    return -1;
}

void bvh_add_child(RTBVHNode * parent, RTBVHNode * child)
{
    for (int i = 0; i < 8; i++)
    {
        if (parent->children[i] == NULL) 
        {
            parent->children[i] = child;
            return;
        }
    }
    cout << "oh no" << endl;
    __builtin_unreachable(); // something has gone horribly horribly wrong if this happens
}

RTBVHNode * RTGeometryBuffer::add_toplevel_block(RTVector origin)
{
    int res = contains_bvh_child(this->bvh_toplevel, origin);
    if (res >= 0) return this->bvh_toplevel[res];

    RTBVHNode * node = new RTBVHNode();
    node->center = origin;
    this->bvh_toplevel.push_back(node);
    return node;
}

std::vector<RTTri *> RTGeometryBuffer::triangles_for_ray(RTRay ray)
{
    //TODO: walk bvh structure
    vector<RTBVHNode *> nodes_to_check = this->bvh_toplevel;
    vector<RTBVHNode *> nodes_to_check_next;
    vector<RTTri *> triangles_to_return;

    int level = 0;
    while (level <= BVH_DIVISION_MAX && !nodes_to_check.empty())
    {
        float radius = BVH_DIVISION_TOPLEVEL_RADIUS*powf(0.5, level);
        for (RTBVHNode * node : nodes_to_check)
        {
            if (block_intersection(node->center, radius, ray))
            {
                //cout << "ray intersected with block " << node->center.describe() << endl;
                if (node->triangles.size() == 1 || level == BVH_DIVISION_MAX) /// needs to be changed when we actually make this work
                {
                    for (RTTri * t : node->triangles)
                        triangles_to_return.push_back(t);
                }
                else
                {
                    for (int i = 0; i < 8; i++)
                    {
                        if (node->children[i] == NULL) break;
                        nodes_to_check_next.push_back(node->children[i]);
                    }
                }
            }
        }
        nodes_to_check.clear();
        nodes_to_check = nodes_to_check_next;
        nodes_to_check_next.clear();
        level++;
    }

    //cout << "triangles given to ray: " << triangles_to_return.size() << endl;
    return triangles_to_return;
}

float round_out(float f, float div_size)
{
    return round(f/div_size)*div_size;
}

float round_out_bvh(float f) { return round_out (f, 2*BVH_DIVISION_TOPLEVEL_RADIUS); }

vector<RTVector> toplevel_blocks_touched(RTTri * tri)
{
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

    // bounds rounded outwards
    bound_min.x = round_out_bvh(bound_min.x); bound_min.y = round_out_bvh(bound_min.y); bound_min.z = round_out_bvh(bound_min.z);
    bound_max.x = round_out_bvh(bound_max.x); bound_max.y = round_out_bvh(bound_max.y); bound_max.z = round_out_bvh(bound_max.z);
    
    RTVector vec_c = tri->v3 - tri->v2;

    vector<RTVector> intersected_blocks;
    for (int z = bound_min.z; z <= bound_max.z; z += BVH_DIVISION_TOPLEVEL_RADIUS*2)
    {
        for (int y = bound_min.y; y <= bound_max.y; y += BVH_DIVISION_TOPLEVEL_RADIUS*2)
        {
            for (int x = bound_min.x; x <= bound_max.x; x += BVH_DIVISION_TOPLEVEL_RADIUS*2)
            {
                RTVector current = RTVector(x,y,z);
                if (block_intersection(current, BVH_DIVISION_TOPLEVEL_RADIUS, tri, vec_c)) intersected_blocks.push_back(current);
            }
        }
    }
    return intersected_blocks;
}

vector<RTVector> blocks_touched(RTTri * tri, int level, RTVector parent_block_origin)
{
    RTVector vec_c = tri->v3 - tri->v2;
    // check 8 blocks within the parent
    float radius = BVH_DIVISION_TOPLEVEL_RADIUS*powf(0.5, level);
    RTVector bld = parent_block_origin + RTVector(-radius, -radius, -radius);
    RTVector blu = parent_block_origin + RTVector(-radius, -radius, radius);
    RTVector brd = parent_block_origin + RTVector(-radius, radius, -radius);
    RTVector bru = parent_block_origin + RTVector(-radius, radius, radius);
    RTVector fld = parent_block_origin + RTVector(radius, -radius, -radius);
    RTVector flu = parent_block_origin + RTVector(radius, -radius, radius);
    RTVector frd = parent_block_origin + RTVector(radius, radius, -radius);
    RTVector fru = parent_block_origin + RTVector(radius, radius, radius);

    vector<RTVector> blocks;
    if (block_intersection(bld, radius, tri, vec_c)) blocks.push_back(bld);
    if (block_intersection(blu, radius, tri, vec_c)) blocks.push_back(blu);
    if (block_intersection(brd, radius, tri, vec_c)) blocks.push_back(brd);
    if (block_intersection(bru, radius, tri, vec_c)) blocks.push_back(bru);
    if (block_intersection(fld, radius, tri, vec_c)) blocks.push_back(fld);
    if (block_intersection(flu, radius, tri, vec_c)) blocks.push_back(flu);
    if (block_intersection(frd, radius, tri, vec_c)) blocks.push_back(frd);
    if (block_intersection(fru, radius, tri, vec_c)) blocks.push_back(fru);

    return blocks;
}

void RTGeometryBuffer::insert_triangle_child(RTTri * tri, int level, RTBVHNode * node)
{
    //cout << "inserting a triangle into child bvh at " << node->center.describe() << endl;
    vector<RTVector> child_blocks_touched = blocks_touched(tri, level, node->center);
    vector<RTBVHNode *> children;

    vector<RTBVHNode *> node_current_children;
    for (int i = 0; i < 8; i++) if (node->children[i] != NULL) node_current_children.push_back(node->children[i]);

    for (RTVector child_block : child_blocks_touched)
    {
        int res = contains_bvh_child(node_current_children, child_block);
        RTBVHNode * child = NULL;
        if (res >= 0) child = node_current_children[res];
        else
        {
            child = new RTBVHNode();
            child->center = child_block;
            bvh_add_child(node, child);
        }
        child->triangles.push_back(tri);
        children.push_back(child);
    }
    
    for (RTBVHNode * child : children)
    {
        if (child->triangles.size() > 1 && level <= BVH_DIVISION_MAX)
        {
            for (RTTri * tri_sub : child->triangles)
            {
                this->insert_triangle_child(tri_sub, level+1, child);
            }
        }
    }
}

void RTGeometryBuffer::insert_triangle(RTTri * tri)
{
    //cout << "inserting triangle." << endl;
    this->triangles.push_back(tri);
    vector<RTVector> toplevel_blocks_t = toplevel_blocks_touched(tri);
    //cout << "it touched " << toplevel_blocks_t.size() << " top level blocks" << endl;
    vector<RTBVHNode *> tlbs;
    for (RTVector tl_block : toplevel_blocks_t)
    {
        RTBVHNode * tlb = this->add_toplevel_block(tl_block);
        tlbs.push_back(tlb);
        tlb->triangles.push_back(tri);
    }

    for (RTBVHNode * tlb : tlbs) this->insert_triangle_child(tri, 1, tlb);
    //cout << "triangle was inserted into " << tlbs.size() << " top level blocks" << endl;
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
            RTPoint imported_vert = RTPoint(stof(parts[1]), stof(parts[2]), stof(parts[3]));
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
    cout << "Here's the loadout: " << endl;
    for (RTBVHNode * n : this->bvh_toplevel)
    {
        cout << n->center.describe() << ":" << n->triangles.size() << endl;
    }
    return true;
}
    