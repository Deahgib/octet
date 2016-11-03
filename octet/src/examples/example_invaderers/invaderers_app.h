////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012-2014
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
// invaderer example: simple game with sprites and sounds
//
// Level: 1
//
// Demonstrates:
//   Basic framework app
//   Shaders
//   Basic Matrices
//   Simple game mechanics
//   Texture loaded from GIF file
//   Audio
//
// Edited by Louis Bennette 
// 
// Sound fire.wav supplied by http://soundbible.com/1326-Tank-Firing.html 
// Explode bang.wav suppiled by http://soundbible.com/538-Blast.html
// Textures:
// tank & gun modified from : http://www.clipartlord.com/wp-content/uploads/2014/06/tank9.png Site: http://www.clipartlord.com/category/military-clip-art/tanks-clip-art/
// Plane : http://www.clker.com/clipart-flying-cartoon-plane.html
// Blimp : http://www.clipartpanda.com/clipart_images/add-this-clip-art-to-your-34326959
// The rest of the art assets are original creations mocked up in photoshop

#include "sprite.h"
#include "fadeout_sprite.h"

namespace octet {
  class invaderers_app : public octet::app {
    // Matrix to transform points in our camera space to the world.
    // This lets us move our camera
    mat4t cameraToWorld;

    // shader to draw a textured triangle
    texture_shader texture_shader_;

    enum {
      num_sound_sources = 8,
      num_rows = 5,
      num_cols = 10,
      num_missiles = 10,
      num_bombs = 2,
      num_borders = 4,
      num_planes = 10, // Number of sprites to pool
      num_blimps = 10, // number of sprites to pool


      first_ground_sprite = 0,
      last_ground_sprite = first_ground_sprite + 1,

      first_sky_sprite,
      last_sky_sprite = first_sky_sprite + 1,

      background_sprite_anchor, // used to scroll the background along.
      enemy_despawn_anchor,

      // sprite definitions
      tank_sprite,
      gun_sprite,

      first_plane_sprite,
      last_plane_sprite = first_plane_sprite + num_planes - 1,

      first_blimp_sprite,
      last_blimp_sprite = first_blimp_sprite + num_blimps - 1,

      first_missile_sprite,
      last_missile_sprite = first_missile_sprite + num_missiles - 1,

      first_bomb_sprite,
      last_bomb_sprite = first_bomb_sprite + num_bombs - 1,

      upgrade_invins_sprite,
      upgrade_atkspeed_sprite,
      upgrade_lifeup_sprite,

      first_border_sprite,
      last_border_sprite = first_border_sprite + num_borders - 1,

      first_enemy_anchor,
      last_enemy_anchor = first_enemy_anchor + num_rows - 1,
      upgrade_anchor,

      game_over_sprite,

      num_sprites
    };

    enum {
      start_sprite = 0,
      invinsible_msg_sprite,
      atkspeed_msg_sprite,
      lifeup_msg_sprite,
      num_fadeout_sprites
    };

    // timers for missiles and bombs
    int missiles_disabled;
    int bombs_disabled;

    // Screen Size (Viewport size NOT windows window size) and mouse coords
    int mouse_x, mouse_y;
    int screen_w, screen_h;

    // Powerups used if powerups are active and for how long.
    bool invinsible_active;
    int invinsible_time;
    bool attack_speed_active;
    int attack_speed_time;

    // accounting for bad guys
    int num_lives;

    // game state
    bool game_over;
    int score;
    int score_multiplier;

    // speed of enemy
    float enemy_velocity;

    // sounds
    ALuint whoosh;
    ALuint bang;
    unsigned cur_source;
    ALuint sources[num_sound_sources];

    // big array of sprites
    sprite sprites[num_sprites];
    fadeout_sprite fadeout_sprites[num_fadeout_sprites];

    // random number generator
    class random randomizer;

    // a texture for our text
    GLuint font_texture;

