// Copyright 2019 Arthur Sonzogni. All rights reserved.
// Use of this source code is governed by the MIT license that can be found in
// the LICENSE file.

#ifndef LEVEL_SCREEN_HPP
#define LEVEL_SCREEN_HPP

#include <memory>
#include "activity/Activity.hpp"
#include "plateau.h"

class LevelScreen : public Activity {
 public:
  LevelScreen(smk::Window& window) : Activity(window) {}
  ~LevelScreen() override = default;

  void Step() override;
  void Draw() override;
  void OnEnter() override;

  std::function<void()> on_quit = []{};
  std::function<void()> on_win = []{};
  std::function<void()> on_lose = []{};
  std::string level_path;
  std::unique_ptr<Plateau> plateau;
 private:
  int finger_id;
  glm::vec2 finger_position;
};

#endif /* end of include guard: LEVEL_SCREEN_HPP */
