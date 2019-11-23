#ifndef INTRO_SCREEN_HPP
#define INTRO_SCREEN_HPP

#include <memory>
#include "activity/Activity.hpp"
#include "plateau.h"

class IntroScreen : public Activity {
 public:
  IntroScreen(smk::Screen& screen) : Activity(screen) {}
  ~IntroScreen() override = default;

  void Animate() override;
  void Step() override;
  void Draw() override;
  void OnEnter() override;

  std::function<void()> on_quit = [] {};

 private:
  Plateau plateau;

  glm::vec3 camera_position;
  int step = 0;
  float camera_angle;
};

#endif /* end of include guard: INTRO_SCREEN_HPP */
