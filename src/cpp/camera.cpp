#include "../h/camera.h"

#define _USE_MATH_DEFINES
#include <math.h>
#include <float.h>
#include <iostream> // TODO remoe
#include <random>
#define cimg_use_png
#define cimg_display 0
#include "../h/CImg.h"
using namespace std;

void RTCamera::compute_vectors()
{
    this->horizontal_view_size_half = std::tan((this->fov / 2.0f) * (M_PI / 180));  // horizontal size of the view plane for this fov
    this->pixel_aspect_ratio = this->view_size_pixels.v / this->view_size_pixels.u; // height/width
    this->camera_aspect_half = RTVector2D(this->horizontal_view_size_half, this->horizontal_view_size_half * this->pixel_aspect_ratio);
    cout << "Horizontal and vertical aperture size: " << camera_aspect_half.describe() << endl;

    RTVector up = RTVector(0,0,1);
    this->view_horizontal_vec = up % this->direction; this->view_horizontal_vec.norm_self();
    cout << "Horizontal view vector (view x axis): " << view_horizontal_vec.describe() << endl;
    this->view_vertical_vec = this->direction % this->view_horizontal_vec; this->view_vertical_vec.norm_self();
    cout << "Vertical view vector (view y axis): " << view_vertical_vec.describe() << endl;

    this->view_size_inv = RTVector2D(1,1) / this->view_size_pixels;
}

RTRay RTCamera::ray_for_pixel(RTVector2D coord)
{
    RTRay ray = RTRay();
    ray.origin = this->origin;
    //cout << "Camera direction: " << this->direction.describe() << endl;
    RTVector2D uv = (((coord + RTVector2D(0.5, 0.5)) * this->view_size_inv) * RTVector2D(2,-2)) + RTVector2D(-1,1);
    //cout << "Screen coord, 0,0 is centre: " << uv.describe() << endl;

    uv = uv * this->camera_aspect_half;

    RTVector camera_space_uv = (this->view_horizontal_vec * uv.u) + (this->view_vertical_vec * uv.v);
    //cout << "Offset from camera view centre, in world space: " << camera_space_uv.describe() << endl;

    ray.direction = (this->direction + camera_space_uv).norm_copy();
    //cout << "Resulting ray: " << ray.describe() << endl;

    return ray;
}

RTCamera::RTCamera()
{
    this->direction = RTVector(1,0,0);
    this->fov = 90;
    this->origin = RTPoint();
    this->view_size_pixels = RTVector2D(2048, 2048);
    this->exposure = 1;
    this->gamma = 2.2;
    this->compute_vectors();
}

RTCamera::RTCamera(int pixels_x, int pixels_y, float _fov, RTPoint _origin, RTVector _direction, float _exp, float _gam)
{
    this->direction = _direction.norm_copy();
    this->fov = _fov;
    this->origin = _origin;
    this->view_size_pixels = RTVector2D(pixels_x, pixels_y);
    this->exposure = _exp;
    this->gamma = _gam;
    this->compute_vectors();
}

RTColour RTRenderer::specular_sample(RTRay reflected_ray, int depth, RTColour incoming_col)
{
    RTRayCast res = ray_cast(reflected_ray, this->gbuf);
    RTColour col = this->handle_raycast(res, reflected_ray, depth, incoming_col);
    return col;
}

RTColour RTRenderer::diffuse_sample(RTRay reflected_ray, int depth, RTColour incoming_col, float roughness)
{
    RTColour col = RTColour(0);
    float comp_rough = roughness;//std::asin(roughness);
    for (int i = 0; i < DIFFUSE_RAY_COUNT; i++)
    {
        RTRay ray = RTRay(reflected_ray.origin, disturb(reflected_ray.direction, this->diffuse_displacements_precompute[i]*comp_rough));
        RTRayCast res = ray_cast(ray, this->gbuf);
        col = col + this->handle_raycast(res, ray, depth, incoming_col);
    }

    col = col/DIFFUSE_RAY_COUNT;
    return col;
}

float checker(RTPoint p)
{
    #ifndef CHECKER_ENABLE
    return 1;
    #else

    RTPoint h = p + RTVector(CHECKER_SIZE/2);
    RTPoint oddness = RTPoint((int)trunc(h.x*CHECKER_SIZE)%2, (int)trunc(h.y*CHECKER_SIZE)%2, (int)trunc(h.z*CHECKER_SIZE)%2);

    if (oddness.y == oddness.z) return oddness.x;
    else return 1-oddness.x;

    #endif
}

