// Copyright 2019 Arthur Sonzogni. All rights reserved.
// Use of this source code is governed by the MIT license that can be found in
// the LICENSE file.

#include "activity/GeneratorScreen.hpp"
#include <cmath>
#include <fstream>
#include <smk/Color.hpp>
#include <smk/Shape.hpp>
#include <smk/Sprite.hpp>
#include <smk/Text.hpp>
#include "Framebuffer.hpp"
#include "Generator.hpp"
#include "Resources.hpp"

GeneratorScreen::Entry GeneratorScreen::BuildEntry(std::string level,
                                                   int score) {
  auto plateau = std::make_unique<Plateau>();
  plateau->Load(SavePath() + "/generated_level/" + level);
  int min_movement = Evaluation(*plateau);

  float dim = 0.5 * std::max(window().width(), window().height());
  auto framebuffer = Framebuffer(dim, dim);
  framebuffer.Bind();
  //glm::mat4 projection = glm::perspective(70.f, 1.f, 1.f, 40.f);
  //glm::vec3 camera_target = glm::vec3(plateau->block.x, plateau->block.y, 0.f) -
                            //0.5f * glm::vec3(-2.f, -5.f, 0.f);
  //glm::vec3 up_direction = {0.f, 0.f, 1.f};
  //glm::vec3 eye = camera_target + 2.f * glm::vec3(-2.f, -5.f, 5.f);
  //glm::mat4 view = glm::lookAt(eye, camera_target, up_direction);
  //window().SetView(view);

  window().SetShaderProgram(window().shader_program_3d());

  glClearColor(0.5f, 1.f, 1.f, 1.f);
  glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glFrontFace(GL_CCW);
  glDisable(GL_CULL_FACE);
  glDisable(GL_DEPTH_TEST);
  glViewport(0, 0, dim, dim);
  plateau->Draw(&window(), dim, dim);
  framebuffer.GenerateMipmap();
  framebuffer.Unbind();

  return {
      level,
      std::move(plateau),
      score,
      min_movement,
      std::move(framebuffer.color_texture),
  };
}

void GeneratorScreen::OnEnter() {
  std::ifstream file(SavePath() + "/generated_level/list");
  std::string level;
  std::string score;
  entries.clear();
  while (std::getline(file, level) && std::getline(file, score))
    entries.push_back(BuildEntry(level, std::stoi(score)));
  entries.push_back({"Generate", nullptr, 0, 0, smk::Texture()});
  generate_dy_ = -10.f;
  glViewport(-1, -1, window().width(), window().height());
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
  if (window().input().IsKeyReleased(GLFW_KEY_ENTER) ||
      window().input().IsKeyReleased(GLFW_KEY_SPACE)) {
    PlaySound(sound_menu_select);
    if (selected_index == (int)entries.size() - 1)
      on_generate();
    else
      on_selected();
  }
  int selected_index_previous = selected_index;

  // clang-format off
  if (window().input().IsKeyPressed(GLFW_KEY_LEFT)) selected_index--;
  if (window().input().IsKeyPressed(GLFW_KEY_RIGHT)) selected_index++;
  if (window().input().IsKeyPressed(GLFW_KEY_ESCAPE)) on_quit();
  // clang-format on

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
    float dx = (level - selected_index_x) * 0.6;
    float dy = (level == (int)entries.size() - 1) ? generate_dy_ : 0.f;

    // Tile with texture.
    {
      auto& texture = entries[level].texture;
      auto sprite = smk::Sprite(texture);
      // sprite.SetColor(entries[level].color);
      sprite.SetPosition(dx - 0.25, -0.2 + 0.5 + dy);
      sprite.SetScale(0.5 / texture.width, -0.5 / texture.height);
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