    // information for our text
    bitmap_font font;

    ALuint get_sound_source() { return sources[cur_source++ % num_sound_sources]; }

    // Enemy spawning schedule
    struct spawn_time {
      int time; // nb of frames between previous spawn and this spawn
      int type; // 1 = plane, 2 = blimp, 3 = life up, 4 = attack speed +, 5 = invinsibility
      int spawn_anchor; // index 0 to 4
    };
    int spawning_disabled;
    std::vector<spawn_time> entity_schedule;
    std::vector<spawn_time>::iterator entity_schelude_it;
    void spawn_enemies() {
      if (spawning_disabled > 0) {
        --spawning_disabled;
      }
      else{
        if (entity_schelude_it >= entity_schedule.end()) {
          entity_schelude_it = entity_schedule.begin();
          enemy_velocity -= 0.01f;
          //printf("Scheduler finished\n");
          return;
        }
        // Do plane sprites
        if ((*entity_schelude_it).type == 1) { // PLANES
          for (int i = 0; i < num_planes; ++i) {
            if (!sprites[first_plane_sprite + i].is_enabled()) {
              sprites[first_plane_sprite + i].set_relative(sprites[first_enemy_anchor + (*entity_schelude_it).spawn_anchor],0,0);
              sprites[first_plane_sprite + i].is_enabled() = true;
              break;
            }
          }
        }
        else if ((*entity_schelude_it).type == 2) { // BLIMPS
          for (int i = 0; i < num_blimps; ++i) {
            if (!sprites[first_blimp_sprite + i].is_enabled()) {
              sprites[first_blimp_sprite + i].set_relative(sprites[first_enemy_anchor + (*entity_schelude_it).spawn_anchor], 0, 0);
              sprites[first_blimp_sprite + i].is_enabled() = true;
              break;
            }
          }
        }
        else if((*entity_schelude_it).type == 3) { // LIFE UP upgrade
          if (!sprites[upgrade_lifeup_sprite].is_enabled()) {
            sprites[upgrade_lifeup_sprite].set_relative(sprites[upgrade_anchor], 0, 0);
            sprites[upgrade_lifeup_sprite].is_enabled() = true;
          }
        }
        else if ((*entity_schelude_it).type == 4) { // ATTACK SPEED upgrade
          if (!sprites[upgrade_atkspeed_sprite].is_enabled()) {
            sprites[upgrade_atkspeed_sprite].set_relative(sprites[upgrade_anchor], 0, 0);
            sprites[upgrade_atkspeed_sprite].is_enabled() = true;
          }
        }
        else if ((*entity_schelude_it).type == 5) { // INVINSIBILITY upgrade
          if (!sprites[upgrade_invins_sprite].is_enabled()) {
            sprites[upgrade_invins_sprite].set_relative(sprites[upgrade_anchor], 0, 0);
            sprites[upgrade_invins_sprite].is_enabled() = true;
          }
        }

        spawning_disabled = (*entity_schelude_it).time;
        ++entity_schelude_it;
      }
    }

    void load_level() {
      /*
        Code taken from Andy Thomason on Github https://github.com/andy-thomason/read_a_csv_file/blob/master/main.cpp
      */
      std::ifstream is("../../../assets/invaderers/level.csv");
      if (is.bad() || !is.is_open()) return;

      // store the line here
      char buffer[2048];

      // loop over lines
      while (!is.eof()) {
        is.getline(buffer, sizeof(buffer));

        spawn_time enemy;
        // loop over columns
        char *b = buffer;
        for (int col = 0; ; ++col) {
          char *e = b;
          while (*e != 0 && *e != ',') ++e;

          // now b -> e contains the chars in a column
          if (col == 0) {
            enemy.time = std::atoi(b);
          }
          else if (col == 1) {
            enemy.type = std::atoi(b);
          }
          else if (col == 2) {
            enemy.spawn_anchor = std::atoi(b);
          }

          if (*e != ',') break;
          b = e + 1;
        }

        entity_schedule.push_back(enemy);
        //printf("enemy: %i | %i | %i\n", enemy.time, enemy.type, enemy.spawn_anchor);
      }
      entity_schelude_it = entity_schedule.begin();
    }

