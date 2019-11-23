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
  if (screen().input().IsKeyHold(GLFW_KEY_LEFT))
    plateau->block.Move(left);
  if (screen().input().IsKeyHold(GLFW_KEY_RIGHT))
    plateau->block.Move(right);
  if (screen().input().IsKeyHold(GLFW_KEY_UP))
    plateau->block.Move(up);
  if (screen().input().IsKeyHold(GLFW_KEY_DOWN))
    plateau->block.Move(down);
  if (screen().input().IsKeyReleased(GLFW_KEY_ENTER) ||
      screen().input().IsKeyReleased(GLFW_KEY_SPACE) ||
      screen().input().IsKeyReleased(GLFW_KEY_ESCAPE)) {
    on_quit();
  }

  if (plateau->IsLevelWinned())
    on_win();

  if (plateau->IsLevelLoosed())
    on_lose();

  plateau->Step();
}

void LevelScreen::Draw() {
  screen().Clear(smk::Color::Black);
  plateau->Draw(&screen(), screen().width(), screen().height());
}
