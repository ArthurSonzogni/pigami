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
};

#endif /* end of include guard: LEVEL_SCREEN_HPP */
