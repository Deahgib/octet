namespace octet {
  class rope_bridge {
    // scene for drawing box
    ref<visual_scene> app_scene;

    std::vector<mesh_instance*> plank_meshes;
    std::vector<btGeneric6DofSpringConstraint*> springs;
    std::vector<btHingeConstraint*> hinges;

  public:
    rope_bridge(ref<visual_scene> root){
      app_scene = root;

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


      for (int i = 0; i < 10; ++i) {
        mat.loadIdentity();
        mat.translate(-7.4f + (i * 1.6f), 4, 0);
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
      
      // Tie up all the planks left to right and save the spring connections in the springs vector.
      for (int i = 0; i < 9; ++i) {
        btRigidBody* rbLeft = plank_meshes[i]->get_node()->get_rigid_body();
        btRigidBody* rbRight = plank_meshes[i + 1]->get_node()->get_rigid_body();
        springs.push_back(app_scene->addSpringConstraint(*rbLeft, *rbRight, localA, localB));
        springs.push_back(app_scene->addSpringConstraint(*rbLeft, *rbRight, localC, localD));

        //hinges.push_back(app_scene->addHingeConstraint(*rbLeft, *rbRight, btVector3(0.8f, 0, 3.5f), btVector3(-0.8f, 0, 3.5f), btVector3(0, 0, 1), btVector3(0, 0, 1)));
        //hinges.push_back(app_scene->addHingeConstraint(*rbLeft, *rbRight, btVector3(0.8f, 0, -3.5f), btVector3(-0.8f, 0, -3.5f), btVector3(0, 0, 1), btVector3(0, 0, 1)));
      }

      // Tie up the bridge to the left anchor
      btRigidBody *leftAnchorRB = leftAnchor->get_node()->get_rigid_body();
      btRigidBody *firstPlank = plank_meshes[0]->get_node()->get_rigid_body();
      
      localA = btTransform::getIdentity();
      localA.setOrigin(get_btVector3(vec3(2, 4, 3.5f)));
      localB = btTransform::getIdentity();
      localB.setOrigin(get_btVector3(vec3(-0.75f, 0, 3.5f)));
      localC = btTransform::getIdentity();
      localC.setOrigin(get_btVector3(vec3(2, 4, -3.5f)));
      localD = btTransform::getIdentity();
      localD.setOrigin(get_btVector3(vec3(-0.75f, 0, -3.5f)));
      springs.push_back(app_scene->addSpringConstraint(*leftAnchorRB, *firstPlank, localA, localB));
      springs.push_back(app_scene->addSpringConstraint(*leftAnchorRB, *firstPlank, localC, localD));
      
      //hinges.push_back(app_scene->addHingeConstraint(*leftAnchorRB, *firstPlank, btVector3(2.2f, 4, 3.5f), btVector3(-0.9f, 0, 3.5f), btVector3(0, 0, 1), btVector3(0, 0, 1)));
      //hinges.push_back(app_scene->addHingeConstraint(*leftAnchorRB, *firstPlank, btVector3(2.2f, 4, -3.5f), btVector3(-0.9f, 0, -3.5f), btVector3(0, 0, 1), btVector3(0, 0, 1)));


      // Tie up the bridge to the right anchor
      btRigidBody *lastPlank = plank_meshes[plank_meshes.size() - 1]->get_node()->get_rigid_body();
      btRigidBody *rightAnchorRB = rightAnchor->get_node()->get_rigid_body();
      
      localA = btTransform::getIdentity();
      localA.setOrigin(get_btVector3(vec3(0.75f, 0, 3.5f)));
      localB = btTransform::getIdentity();
      localB.setOrigin(get_btVector3(vec3(-2, 4, 3.5f)));
      localC = btTransform::getIdentity();
      localC.setOrigin(get_btVector3(vec3(0.75f, 0, -3.5f)));
      localD = btTransform::getIdentity();
      localD.setOrigin(get_btVector3(vec3(-2, 4, -3.5f)));
      springs.push_back(app_scene->addSpringConstraint(*lastPlank, *rightAnchorRB, localA, localB));
      springs.push_back(app_scene->addSpringConstraint(*lastPlank, *rightAnchorRB, localC, localD));
      
      /*
      hinges.push_back(app_scene->addHingeConstraint(*lastPlank, *rightAnchorRB, btVector3(0.9f, 0, 3.5f), btVector3(-2.2f, 4, 3.5f), btVector3(0, 0, 1), btVector3(0, 0, 1)));
      hinges.push_back(app_scene->addHingeConstraint(*lastPlank, *rightAnchorRB, btVector3(0.9f, 0, -3.5f), btVector3(-2.2f, 4, -3.5f), btVector3(0, 0, 1), btVector3(0, 0, 1)));
      */
      // Now set up the dangling tastles.
      std::vector<mesh_instance*> dangle_meshes;
      for (int i = 0; i < 10; ++i) {
        btRigidBody *plankRB = plank_meshes[i]->get_node()->get_rigid_body();

        mat.loadIdentity();
        mat.translate(-7.4f + (i * 1.6f), 3.5f, -3.5f);
        mesh_instance *dangleMesh = app_scene->add_shape(mat, new mesh_sphere(vec3(1, 1, 1), 0.25f), brown, true, 0.2f);
        btRigidBody *dangleRB = dangleMesh->get_node()->get_rigid_body();
        hinges.push_back(app_scene->addHingeConstraint(*plankRB, *dangleRB, btVector3(0, -0.05f, -3.5f), btVector3(0, 0.75f, 0), btVector3(1, 1, 1), btVector3(1, 1, 1)));

        mat.loadIdentity();
        mat.translate(-7.4f + (i * 1.6f), 3.5f, 3.5f);
        dangleMesh = app_scene->add_shape(mat, new mesh_sphere(vec3(1, 1, 1), 0.25f), brown, true, 0.2f);
        dangleRB = dangleMesh->get_node()->get_rigid_body();
        hinges.push_back(app_scene->addHingeConstraint(*plankRB, *dangleRB, btVector3(0, -0.05f, 3.5f), btVector3(0, 0.5f, 0), btVector3(1, 1, 1), btVector3(1, 1, 1)));

      }
    }

    ~rope_bridge() {
    }

    /* Taken from http://bulletphysics.org/Bullet/BulletFull/classbtGeneric6DofSpringConstraint.html#details
    When an index is refered to.
      0 : translation X 
      1 : translation Y 
      2 : translation Z 
      3 : rotation X(3rd Euler rotational around new position of X axis, range[-PI + epsilon, PI - epsilon]) 
      4 : rotation Y(2nd Euler rotational around new position of Y axis, range[-PI / 2 + epsilon, PI / 2 - epsilon]) 
      5 : rotation Z(1st Euler rotational around Z axis, range[-PI + epsilon, PI - epsilon])
    */
    void set_spring_stiffness(float val) {
      std::vector<btGeneric6DofSpringConstraint*>::iterator spring;
      for (spring = springs.begin(); spring != springs.end(); ++spring) {
        (*spring)->setStiffness(0, val);
        (*spring)->setStiffness(1, val);
        (*spring)->setStiffness(2, val);
        (*spring)->setStiffness(3, val);
        (*spring)->setStiffness(4, val);
        (*spring)->setStiffness(5, val);
      }
    }

    void set_spring_damping(float val) {
      std::vector<btGeneric6DofSpringConstraint*>::iterator spring;
      for (spring = springs.begin(); spring != springs.end(); ++spring) {
        (*spring)->setDamping(0, val);
        (*spring)->setDamping(1, val);
        (*spring)->setDamping(2, val);
        (*spring)->setDamping(3, val);
        (*spring)->setDamping(4, val);
        (*spring)->setDamping(5, val);
      }
    }

    void set_spring_linear_upper_limit(btVector3 val) {
      std::vector<btGeneric6DofSpringConstraint*>::iterator spring;
      for (spring = springs.begin(); spring != springs.end(); ++spring) {
        (*spring)->setLinearUpperLimit(val);
      }
    }

  };
}