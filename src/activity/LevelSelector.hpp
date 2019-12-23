#ifndef ACTIVITY_LEVEL_SELECTOR_HPP
#define ACTIVITY_LEVEL_SELECTOR_HPP

#include "activity/Activity.hpp"

class LevelSelector : public Activity {
 public:
  LevelSelector(smk::Window& window, Activity* background_activity)
      : Activity(window), background_activity_(background_activity) {}
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
  int max_level;
  std::vector<float> alpha_ = std::vector<float>(nb_level + 1, 0.f);
  float left_arrow_alpha_ = 0.f;
  float right_arrow_alpha_ = 0.f;
  float background_alpha_ = 0.f;
};

#endif /* end of include guard: ACTIVITY_LEVEL_SELECTOR_HPP */
