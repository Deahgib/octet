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
    btGeneric6DofSpringConstraint* spring;
    float test_ball_mass;

    /*
    Code taken from Andy Thomason on Github https://github.com/andy-thomason/read_a_csv_file/blob/master/main.cpp
    */
    std::vector<float> load_file() {
      std::vector<float> masses;

      std::ifstream is("masses.csv");

      if (is.bad()) return std::vector<float>();

      // store the line here
      char buffer[2048];

      // loop over lines
      while (!is.eof()) {
        is.getline(buffer, sizeof(buffer));

        // loop over columns
        char *b = buffer;
        for (int col = 0; ; ++col) {
          char *e = b;
          while (*e != 0 && *e != ',') ++e;

          masses.push_back(std::atof(b));

          if (*e != ',') break;
          b = e + 1;
        }
      }
      return masses;
    }

  public:
    example_shapes(int argc, char **argv) : app(argc, argv) {
    }

    ~example_shapes() {
    }

    /// this is called once OpenGL is initialized
    void app_init() {
      app_scene =  new visual_scene();
      app_scene->create_default_camera_and_lights();
      app_scene->get_camera_instance(0)->get_node()->translate(vec3(0, 4, 0));

      std::vector<float> masses = load_file();
      if (masses.empty()) return;
      test_ball_mass = masses[2];

      // ground
      material *green = new material(vec4(0, 1, 0, 1));
      mat4t mat;
      mat.translate(0, -4, 0);
      app_scene->add_shape(mat, new mesh_box(vec3(200, 1, 200)), green, false);

      // Create a ropebridge object
      rope_bridge bridge(app_scene, masses);

      bridge.set_spring_linear_upper_limit(btVector3(0.02f,0,0));

    }

    /// this is called to draw the world
    void draw_world(int x, int y, int w, int h) {
      int vx = 0, vy = 0;
      get_viewport_size(vx, vy);

      if (is_key_going_up(key_space)) {
        material *red = new material(vec4(1, 0, 0, 1));
        mat4t mat;
        mat.translate(7, 20, 0);
        app_scene->add_shape(mat, new mesh_sphere(vec3(1, 1, 1), 1), red, true);
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
