// Copyright 2019 Arthur Sonzogni. All rights reserved.
// Use of this source code is governed by the MIT license that can be found in
// the LICENSE file.

#include "activity/GeneratorScreen.hpp"

#include <cmath>
#include <fstream>
#include <smk/Color.hpp>
#include <smk/Input.hpp>
#include <smk/Shape.hpp>
#include <smk/Sprite.hpp>
#include <smk/Text.hpp>
#include <smk/Vibrate.hpp>

#include "Generator.hpp"
#include "Resources.hpp"

const float tile_space = 0.6f;

GeneratorScreen::GeneratorScreen(smk::Window& window,
                                 Activity* background_activity)
    : Activity(window),
      back_button_(window),
      background_activity_(background_activity) {
  back_button_.on_quit = [&] { on_quit(); };
}

GeneratorScreen::Entry GeneratorScreen::BuildEntry(std::string level,
                                                   int score) {
  auto plateau = std::make_unique<Plateau>();
  plateau->Load(SavePath() + "/generated_level/" + level);
  int min_movement = Evaluation(*plateau);

  float dim = 0.5 * std::max(window().width(), window().height());
  auto framebuffer = smk::Framebuffer(dim, dim);
  plateau->Draw(&framebuffer, dim, dim);
  plateau->Draw(&window(), dim, dim);

  return {
      level,
      std::move(plateau),
      score,
      min_movement,
      std::move(framebuffer),
  };
}

void GeneratorScreen::OnEnter() {
  std::ifstream file(SavePath() + "/generated_level/list");
  std::string level;
  std::string score;
  entries.clear();
  while (std::getline(file, level) && std::getline(file, score))
    entries.push_back(BuildEntry(level, std::stoi(score)));
  entries.push_back({"Generate", nullptr, 0, 0, smk::Framebuffer(100, 100)});
  generate_dy_ = -10.f;

  touch_dx_ = 0.f;
  touch_dxx_ = 0.f;
  touched_ = false;
  selected_index_x = selected_index;
  selected_index_dx = 0.f;
}

void GeneratorScreen::Animate() {
  selected_index_dx += (selected_index - selected_index_x) * 0.003;
  selected_index_dx *= 0.9;
  selected_index_x +=
      (selected_index - selected_index_x) * 0.03 + selected_index_dx;
  background_activity_->Animate();
  generate_dy_ += (0.f - generate_dy_) * 0.1;
}

void GeneratorScreen::Step() {
  int selected_index_previous = selected_index;
  float width = window().width();
  float height = window().height();
  float zoom = std::min(width, height);
  bool select_by_touch = false;

  if (window().input().IsKeyPressed(GLFW_KEY_LEFT))
    selected_index--;
  if (window().input().IsKeyPressed(GLFW_KEY_RIGHT))
    selected_index++;
  if (window().input().IsKeyPressed(GLFW_KEY_ESCAPE))
    on_quit();

  if (window().input().touches().size()) {
    touched_ = true;
    for (auto& it : window().input().touches()) {
      auto& touch = it.second;
      auto delta = touch.data_points.back().position -
                   touch.data_points.front().position;
      touch_dxx_ += ((delta.x - touch_dx_) - touch_dxx_) * 0.5f;
      touch_dx_ = delta.x;
      touch_position = touch.data_points.front().position;
    }
  } else {
    if (touched_) {
      touched_ = false;
      float trigger = std::min(window().width(), window().height()) * 0.01f;
      if (std::abs(touch_dx_) < trigger && std::abs(selected_index_dx) < 0.01f) {
        bool in_strip = std::abs((touch_position.y - window().height() * 0.5) /
                                 (zoom * tile_space)) < 0.5f;
        int new_selected_index = std::round(
            selected_index_x +
            (touch_position.x - window().width() * 0.5) / (zoom * tile_space));
        int new_selected_index_clamped = new_selected_index;
        new_selected_index_clamped = std::max(new_selected_index_clamped, 0);
        new_selected_index_clamped =
            std::min(new_selected_index_clamped, (int)entries.size() - 1);

        if (new_selected_index_clamped == new_selected_index && in_strip) {
          smk::Vibrate(20);
          selected_index = new_selected_index_clamped;
          select_by_touch = true;
        }
      } else if (!select_by_touch) {
        selected_index_dx -= touch_dxx_ / (zoom * tile_space);
        selected_index_x -= touch_dx_ / (zoom * tile_space);
        selected_index = std::round(selected_index_x -
                                    50.f * touch_dxx_ / (zoom * tile_space));
      }
      touch_dx_ = 0.f;
      touch_dxx_ = 0.f;
    }
  }

  if (window().input().IsKeyReleased(GLFW_KEY_ENTER) ||
      window().input().IsKeyReleased(GLFW_KEY_SPACE) || select_by_touch) {
    PlaySound(sound_menu_select);
    if (selected_index == (int)entries.size() - 1)
      on_generate();
    else
      on_selected();
  }

  back_button_.Step();

  Animate();

  selected_index = std::max(selected_index, 0);
  selected_index = std::min(selected_index, (int)entries.size() - 1);

  if (selected_index_previous != selected_index)
    PlaySound(sound_menu_change);
}