    // called when we hit an enemy
    void on_hit_invaderer() {
      ALuint source = get_sound_source();
      alSourcei(source, AL_BUFFER, bang);
      alSourcePlay(source);

      score += score_multiplier;
      ++score_multiplier;
    }

    // called when we are hit
    void on_hit_ship() {
      ALuint source = get_sound_source();
      alSourcei(source, AL_BUFFER, bang);
      alSourcePlay(source);
      score_multiplier = 1;
      if (--num_lives == 0) {
        game_over = true;
        sprites[game_over_sprite].translate(-20, 0);
      }
    }

    // use the keyboard to move the ship
    void move_ship() {
      const float ship_speed = 0.05f;
      // left and right arrows
      if (is_key_down(key_left) || is_key_down(key_a)) {
        sprites[tank_sprite].translate(-ship_speed, 0);
        if (sprites[tank_sprite].collides_with(sprites[first_border_sprite+2])) {
          sprites[tank_sprite].translate(+ship_speed, 0);
        }
      } else if (is_key_down(key_right) || is_key_down(key_d)) {
        sprites[tank_sprite].translate(+ship_speed, 0);
        if (sprites[tank_sprite].collides_with(sprites[first_border_sprite+3])) {
          sprites[tank_sprite].translate(-ship_speed, 0);
        }
      }
      float x, y;
      sprites[tank_sprite].get_position(cameraToWorld, x, y);
      sprites[gun_sprite].set_position(x-0.02f, y+0.2f);

    }

    // fire button (space) or Left click Mouse
    void fire_missiles() {
      if (missiles_disabled) {
        --missiles_disabled;
      } else if (is_key_going_down(' ') || is_key_going_down(key_lmb)) {
        // find a missile
        for (int i = 0; i != num_missiles; ++i) {
          if (!sprites[first_missile_sprite+i].is_enabled()) {
            // Set the missle to be relative to the gun of the tank so it follows the direction the gun is pointing.
            sprites[first_missile_sprite+i].set_relative(sprites[gun_sprite], 0, 0.5f);
            sprites[first_missile_sprite+i].is_enabled() = true;
            missiles_disabled = (attack_speed_active ? 2 : 8);
            ALuint source = get_sound_source();
            alSourcei(source, AL_BUFFER, whoosh);
            alSourcePlay(source);
            break;
          }
        }
      }
    }

    // pick and invader and fire a bomb
    void fire_bombs() {
      if (bombs_disabled) {
        --bombs_disabled;
      } else {
        // find an invaderer
        sprite &ship = sprites[tank_sprite];
        for (int j = randomizer.get(0, num_planes); j < num_planes; ++j) {
          sprite &invaderer = sprites[first_plane_sprite+j];
          if (invaderer.is_enabled() && invaderer.is_above(ship, 0.3f)) {
            // find a bomb
            for (int i = 0; i != num_bombs; ++i) {
              if (!sprites[first_bomb_sprite+i].is_enabled()) {
                sprites[first_bomb_sprite+i].set_relative(invaderer, 0, -0.25f);
                sprites[first_bomb_sprite+i].is_enabled() = true;
                bombs_disabled = 30;
                ALuint source = get_sound_source();
                alSourcei(source, AL_BUFFER, whoosh);
                alSourcePlay(source);
                return;
              }
            }
            return;
          }
        }
        for (int j = randomizer.get(0, num_blimps); j < num_blimps; ++j) {
          sprite &blimp = sprites[first_blimp_sprite + j];
          if (blimp.is_enabled() && blimp.is_above(ship, 0.3f)) {
            // find a bomb
            for (int i = 0; i != num_bombs; ++i) {
              if (!sprites[first_bomb_sprite + i].is_enabled()) {
                sprites[first_bomb_sprite + i].set_relative(blimp, 0, -0.25f);
                sprites[first_bomb_sprite + i].is_enabled() = true;
                bombs_disabled = 30;
                ALuint source = get_sound_source();
                alSourcei(source, AL_BUFFER, whoosh);
                alSourcePlay(source);
                return;
              }
            }
            return;
          }
        }
      }
    }

