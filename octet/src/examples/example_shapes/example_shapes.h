////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012-2014
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
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

      material *red = new material(vec4(0.4f, 0, 0.4f, 1));
      material *green = new material(vec4(0, 1, 0, 1));
      material *blue = new material(vec4(0, 0, 1, 1));
      material *brown = new material(vec4(1, 0.6f, 0, 1));

      mat4t mat;
      mat.translate(-10, 0, 0);
      mesh_instance *leftAnchor = app_scene->add_shape(mat, new mesh_box(vec3(2, 4, 6)), red, false);

      mat.loadIdentity();
      mat.translate(10, 0, 0);
      mesh_instance *rightAnchor = app_scene->add_shape(mat, new mesh_box(vec3(2, 4, 6)), red, false);

      // ground
      mat.loadIdentity();
      mat.translate(0, -4, 0);
      app_scene->add_shape(mat, new mesh_box(vec3(200, 1, 200)), green, false);

      // TODO :Load player capsule mesh and attach the camera as a child node to the player rigidbody. Then modify 
      // movement code for the player rigid body

      std::vector<mesh_instance*> plank_meshes;
      for (int i = 0; i < 10; ++i) {
        mat.loadIdentity();
        mat.translate(-10 + (i*2), 4, 0);
        mesh_instance* temp = app_scene->add_shape(mat, new mesh_box(vec3(0.8f, 0.05f, 4)), brown, true);
        plank_meshes.push_back(temp);
      }

      btTransform localA = btTransform::getIdentity();
      localA.setOrigin(get_btVector3(vec3(0.75f, 0, 3.5f)));
      btTransform localB = btTransform::getIdentity();
      localB.setOrigin(get_btVector3(vec3(-0.75f, 0, 3.5f)));
      btTransform localC = btTransform::getIdentity();
      localC.setOrigin(get_btVector3(vec3(0.75f, 0, -3.5f)));
      btTransform localD = btTransform::getIdentity();
      localD.setOrigin(get_btVector3(vec3(-0.75f, 0, -3.5f)));
      // Tie up all the planks left to right
      for (int i = 0; i < 9; ++i) {
        btRigidBody* rbLeft = plank_meshes[i]->get_node()->get_rigid_body();
        btRigidBody* rbRight = plank_meshes[i+1]->get_node()->get_rigid_body();
        app_scene->addSpringConstraint(*rbLeft, *rbRight, localA, localB);
        app_scene->addSpringConstraint(*rbLeft, *rbRight, localC, localD);
      }

      localA = btTransform::getIdentity();
      localA.setOrigin(get_btVector3(vec3(2, 4, 3.5f)));
      localB = btTransform::getIdentity();
      localB.setOrigin(get_btVector3(vec3(-0.75f, 0, 3.5f)));
      localC = btTransform::getIdentity();
      localC.setOrigin(get_btVector3(vec3(2, 4, -3.5f)));
      localD = btTransform::getIdentity();
      localD.setOrigin(get_btVector3(vec3(-0.75f, 0, -3.5f)));

      btRigidBody *leftAnchorRB = leftAnchor->get_node()->get_rigid_body();
      btRigidBody *firstPlank = plank_meshes[0]->get_node()->get_rigid_body();
      app_scene->addSpringConstraint(*leftAnchorRB, *firstPlank, localA, localB);
      app_scene->addSpringConstraint(*leftAnchorRB, *firstPlank, localC, localD);

      localA = btTransform::getIdentity();
      localA.setOrigin(get_btVector3(vec3(0.75f, 0, 3.5f)));
      localB = btTransform::getIdentity();
      localB.setOrigin(get_btVector3(vec3(-2, 4, 3.5f)));
      localC = btTransform::getIdentity();
      localC.setOrigin(get_btVector3(vec3(0.75f, 0, -3.5f)));
      localD = btTransform::getIdentity();
      localD.setOrigin(get_btVector3(vec3(-2, 4, -3.5f)));

      btRigidBody *lastPlank = plank_meshes[plank_meshes.size()-1]->get_node()->get_rigid_body();
      btRigidBody *rightAnchorRB = rightAnchor->get_node()->get_rigid_body();
      app_scene->addSpringConstraint(*lastPlank, *rightAnchorRB, localA, localB);
      app_scene->addSpringConstraint(*lastPlank, *rightAnchorRB, localC, localD);

    }

    void move_camera() {
      if (is_key_down(key_w)) {
        app_scene->get_camera_instance(0)->get_node()->translate(vec3(0,0,-1));
      }
      if (is_key_down(key_s)) {
        app_scene->get_camera_instance(0)->get_node()->translate(vec3(0, 0, 1));
      }
      if (is_key_down(key_a)) {
        app_scene->get_camera_instance(0)->get_node()->rotate(5, vec3(0,1,0));
      }
      if (is_key_down(key_d)) {
        app_scene->get_camera_instance(0)->get_node()->rotate(-5, vec3(0, 1, 0));
      }
    }

    /// this is called to draw the world
    void draw_world(int x, int y, int w, int h) {
      int vx = 0, vy = 0;
      get_viewport_size(vx, vy);

      move_camera();

      if (is_key_going_up(key_space)) {
        material *white = new material(vec4(0, 0.3f, 0, 1));
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
