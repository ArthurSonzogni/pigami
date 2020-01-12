// Copyright 2019 Arthur Sonzogni. All rights reserved.
// Use of this source code is governed by the MIT license that can be found in
// the LICENSE file.

#include "activity/LevelScreen.hpp"

#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <smk/Color.hpp>
#include <smk/Input.hpp>
#include <smk/Sprite.hpp>
#include <smk/Text.hpp>
#include <smk/Vibrate.hpp>
#include "Generator.hpp"
#include "Resources.hpp"
#include "plateau.h"

LevelScreen::LevelScreen(smk::Window& window)
    : Activity(window), back_button_(window) {
  back_button_.on_quit = [&] { on_quit(); };
}

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

  float trigger = std::min(window().width(), window().height()) * 0.12f;
  for (auto& it : window().input().touches()) {
    auto& touch = it.second;
    auto delta =
        touch.data_points.back().position - touch.data_points.front().position;
    if (glm::length(delta) < trigger)
      continue;
    touch.data_points = {touch.data_points.back()};

    if (plateau->block.animation != wait)
      continue;

    smk::Vibrate(20);

    delta = glm::normalize(delta);
    if (delta.x > +0.5f) {
      plateau->block.Move(right);
      continue;
    }
    if (delta.x < -0.5f) {
      plateau->block.Move(left);
      continue;
    }
    if (delta.y > 0.f) {
      plateau->block.Move(down);
      continue;
    }
    if (delta.y < 0.f) {
      plateau->block.Move(up);
      continue;
    }
  }

  if (plateau->IsLevelWinned())
    on_win();

  if (plateau->IsLevelLoosed())
    on_lose();

  plateau->Step();
  back_button_.Step();
}

void LevelScreen::Draw() {
  window().Clear(smk::Color::Black);
  plateau->Draw(&window(), window().width(), window().height());
  back_button_.Draw();
}