    // animate the missiles
    void move_missiles() {
      const float missile_speed = 0.3f;
      for (int i = 0; i != num_missiles; ++i) {
        sprite &missile = sprites[first_missile_sprite+i];
        if (missile.is_enabled()) {
          missile.translate(0, missile_speed);
          for (int j = 0; j != num_planes; ++j) {
            sprite &invaderer = sprites[first_plane_sprite+j];
            if (invaderer.is_enabled() && missile.collides_with(invaderer)) {
              invaderer.is_enabled() = false;
              invaderer.translate(20, 0);
              missile.is_enabled() = false;
              missile.translate(20, 0);
              on_hit_invaderer();

              goto next_missile;
            }
          }
          for (int j = 0; j != num_blimps; ++j) {
            sprite &blimp = sprites[first_blimp_sprite + j];
            if (blimp.is_enabled() && missile.collides_with(blimp)) {
              blimp.is_enabled() = false;
              blimp.translate(20, 0);
              missile.is_enabled() = false;
              missile.translate(20, 0);
              on_hit_invaderer();

              goto next_missile;
            }
          }
          if (missile.collides_with(sprites[first_border_sprite+1])) {
            missile.is_enabled() = false;
            missile.translate(20, 0);
            score_multiplier = 1;
          }else if (missile.collides_with(sprites[first_border_sprite + 2])) {
            missile.is_enabled() = false;
            missile.translate(20, 0);
            score_multiplier = 1;
          }else if (missile.collides_with(sprites[first_border_sprite + 3])) {
            missile.is_enabled() = false;
            missile.translate(20, 0);
            score_multiplier = 1;
          }
        }
      next_missile:;
      }
    }

    // animate the bombs
    void move_bombs() {
      const float bomb_speed = 0.2f;
      for (int i = 0; i != num_bombs; ++i) {
        sprite &bomb = sprites[first_bomb_sprite+i];
        if (bomb.is_enabled()) {
          bomb.translate(0, -bomb_speed);
          if (bomb.collides_with(sprites[tank_sprite])) {
            bomb.is_enabled() = false;
            bomb.translate(20, 0);
            bombs_disabled = 50;
            on_hit_ship();
            goto next_bomb;
          }
          if (bomb.collides_with(sprites[first_border_sprite+0])) {
            bomb.is_enabled() = false;
            bomb.translate(20, 0);
          }
        }
        next_bomb:;
      }
    }

    // move the array of enemies (Two types of enemies blimps and planes)
    void move_enemies() {
      for (int j = 0; j != num_planes; ++j) {
        sprite &plane = sprites[first_plane_sprite+j];
        if (plane.is_enabled()) {
          plane.translate(enemy_velocity, 0);
          if (plane.collides_with(sprites[enemy_despawn_anchor])) {
            plane.is_enabled() = false;
            if (--num_lives == 0) {
              game_over = true;
              sprites[game_over_sprite].translate(-20, 0);
            }
          }
        }
      }
      for (int j = 0; j != num_blimps; ++j) {
        sprite &blimp = sprites[first_blimp_sprite + j];
        if (blimp.is_enabled()) {
          blimp.translate(enemy_velocity * 0.5f, 0);
          if (blimp.collides_with(sprites[enemy_despawn_anchor])) {
            blimp.is_enabled() = false;
            if (--num_lives == 0) {
              game_over = true;
              sprites[game_over_sprite].translate(-20, 0);
            }
          }
        }
      }
    }


