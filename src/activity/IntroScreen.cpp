#include "activity/IntroScreen.hpp"

#include <glm/gtc/matrix_transform.hpp>
#include <smk/Color.hpp>
#include <smk/Input.hpp>
#include <smk/Sprite.hpp>
#include <smk/Text.hpp>
#include "Resources.hpp"

namespace {

std::vector<Direction> intro_movement = {
    right, right, right, right, right, none, up,   up,    up,    up,
    up,    up,    up,    up,    up,    up,   none, right, right, right,
    right, right, none,  down,  down,  down, down, down,  none,  left,
    left,  left,  left,  left,  left,  left, left, left,  left,  none,
    down,  down,  down,  down,  down,  none,
};

}  // namespace

void IntroScreen::OnEnter() {
  PlaySound(sound_press_enter);
  plateau.Load(ResourcePath() + "/level/level_intro");
  camera_position = glm::vec3(plateau.block.x, plateau.block.y, 0.f);
  step = 0;
}

void IntroScreen::Step() {
  if (screen().input().IsKeyHold(GLFW_KEY_LEFT))
    plateau.block.Move(left);
  if (screen().input().IsKeyHold(GLFW_KEY_RIGHT))
    plateau.block.Move(right);
  if (screen().input().IsKeyHold(GLFW_KEY_UP))
    plateau.block.Move(up);
  if (screen().input().IsKeyHold(GLFW_KEY_DOWN))
    plateau.block.Move(down);
  if (screen().input().IsKeyReleased(GLFW_KEY_ENTER) ||
      screen().input().IsKeyReleased(GLFW_KEY_SPACE) ||
      screen().input().IsKeyReleased(GLFW_KEY_ESCAPE)) {
    on_quit();
  }
  Animate();
}

void IntroScreen::Animate() {
  if (step % (rolling_step + 1) == 0) {
    plateau.block.Move(
        intro_movement[step / (rolling_step + 1) % intro_movement.size()]);
  }

  auto camera_target = glm::vec3(plateau.block.x, plateau.block.y, 0.f);
  camera_position += (camera_target - camera_position) * 0.01f;

  plateau.Step();
  ++step;
}

void IntroScreen::Draw() {
  screen().Clear(smk::Color::Black);
  //glm::mat4 projection = glm::perspective(
      //70.f, float(screen().width()) / screen().height(), 1.f, 40.f);

  //float alpha = step * 0.0025f;

  //glm::vec3 eye =
      //camera_position + glm::vec3(cos(alpha) * 10.f, sin(alpha) * 10.f, 5.f);
  //glm::vec3 up_direction = {0.f, 0.f, 1.f};
  //auto view = glm::lookAt(eye, camera_position, up_direction);

  //screen().SetShaderProgram(screen().shader_program_3d());
  //screen().Clear(smk::Color::Black);
  //glClear(GL_STENCIL_BUFFER_BIT);
  //screen().SetView(view);

  plateau.Draw(&screen(), screen().width(), screen().height());
}