RTColour RTRenderer::handle_raycast(RTRayCast raycast_result, RTRay ray, int current_depth, RTColour incoming_col)
{
    RTColour col;
    if (raycast_result.intersected)
    {
        //cout << "we hit a triangle!" << endl;
        //if (raycast_result.facing < 0) cout << "the back of it, in fact" << endl;
        col = incoming_col * raycast_result.triangle->material->colour * checker ((raycast_result.distance*ray.direction) + ray.origin);
        if (current_depth >= SAMPLE_DEPTH_MAX) return RTColour(0);
        RTRay reflected_ray = get_reflected(ray, raycast_result.distance, raycast_result.triangle->norm*raycast_result.facing);
        if (raycast_result.triangle->material->roughness == 0)
        {
            col = this->specular_sample(reflected_ray, current_depth+1, col);
        }
        else
        {
            col = this->diffuse_sample(reflected_ray, current_depth+1, col, raycast_result.triangle->material->roughness);
        }
    }
    else
    {
        col = incoming_col * sample_sky(ray);
    }
    return col;
}

void RTRenderer::sample_pixel(int x, int y, RTColour * ret)
{
    float rx = ((float)(rand())/(float)(INT_MAX))-0.5;
    float ry = ((float)(rand())/(float)(INT_MAX))-0.5;

    RTRay ray = this->camera->ray_for_pixel(RTVector2D(x+rx,y+ry));
    RTRayCast res = ray_cast(ray, this->gbuf);

    int buf_pos = x + (y * this->camera->view_size_pixels.u);
    // TODO: occlusion

    RTColour col = handle_raycast (res, ray, 0, RTColour(1));
    
    ret[RENDER_PASS_SHADE] = col;
    ret[RENDER_PASS_DEPTH] = RTColour(res.intersected ? res.distance : WORLD_DEPTH_MAX);
    ret[RENDER_PASS_NORMAL] = RTColour(res.intersected ? res.triangle->norm : RTColour(0));
    ret[RENDER_PASS_DEBUG] = RTColour(res.facing);
}

RTColour RTRenderer::sample_sky(RTRay ray)
{
    //cout << "sky sample!" << ray.describe() << " got colour " << sky->sample(ray.direction).describe() << endl;
    return sky->sample(ray.direction);
}

RTColour * RTRenderer::start_render(int s, int worker_threads, RTCamera * _camera, RTSky * _sky, RTGeometryBuffer * _gbuf)
{
    if (this->rendering_now) return this->rbuf;
    this->rendering_done = false;
    this->rendering_cancel = false;

    this->camera = _camera;
    this->sky = _sky;
    this->gbuf = _gbuf;
    this->samples_per_pixel = s;

    this->rbuf_size = this->camera->view_size_pixels.u * this->camera->view_size_pixels.v;
    this->rbuf = new RTColour[rbuf_size*RENDER_PASSES]; // make room for 5 passes: shade, depth, normal, occlude, sample

    this->diffuse_displacements_precompute.clear();
    float r = M_2_PI/DIFFUSE_RAY_COUNT;
    for (int i = 0; i < DIFFUSE_RAY_COUNT; i++)
    {   
        float r_this = r*i;
        RTVector2D displace = RTVector2D(std::cos(r_this), std::sin(r_this));
        this->diffuse_displacements_precompute.push_back(displace);
    }

    this->workers.clear();
    this->workers_done.clear();
    this->samples_completed.clear();
    for (int i = 0; i < worker_threads; i++)
    {
        this->workers_done.push_back(false);
        this->workers.push_back(std::thread(&RTRenderer::render_thread_start, this, i));
        this->samples_completed.push_back(0);
    }
    this->rendering_now = true;

    return this->rbuf;
}

