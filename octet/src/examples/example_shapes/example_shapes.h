////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012-2014
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
#include "rope_bridge.h"
namespace octet {
  /// Scene containing a box with octet.
  class example_shapes : public app {
    // scene for drawing box
    ref<visual_scene> app_scene;

  public:
    example_shapes(int argc, char **argv) : app(argc, argv) {
    }

    ~example_shapes() {
    }

    /// this is called once OpenGL is initialized
    void app_init() {
      app_scene =  new visual_scene();
      app_scene->create_default_camera_and_lights();
      app_scene->get_camera_instance(0)->get_node()->translate(vec3(0, 5, 0));

      // ground
      material *green = new material(vec4(0, 1, 0, 1));
      mat4t mat;
      mat.translate(0, -4, 0);
      app_scene->add_shape(mat, new mesh_box(vec3(200, 1, 200)), green, false);


      rope_bridge bridge(app_scene);

      bridge.set_spring_linear_upper_limit(btVector3(0.2f,0,0));

    }

    /// this is called to draw the world
    void draw_world(int x, int y, int w, int h) {
      int vx = 0, vy = 0;
      get_viewport_size(vx, vy);

      if (is_key_going_up(key_space)) {
        material *white = new material(vec4(1, 1, 1, 1));
        mat4t mat;
        mat.translate(7, 20, 0);
        app_scene->add_shape(mat, new mesh_sphere(vec3(1, 1, 1), 1), white, true);
        // Don't abuse! Those spheres will stick around in memory :D
      }

      glClearColor(0, 0, 1, 1);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

      app_scene->begin_render(vx, vy);

      // update matrices. assume 30 fps.
      app_scene->update(1.0f/30);

      // draw the scene
      app_scene->render((float)vx / vy);
    }
  };
}
