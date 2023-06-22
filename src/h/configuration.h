#ifndef CONFIGURATION_H
#define CONFIGURAITON_H

#include "maths.h"

#include <string.h>
#include <vector>

struct RTConfiguration
{
    RTVector2D canvas = RTVector2D(256, 256);
    float fov = 60;
    RTPoint camera_origin = RTPoint(5,-5,5);
    RTVector camera_vector = RTVector(-1,1,-1);
    float exposure = 1;
    float gamma = 2.2;

    RTColour sky_horizon = RTColour(0.86274509804, 0.98823529412, 1.00000000000);
    RTColour sky_upper = RTColour(0.51372549020, 0.65490196078, 0.96470588235);
    RTColour sky_lower = RTColour(0.25294117647, 0.20784313725, 0.36274509804);

    RTColour demo_mat_colour = RTColour(1,0,1);
    float demo_mat_roughness = 0;

    std::string geom_path = "suzanne.obj";
    RTVector geom_offset = RTVector(0,0,0);

    int render_samples = 1;
    int render_threads = 4;

    RTConfiguration (std::string file);
};

#endif