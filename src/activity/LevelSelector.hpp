// Copyright 2019 Arthur Sonzogni. All rights reserved.
// Use of this source code is governed by the MIT license that can be found in
// the LICENSE file.

#ifndef ACTIVITY_LEVEL_SELECTOR_HPP
#define ACTIVITY_LEVEL_SELECTOR_HPP

#include "activity/Activity.hpp"

class LevelSelector : public Activity {
 public:
  LevelSelector(smk::Window& window, Activity* background_activity);
  ~LevelSelector() override = default;

  std::function<void()> on_selected = [] {};
  std::function<void()> on_generator = [] {};
  int selection_level_selected = 1;

  void OnEnter() override;
  void Step() override;
  void Draw() override;

 private:
  Activity* background_activity_;
  const int nb_level = 12;
  bool first_run = true;
  int max_level;
  std::vector<float> alpha_ = std::vector<float>(nb_level + 1, 0.f);
  float left_arrow_alpha_ = 0.f;
  float right_arrow_alpha_ = 0.f;
  float background_alpha_ = 0.f;
  bool selected_by_touch_ = false;
  float view_dy_ = 0.f;
};

#endif /* end of include guard: ACTIVITY_LEVEL_SELECTOR_HPP */