void RTRenderer::wait_for_render()
{
    int last_checked_sample = 0;
    while (!this->rendering_done)
    {
        if (this->samples_completed[0] > last_checked_sample)
        {
            last_checked_sample = this->samples_completed[0];
            cout << "Thread 0 finished sample " << last_checked_sample << endl;
            this->rbuf_export("render.png", RENDER_PASS_SHADE);
        }
        bool b = true;
        for (bool b2 : this->workers_done) b &= b2;
        if (b) this->rendering_done = true;
    };
    cout << "Rendering seems to have finished. Collecting workers." << endl;
    for (int i = 0; i < this->workers.size(); i++)
    {
        this->workers[i].join();
    }

    this->rendering_now = false;

    cout << "Rendering done." << endl;
    return;
}

void RTRenderer::rbuf_write(int buf_pos, int buf_id, float f)
{
    rbuf_write(buf_pos, buf_id, RTColour(f));
}

void RTRenderer::rbuf_write(int buf_pos, int buf_id, RTColour v)
{
    this->rbuf[buf_pos + (buf_id * this->rbuf_size)] = v;
}

RTColour RTRenderer::rbuf_read(int x, int y, int buf_id)
{
    int buf_pos = x + (y * this->camera->view_size_pixels.u);
    return this->rbuf[buf_pos + (buf_id * this->rbuf_size)];
}

RTColour RTRenderer::rbuf_read(int buf_pos, int buf_id)
{
    return this->rbuf[buf_pos + (buf_id * this->rbuf_size)];
}

bool RTRenderer::rbuf_export(std::string file_name, int buf_id)
{
    cout << "Exporting pass " << buf_id << endl;
    int size_x = this->camera->view_size_pixels.u;
    int size_y = this->camera->view_size_pixels.v;
    cimg_library::CImg<uint8_t> image(size_x, size_y, 1, 3, 0);
    uint8_t pixel[3];
    RTColour c;
    for (int y = 0; y < size_y; y++)
    {
        for (int x = 0; x < size_x; x++)
        {
            c = this->rbuf_read(x, y, buf_id);
            //cout << "Pixel " << x << "," << y << ": Colour of " << c.describe() << endl;
            if (buf_id == RENDER_PASS_SHADE) gamma_correct(c, this->camera->exposure, this->camera->gamma, pixel);
            else if (buf_id == RENDER_PASS_DEPTH) gamma_correct(c, 1.0/WORLD_DEPTH_MAX, 1, pixel);
            else gamma_correct(c, 1, 1, pixel);
            //cout << to_string(pixel[0]) << " " << to_string(pixel[1]) << " " << to_string(pixel[2]) << endl;
            image.draw_point(x,y,pixel);
        }
    }
    image.save_png(file_name.c_str());
    return true;
}

void RTRenderer::render_thread_start(int thread_id)
{
    cout << "thread " << thread_id << " started." << endl;
    while(!this->rendering_now);
    cout << "thread " << thread_id << " kicking off." << endl;
    RTColour sample_result[RENDER_PASSES] = {};
    for (int s = 0; s < this->samples_per_pixel; s++)
    {
        int i = 0;
        int buf_pos = thread_id;
        while (buf_pos < this->rbuf_size)
        {
            if (this->rendering_cancel) return;
            this->sample_pixel(buf_pos % (int)(this->camera->view_size_pixels.u), buf_pos / (int)(this->camera->view_size_pixels.u), sample_result);
            this->rbuf_write(buf_pos, RENDER_PASS_SAMPLE, RTColour((float)s/255.0));
            if (s == 0)
            {
                this->rbuf_write(buf_pos, RENDER_PASS_DEPTH, sample_result[RENDER_PASS_DEPTH]);
                this->rbuf_write(buf_pos, RENDER_PASS_NORMAL, sample_result[RENDER_PASS_NORMAL]);
                this->rbuf_write(buf_pos, RENDER_PASS_DEBUG, sample_result[RENDER_PASS_DEBUG]);
            }
            this->rbuf_write(buf_pos, RENDER_PASS_SHADE, 
                ((this->rbuf_read(buf_pos, RENDER_PASS_SHADE)*(s))+sample_result[RENDER_PASS_SHADE])/(s+1)
            );

            buf_pos += this->workers.size();
            i++;
            //cout << thread_id << " thread rendered a pixel." << endl;
        }
        this->samples_completed[thread_id] = s;
        //std::cout << "thread " << thread_id << " did a sample; i drew this many pixels: " << i << std::endl;
    }
    this->workers_done[thread_id] = true;
}

RTRenderer::RTRenderer()
{
    cout << "hello from the renderer!" << endl;
}
