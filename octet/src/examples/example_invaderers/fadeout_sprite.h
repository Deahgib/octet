
namespace octet {
  class fadeout_sprite : public sprite {
  private:
    float alpha;
  public:
    fadeout_sprite() : sprite() {}
    void init(int _texture, float x, float y, float w, float h) {
      sprite::init(_texture, x, y, w, h);
      alpha = 1;
    }
    void render(texture_shader &shader, mat4t &cameraToWorld) {
      if (alpha > 0) {
        sprite::render(shader, cameraToWorld, alpha);
        alpha -= 0.025f;
      }
      else {
        is_enabled() = false;
      }
    }
    void fade() {
      is_enabled() = true;
      alpha = 1;
    }
  };
}