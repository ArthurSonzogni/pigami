#ifndef ACTIVITY_GENERATE_SCREEN_HPP
#define ACTIVITY_GENERATE_SCREEN_HPP

#include <memory>
#include "Generator.hpp"
#include "activity/Activity.hpp"

class GenerateScreen : public Activity {
 public:
  GenerateScreen(smk::Screen& screen, Activity* background_activity)
      : Activity(screen), background_activity_(background_activity) {}
  ~GenerateScreen() override = default;

  void Animate() override;
  void Step() override;
  void Draw() override;
  void OnEnter() override;

  std::function<void()> on_done = [] {};
  std::unique_ptr<Generator> generator;

  int size_x = 0;
  int size_y = 0;
 private:
  void Save();

  float background_alpha = 0.f;
  Activity* background_activity_;
};

#endif /* end of include guard: ACTIVITY_GENERATE_SCREEN_HPP */
