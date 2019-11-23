#ifndef ACTIVITY_GENERATOR_SCREEN_HPP
#define ACTIVITY_GENERATOR_SCREEN_HPP

#include <memory>
#include <smk/Color.hpp>
#include "Framebuffer.hpp"
#include "activity/Activity.hpp"
#include "plateau.h"

class GeneratorScreen : public Activity {
 public:
  GeneratorScreen(smk::Screen& screen, Activity* background_activity)
      : Activity(screen), background_activity_(background_activity) {}
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
    smk::Texture texture;
  };
  std::vector<Entry> entries;
  int selected_index = 0.f;

 private:
  Entry BuildEntry(std::string level, int score);
  Activity* background_activity_;

  float selected_index_x = 0.f;
  float selected_index_dx = 0.f;
  float generate_dy_ = -10.f;
};

#endif /* end of include guard: ACTIVITY_GENERATOR_SCREEN_HPP */