    void draw_text(texture_shader &shader, float x, float y, float scale, const char *text) {
      mat4t modelToWorld;
      modelToWorld.loadIdentity();
      modelToWorld.translate(x, y, 0);
      modelToWorld.scale(scale, scale, 1);
      mat4t modelToProjection = mat4t::build_projection_matrix(modelToWorld, cameraToWorld);

      /*mat4t tmp;
      glLoadIdentity();
      glTranslatef(x, y, 0);
      glGetFloatv(GL_MODELVIEW_MATRIX, (float*)&tmp);
      glScalef(scale, scale, 1);
      glGetFloatv(GL_MODELVIEW_MATRIX, (float*)&tmp);*/

      enum { max_quads = 32 };
      bitmap_font::vertex vertices[max_quads*4];
      uint32_t indices[max_quads*6];
      aabb bb(vec3(0, 0, 0), vec3(512, 512, 0));

      unsigned num_quads = font.build_mesh(bb, vertices, indices, max_quads, text, 0);
      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, font_texture);

      shader.render(modelToProjection, 0, 1);

      glVertexAttribPointer(attribute_pos, 3, GL_FLOAT, GL_FALSE, sizeof(bitmap_font::vertex), (void*)&vertices[0].x );
      glEnableVertexAttribArray(attribute_pos);
      glVertexAttribPointer(attribute_uv, 3, GL_FLOAT, GL_FALSE, sizeof(bitmap_font::vertex), (void*)&vertices[0].u );
      glEnableVertexAttribArray(attribute_uv);

      glDrawElements(GL_TRIANGLES, num_quads * 6, GL_UNSIGNED_INT, indices);
    }

	  void move_background() {
		sprites[first_ground_sprite].translate(-0.05f, 0);
		sprites[last_ground_sprite].translate(-0.05f, 0);
        sprites[first_sky_sprite].translate(-0.025f, 0);
        sprites[last_sky_sprite].translate(-0.025f, 0);

        if (sprites[first_ground_sprite].collides_with(sprites[background_sprite_anchor])) {
          sprites[first_ground_sprite].translate(12,0);
        }
        else if (sprites[last_ground_sprite].collides_with(sprites[background_sprite_anchor])) {
          sprites[last_ground_sprite].translate(12, 0);
        }
        if (sprites[first_sky_sprite].collides_with(sprites[background_sprite_anchor])) {
          sprites[first_sky_sprite].translate(12, 0);
        }
        else if (sprites[last_sky_sprite].collides_with(sprites[background_sprite_anchor])) {
          sprites[last_sky_sprite].translate(12, 0);
        }
	  }

      // This function calculates the angle the tank's gun should point so it's always pointing to where the cursor is on screen
    void do_shoot_angle() {
      app_common::get_mouse_pos(mouse_x, mouse_y);
      float mouse_coord_x = (((float)(mouse_x)-((float)(screen_w) * 0.5f)) / ((float)(screen_w) * 0.5f)) * 3; // Map the mouse coords on the screen with boundaries -3 to 3 for x and y
      float mouse_coord_y = (((float)(mouse_y)-((float)(screen_h) * 0.5f)) / ((float)(screen_h) * 0.5f)) * -3;

      float ship_x, ship_y;
      sprites[tank_sprite].get_position(cameraToWorld, ship_x, ship_y);

      // Work out the angle between the gun and the cursor relative to the y axis
      float dx = ship_x - mouse_coord_x;
      float dy = ship_y - mouse_coord_y;
      // FIRST QUADRANT top left
      if (dx >= 0 && dy < 0) { 
        float radAngle = math::atan2(dx, -dy);
        float angle = radAngle * 180.0f / 3.141592f; // Convert rad angle to degrees
        //printf("Angle %f\n", angle);
        sprites[gun_sprite].set_rotation(angle);
      }
      // SECOND QUADRANT top right
      else if (dx < 0 && dy < 0) { 
        float radAngle = math::atan2(-dx, -dy);
        float angle = -1 * radAngle * 180.0f / 3.141592f; 
        //printf("Angle %f\n", angle);
        sprites[gun_sprite].set_rotation(angle);
      }
      
    }