void GeneratorScreen::Draw() {
  window().Clear(smk::Color::Black);
  background_activity_->Draw();

  glClear(GL_STENCIL_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glFrontFace(GL_CCW);
  glDisable(GL_CULL_FACE);
  glDisable(GL_DEPTH_TEST);

  // Set the view.
  float width = window().width();
  float height = window().height();
  float zoom = std::min(width, height);
  smk::View view;
  view.SetCenter(0, 0);
  view.SetSize(width / zoom, height / zoom);
  window().SetShaderProgram(window().shader_program_2d());
  window().SetView(view);

  auto square = smk::Shape::Square();
  square.SetColor({0.f, 0.f, 0.f, 0.9});
  square.SetScale(width / zoom, height / zoom);
  square.SetPosition(-width / zoom * 0.5, -height / zoom * 0.5);
  window().Draw(square);

  auto draw_level = [&](int level) {
    float dx = (level - selected_index_x) * tile_space;
    float dy = (level == (int)entries.size() - 1) ? generate_dy_ : 0.f;

    dx += touch_dx_ / zoom;

    // Tile with texture.
    {
      auto& framebuffer = entries[level].framebuffer;
      auto sprite = smk::Sprite(framebuffer);
      sprite.SetPosition(dx - 0.25, -0.2 + dy);
      sprite.SetScale(0.5 / framebuffer.color_texture.width(),
                      0.5 / framebuffer.color_texture.height());
      window().Draw(sprite);
    }

    {
      square.SetPosition(dx - 0.25, -0.2 + dy);
      square.SetScale(0.5, 0.1);
      square.SetColor({0.f, 0.f, 0.f, 0.2f});
      window().Draw(square);
    }

    // Title.
    {
      auto text = smk::Text(font_arial, entries[level].name);
      auto dimension = text.ComputeDimensions();
      float scale = 0.05 / 70.0;
      text.SetColor(smk::Color::White);
      text.SetScale(scale, scale);
      text.SetPosition(dx - dimension.x * 0.5 * scale, -0.2f + dy);
      window().Draw(text);
    }

    // Min_movement.
    if (entries[level].nb_move_player != -1) {
      std::string score = std::to_string(entries[level].nb_move_player) + "/" +
                          std::to_string(entries[level].nb_move_min);
      auto text = smk::Text(font_arial, score);
      auto dimension = text.ComputeDimensions();
      float scale = 0.05 / 70.0;
      text.SetColor(smk::Color::White);
      text.SetScale(scale, scale);
      text.SetPosition(dx - dimension.x * 0.5 * scale, -0.2f + dy + 0.5);
      window().Draw(text);
    }
  };

  for (int level = 0; level < (int)entries.size(); ++level)
    draw_level(level);

  back_button_.Draw();
}

void GeneratorScreen::Save(int nb_move) {
  int& nb_player_move = entries[selected_index].nb_move_player;
  if (nb_player_move == -1) {
    nb_player_move = nb_move;
  } else {
    nb_player_move = std::min(nb_player_move, nb_move);
  }

  {
    std::ofstream file(SavePath() + "/generated_level/list");
    for (int i = 0; i < (int)entries.size() - 1; ++i) {
      auto& entry = entries[i];
      file << entry.name << std::endl;
      file << entry.nb_move_player << std::endl;
    }
  }
  SyncFilesystem();
}
