#ifndef CAMERA_H
#define CAMERA_H

#include "maths.h"
#include "geometry.h"
#include "raycast.h"
#include <thread>

#define RENDER_PASS_SHADE   0
#define RENDER_PASS_DEPTH   1
#define RENDER_PASS_NORMAL  2
#define RENDER_PASS_OCCLUDE 3
#define RENDER_PASS_SAMPLE 4
#define RENDER_PASS_DEBUG 5

#define SAMPLE_DEPTH_MAX 4
#define DIFFUSE_RAY_COUNT 4
#define WORLD_DEPTH_MAX 100

struct RTCamera
{
    RTPoint origin;
    RTVector direction; // don't set this to be directly up or down. i don't know what happens if you do, just dont

    RTVector view_horizontal_vec; // dont manually modify these. bad things will happen
    RTVector view_vertical_vec;
    float horizontal_view_size_half;
    float pixel_aspect_ratio;
    RTVector2D camera_aspect_half;

    void compute_vectors();

    float exposure;
    float gamma;

    RTVector2D view_size_pixels;

    float fov;

    RTRay ray_for_pixel(RTVector2D);

    RTCamera();
    RTCamera(int, int, float, RTPoint, RTVector, float, float);
};

struct RTRenderer
{
    RTCamera * camera;
    RTGeometryBuffer * gbuf;
    int samples_per_pixel;
    int rbuf_size;
    RTColour * rbuf;
    RTSky * sky;
    
    bool rendering_now = false;
    bool rendering_done = false;
    bool rendering_cancel = false;

    std::vector<std::thread> workers;
    std::vector<bool> workers_done;

    std::vector<RTVector2D> diffuse_displacements_precompute;

    RTColour specular_sample(RTRay, int, RTColour);
    RTColour diffuse_sample(RTRay, int, RTColour, float);
    RTColour handle_raycast(RTRayCast, RTRay, int, RTColour);

    void sample_pixel(int, int);
    RTColour sample_sky(RTRay);

    RTColour * start_render(int, int, RTCamera *, RTSky *, RTGeometryBuffer *);
    void wait_for_render();
    void cancel_render();
    
    void rbuf_write(int, int, float);
    void rbuf_write(int, int, RTColour);
    RTColour rbuf_read(int, int, int);

    bool rbuf_export(std::string, int);

    void render_thread_start(int);

    RTRenderer();
};

#endif