    // This functions scrolls the upgrades left. The player is guarantied to get the upgrade
    void move_upgrades() {
      if (sprites[upgrade_lifeup_sprite].is_enabled()) {
        sprites[upgrade_lifeup_sprite].translate(-0.05f, 0);
        if (sprites[upgrade_lifeup_sprite].collides_with(sprites[tank_sprite])) {
          sprites[upgrade_lifeup_sprite].is_enabled() = false;
          fadeout_sprites[lifeup_msg_sprite].fade();
          ++num_lives;
        }
      }
      if (sprites[upgrade_atkspeed_sprite].is_enabled()) {
        sprites[upgrade_atkspeed_sprite].translate(-0.05f, 0);
        if (sprites[upgrade_atkspeed_sprite].collides_with(sprites[tank_sprite])) {
          sprites[upgrade_atkspeed_sprite].is_enabled() = false;
          fadeout_sprites[atkspeed_msg_sprite].fade();
          attack_speed_active = true;
          attack_speed_time = 250;
          missiles_disabled = 0;
        }
      }
      if (sprites[upgrade_invins_sprite].is_enabled()) {
        sprites[upgrade_invins_sprite].translate(-0.05f, 0);
        if (sprites[upgrade_invins_sprite].collides_with(sprites[tank_sprite])) {
          sprites[upgrade_invins_sprite].is_enabled() = false;
          fadeout_sprites[invinsible_msg_sprite].fade();
          invinsible_active = true;
          invinsible_time = 250;
        }
      }
    }

    // This function counts down the duration of an active upgrade on the player and removes the upgrade once the 
    // time has elapsed
    void update_upgrades() {
      if (attack_speed_active) {
        --attack_speed_time;
        if (attack_speed_time < 0) {
          attack_speed_active = false;
        }
      }
      if (invinsible_active) {
        --invinsible_time;
        if (invinsible_time < 0) {
          invinsible_active = false;
        }
      }
    }

  public:

    // this is called when we construct the class
    invaderers_app(int argc, char **argv) : app(argc, argv), font(512, 256, "assets/big.fnt") {
    }

