#include "../h/configuration.h"
#include "../h/geometry.h"

#include <fstream>
#include <iostream>

using namespace std;

RTVector2D read_to_v2d(vector<string> l)
{
    return RTVector2D(stof(l[1]), stof(l[2]));
}

RTVector read_to_v(vector<string> l)
{
    return RTVector(stof(l[1]), stof(l[2]), stof(l[3]));
}

float read_to_f(vector<string> l)
{
    return stof(l[1]);
}

int read_to_i(vector<string> l)
{
    return stoi(l[1]);
}

RTConfiguration::RTConfiguration(string file)
{
    ifstream file_stream;
    file_stream.open(file);
    if (!file_stream.is_open()) return;
    cout << "Reading config file " << file << endl;
    
    vector<RTPoint> verts;
    vector<RTTri *> tris;

    string line;
    while(getline(file_stream, line))
    {
        vector<string> parts = split_str(line, ' ');
        if (parts[0] == "canvas") this->canvas = read_to_v2d(parts);
        if (parts[0] == "fov") this->fov = read_to_f(parts);
        if (parts[0] == "camera_origin") this->camera_origin = read_to_v(parts);
        if (parts[0] == "camera_vector") this->camera_vector = read_to_v(parts);
        if (parts[0] == "exposure") this->exposure = read_to_f(parts);
        if (parts[0] == "gamma") this->gamma = read_to_f(parts);
        if (parts[0] == "sky_horizon") this->sky_horizon = read_to_v(parts);
        if (parts[0] == "sky_upper") this->sky_upper = read_to_v(parts);
        if (parts[0] == "sky_lower") this->sky_lower = read_to_v(parts);
        if (parts[0] == "demo_mat_colour") this->demo_mat_colour = read_to_v(parts);
        if (parts[0] == "demo_mat_roughness") this->demo_mat_roughness = read_to_f(parts);
        if (parts[0] == "geom_path") this->geom_path = parts[1];
        if (parts[0] == "geom_offset") this->geom_offset = read_to_v(parts);
        if (parts[0] == "render_samples") this->render_samples = read_to_i(parts);
        if (parts[0] == "render_threads") this->render_threads = read_to_i(parts);
    }
    file_stream.close();
}