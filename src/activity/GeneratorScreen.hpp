// Copyright 2019 Arthur Sonzogni. All rights reserved.
// Use of this source code is governed by the MIT license that can be found in
// the LICENSE file.

#ifndef ACTIVITY_GENERATOR_SCREEN_HPP
#define ACTIVITY_GENERATOR_SCREEN_HPP

#include <memory>
#include <smk/Color.hpp>
#include <smk/Framebuffer.hpp>
#include "activity/Activity.hpp"
#include "activity/Backbutton.hpp"
#include "plateau.h"

class GeneratorScreen : public Activity {
 public:
  GeneratorScreen(smk::Window& window, Activity* background_activity);
  ~GeneratorScreen() override = default;

  void Animate() override;
  void OnEnter() override;
  void Step() override;
  void Draw() override;
  void Save(int nb_move);
  std::function<void()> on_selected = [] {};
  std::function<void()> on_generate = [] {};
  std::function<void()> on_quit = [] {};

  struct Entry {
    std::string name;
    std::unique_ptr<Plateau> plateau;
    int nb_move_player;
    int nb_move_min;
    smk::Framebuffer framebuffer;
  };
  std::vector<Entry> entries;
  int selected_index = 0.f;

 private:
  Backbutton back_button_;
  Entry BuildEntry(std::string level, int score);
  Activity* background_activity_;

  float selected_index_x = 0.f;
  float selected_index_dx = 0.f;
  float generate_dy_ = -10.f;

  float touch_dx_ = 0.f;
  float touch_dxx_ = 0.f;
  bool touched_ = false;
  bool back_button_hover = false;

  glm::vec2 touch_position;
  glm::vec2 back_button_position;
};

#endif /* end of include guard: ACTIVITY_GENERATOR_SCREEN_HPP */