    // this is called once OpenGL is initialized
    void app_init() {
      // set up the shader
      texture_shader_.init();

      // set up the matrices with a camera 5 units from the origin
      cameraToWorld.loadIdentity();
      cameraToWorld.translate(0, 0, 3);

      font_texture = resource_dict::get_texture_handle(GL_RGBA, "assets/big_0.gif");

      GLuint tank = resource_dict::get_texture_handle(GL_RGBA, "assets/invaderers/tank.gif");
      sprites[tank_sprite].init(tank, 0, -2.05f, 0.83f, 0.5f);

      GLuint gun = resource_dict::get_texture_handle(GL_RGBA, "assets/invaderers/gun.gif");
      sprites[gun_sprite].init(gun, 0, 0, 0.2f, 0.83f);
      sprites[gun_sprite].rotate(-90);

      GLuint GameOver = resource_dict::get_texture_handle(GL_RGBA, "assets/invaderers/GameOver.gif");
      sprites[game_over_sprite].init(GameOver, 20, 0, 3, 1.5f);

      GLuint plane = resource_dict::get_texture_handle(GL_RGBA, "assets/invaderers/plane.gif");
      for (int i = 0; i != num_planes; ++i) {
        assert(first_plane_sprite + num_planes - 1 <= last_plane_sprite);
        sprites[first_plane_sprite + i ].init(plane, 0, 0, 0.45f, 0.25f);
        sprites[first_plane_sprite + i ].is_enabled() = false;
      }

      GLint blimp = resource_dict::get_texture_handle(GL_RGBA, "assets/invaderers/blimp.gif");
      for (int i = 0; i != num_blimps; ++i) {
        assert(first_blimp_sprite + num_blimps - 1 <= last_blimp_sprite);
        sprites[first_blimp_sprite + i].init(blimp, 0, 0, 0.65f, 0.45f);
        sprites[first_blimp_sprite + i].is_enabled() = false;
      }

      // set the border to white for clarity
      GLuint white = resource_dict::get_texture_handle(GL_RGB, "#ffffff");
      sprites[first_border_sprite+0].init(white, 0, -3.2f, 6.4f, 0.2f);
      sprites[first_border_sprite+1].init(white, 0,  3.2f, 6.4f, 0.2f);
      sprites[first_border_sprite+2].init(white, -3.2f, 0, 0.2f, 6.4f);
      sprites[first_border_sprite+3].init(white, 3.2f,  0, 0.2f, 6.4f);

      // background textures
      GLint ground = resource_dict::get_texture_handle(GL_RGBA, "assets/invaderers/terrain.gif");
      sprites[first_ground_sprite].init(ground, 0, -2.4f, 6, 0.4f);
      sprites[last_ground_sprite].init(ground, 6, -2.4f, 6, 0.4f);
      GLint sky = resource_dict::get_texture_handle(GL_RGBA, "assets/invaderers/sky.gif");
      sprites[first_sky_sprite].init(sky, 0, 0.4f, 6, 5.2f);
      sprites[last_sky_sprite].init(sky, 6, 0.4f, 6, 5.2f);

      // use the missile texture
      GLuint missile = resource_dict::get_texture_handle(GL_RGBA, "assets/invaderers/missile.gif");
      for (int i = 0; i != num_missiles; ++i) {
        // create missiles off-screen
        sprites[first_missile_sprite+i].init(missile, 20, 0, 0.0625f, 0.25f);
        sprites[first_missile_sprite+i].is_enabled() = false;
      }

      // use the bomb texture
      GLuint bomb = resource_dict::get_texture_handle(GL_RGBA, "assets/invaderers/bomb.gif");
      for (int i = 0; i != num_bombs; ++i) {
        // create bombs off-screen
        sprites[first_bomb_sprite+i].init(bomb, 20, 0, 0.0625f, 0.25f);
        sprites[first_bomb_sprite+i].is_enabled() = false;
      }

      GLuint lifeup = resource_dict::get_texture_handle(GL_RGBA, "assets/invaderers/lifeup.gif");
      sprites[upgrade_lifeup_sprite].init(lifeup, 20, 0, 0.25f, 0.25f);
      sprites[upgrade_lifeup_sprite].is_enabled() = false;
      GLuint atkspeed = resource_dict::get_texture_handle(GL_RGBA, "assets/invaderers/atkspeed.gif");
      sprites[upgrade_atkspeed_sprite].init(atkspeed, 20, 0, 0.25f, 0.25f);
      sprites[upgrade_atkspeed_sprite].is_enabled() = false;
      GLuint invins = resource_dict::get_texture_handle(GL_RGBA, "assets/invaderers/invinsibility.gif");
      sprites[upgrade_invins_sprite].init(invins, 20, 0, 0.25f, 0.25f);
      sprites[upgrade_invins_sprite].is_enabled() = false;

      // Fade out messages
      GLuint start_msg = resource_dict::get_texture_handle(GL_RGBA, "assets/invaderers/Start.gif");
      fadeout_sprites[start_sprite].init(start_msg, 0, 0, 3, 1.5f);
      fadeout_sprites[start_sprite].is_enabled() = true;
      GLuint lifeup_msg = resource_dict::get_texture_handle(GL_RGBA, "assets/invaderers/LifeUpMsg.gif");
      fadeout_sprites[lifeup_msg_sprite].init(lifeup_msg, 0, 0, 3, 1.5f);
      fadeout_sprites[lifeup_msg_sprite].is_enabled() = false;
      GLuint atkspeed_msg = resource_dict::get_texture_handle(GL_RGBA, "assets/invaderers/AttackSpeed.gif");
      fadeout_sprites[atkspeed_msg_sprite].init(atkspeed_msg, 0, 0, 3, 1.5f);
      fadeout_sprites[atkspeed_msg_sprite].is_enabled() = false;
      GLuint invinsible_msg = resource_dict::get_texture_handle(GL_RGBA, "assets/invaderers/Invinsible.gif");
      fadeout_sprites[invinsible_msg_sprite].init(invinsible_msg, 0, 0, 3, 1.5f);
      fadeout_sprites[invinsible_msg_sprite].is_enabled() = false;

      // Anchors (Off screen)
      sprites[background_sprite_anchor].init(white, -9.1f, 0, 0.2f, 6);
      sprites[enemy_despawn_anchor].init(white, -3.5f, 0, 0.2f, 6);
      sprites[upgrade_anchor].init(white, 3.5f, -2.05f, 0.25f, 0.25f);
      for (int i = 0; i != num_rows; ++i) {
        sprites[first_enemy_anchor + i].init(white, 3.5f, 2.5f - 0.5f*i, 0.2f, 0.2f);
      }

      // sounds
      whoosh = resource_dict::get_sound_handle(AL_FORMAT_MONO16, "assets/invaderers/fire.wav");
      bang = resource_dict::get_sound_handle(AL_FORMAT_MONO16, "assets/invaderers/bang.wav");
      cur_source = 0;
      alGenSources(num_sound_sources, sources);

      load_level();
      spawning_disabled = 50;

      // sundry counters and game state.
      invinsible_active = false;
      attack_speed_active = false;
      missiles_disabled = 0;
      bombs_disabled = 50;
      enemy_velocity = -0.05f;
      num_lives = 3;
      game_over = false;
      score = 0;
      score_multiplier = 1;
    }

