#include "activity/LevelScreen.hpp"

#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <smk/Color.hpp>
#include <smk/Input.hpp>
#include <smk/Sprite.hpp>
#include <smk/Text.hpp>
#include "Generator.hpp"
#include "Resources.hpp"
#include "plateau.h"

void LevelScreen::OnEnter() {
  plateau = std::make_unique<Plateau>();
  plateau->Load(ResourcePath() + "/level/level_intro");
  plateau->Load(level_path);
  plateau->Save(level_path);
}

void LevelScreen::Step() {
  if (window().input().IsKeyHold(GLFW_KEY_LEFT))
    plateau->block.Move(left);
  if (window().input().IsKeyHold(GLFW_KEY_RIGHT))
    plateau->block.Move(right);
  if (window().input().IsKeyHold(GLFW_KEY_UP))
    plateau->block.Move(up);
  if (window().input().IsKeyHold(GLFW_KEY_DOWN))
    plateau->block.Move(down);
  if (window().input().IsKeyReleased(GLFW_KEY_ENTER) ||
      window().input().IsKeyReleased(GLFW_KEY_SPACE) ||
      window().input().IsKeyReleased(GLFW_KEY_ESCAPE)) {
    on_quit();
  }

  if (plateau->IsLevelWinned())
    on_win();

  if (plateau->IsLevelLoosed())
    on_lose();

  plateau->Step();
}

void LevelScreen::Draw() {
  window().Clear(smk::Color::Black);
  plateau->Draw(&window(), window().width(), window().height());
}
