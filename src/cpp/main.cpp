#include "../h/camera.h"
#include "../h/configuration.h"

#include <iostream>

using namespace std;

int main ()
{
    RTConfiguration config ("scenesetup.cfg");

    RTCamera cam = RTCamera (config.canvas.u, config.canvas.v, config.fov, config.camera_origin, config.camera_vector, config.exposure, config.gamma);
    RTSky sky = RTSky ();
    sky.horizon = config.sky_horizon;
    sky.upper = config.sky_upper;
    sky.lower = config.sky_lower;
    
    RTMaterial demo_material = RTMaterial();
    demo_material.colour = config.demo_mat_colour;
    demo_material.roughness = config.demo_mat_roughness;

    // RTTri * tri = new RTTri(
    //     RTPoint(2,0,0),
    //     RTPoint(2,0,1),
    //     RTPoint(3,2,2),
    //     &demo_material
    // );
    // cout << tri->vec_a.describe() << endl;
    // cout << tri->vec_b.describe() << endl;
    // cout << tri->norm.describe() << endl;
    //return 0;

    RTGeometryBuffer geom = RTGeometryBuffer ();
    // geom.insert_triangle(tri);
    // cout << geom.triangles[0]->describe() << endl;
    // cout << geom.triangles_for_ray(RTRay())[0]->describe() << endl;
    geom.load_obj(config.geom_path, config.geom_offset, &demo_material);

    RTRenderer renderer = RTRenderer();

    RTColour * buf = renderer.start_render(config.render_samples, config.render_threads, &cam, &sky, &geom);
    renderer.wait_for_render();
    renderer.rbuf_export("debug.png", RENDER_PASS_DEBUG);
    renderer.rbuf_export("render.png", RENDER_PASS_SHADE);
    renderer.rbuf_export("normal.png", RENDER_PASS_NORMAL);
    renderer.rbuf_export("sample.png", RENDER_PASS_SAMPLE);
    renderer.rbuf_export("depth.png", RENDER_PASS_DEPTH);


    return 0;
}