    // called every frame to move things
    void simulate() {
      if (game_over) {
        return;
      }

      spawn_enemies();

      move_ship();

      do_shoot_angle();

      fire_missiles();

      fire_bombs();

      move_missiles();

      move_bombs();

      move_enemies();

      move_background();

      move_upgrades();

      update_upgrades();

    }

    // this is called to draw the world
    void draw_world(int x, int y, int w, int h) {
      simulate();

	  if (game_over && is_key_going_up(key_enter)) {
		  app_init();
		  return;
	  }

      // Local values for viewport
      app_common::get_viewport_size(screen_w, screen_h);
      // set a viewport - includes whole window area
      glViewport(x, y, w, h);

      // clear the background to black
      glClearColor(0, 0, 0, 1);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

      // don't allow Z buffer depth testing (closer objects are always drawn in front of far ones)
      glDisable(GL_DEPTH_TEST);

      // allow alpha blend (transparency when alpha channel is 0)
      glEnable(GL_BLEND);
      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

      // draw all the sprites
      for (int i = 0; i != num_sprites; ++i) {
        sprites[i].render(texture_shader_, cameraToWorld);
      }

      // Fadeout rendering
      for (int i = 0; i < num_fadeout_sprites; ++i) {
        if (fadeout_sprites[i].is_enabled()) {
          fadeout_sprites[i].render(texture_shader_, cameraToWorld);
        }
      }

      char score_text[64];
      sprintf(score_text, "combo: X%d   score: %d   lives: %d\n", score_multiplier, score, num_lives);
      draw_text(texture_shader_, -0.75f, -4.6f, 1.0f/256, score_text);

      // move the listener with the camera
      vec4 &cpos = cameraToWorld.w();
      alListener3f(AL_POSITION, cpos.x(), cpos.y(), cpos.z());
    }
  };
}
