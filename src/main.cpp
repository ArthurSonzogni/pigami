// Copyright 2019 Arthur Sonzogni. All rights reserved.
// Use of this source code is governed by the MIT license that can be found in
// the LICENSE file.

#include <filesystem>
#include <iostream>
#include <locale>
#include <smk/Audio.hpp>
#include <smk/Input.hpp>
#include <smk/Window.hpp>
#include <smk/Shape.hpp>
#include <smk/Sound.hpp>
#include <smk/Text.hpp>
#include "Resources.hpp"
#include "activity/GenerateScreen.hpp"
#include "activity/GeneratorScreen.hpp"
#include "activity/IntroScreen.hpp"
#include "activity/LevelScreen.hpp"
#include "activity/LevelSelector.hpp"
#include "activity/ResourceLoadingScreen.hpp"
#include "save.hpp"

class Main {
 public:
  Main()
      : resource_loading_screen_(window_),
        intro_screen_(window_),
        level_selector_(window_, &intro_screen_),
        level_screen_(window_),
        generator_screen_(window_, &intro_screen_),
        generate_screen_(window_, &generator_screen_) {
    window_ = smk::Window(640, 480, "Pigami");
    Display(&resource_loading_screen_);

    // ─────────── ResourceLoadingScreen ───────────
    resource_loading_screen_.on_quit = [&] { Display(&intro_screen_); };

    // ─────────── IntroScreen ───────────
    intro_screen_.on_quit = [&] { Display(&level_selector_); };

    // ─────────── LevelSelector ───────────
    level_selector_.on_selected = [&] {
      level_screen_.level_path =
          ResourcePath() + "/level/level" +
          std::to_string(level_selector_.selection_level_selected + 1);
      level_screen_.on_win = [&] {
        save::level_completed(level_selector_.selection_level_selected);
        Display(&level_selector_);
      };
      level_screen_.on_lose = [&] { Display(&level_selector_); };
      level_screen_.on_quit = [&] { Display(&level_selector_); };
      Display(&level_screen_);
    };
    level_selector_.on_generator = [&] { Display(&generator_screen_); };

    // ─────────── GeneratorScreen ───────────
    generator_screen_.on_quit = [&] { Display(&level_selector_); };
    generator_screen_.on_generate = [&] {
      int n = 3 * generator_screen_.selected_index + 12;
      int& a = generate_screen_.size_x = 0;
      int& b = generate_screen_.size_y = 0;
      for (a = std::sqrt(n); a > 3; --a) {
        if (n % a == 0) {
          b = n / a;
          break;
        }
      }
      if (b == 0) {
        a = std::sqrt(n);
        b = std::sqrt(n);
      }
      Display(&generate_screen_);
    };
    generator_screen_.on_selected = [&] {
      auto& entry = generator_screen_.entries[generator_screen_.selected_index];
      level_screen_.level_path = SavePath() + "/generated_level/" + entry.name;
      level_screen_.on_win = [&] {
        generator_screen_.Save(level_screen_.plateau->nb_move);
        Display(&generator_screen_);
      };
      level_screen_.on_lose = [&] { Display(&generator_screen_); };
      level_screen_.on_quit = [&] { Display(&generator_screen_); };
      Display(&level_screen_);
      level_screen_.plateau->ActiveExtScreen(entry.nb_move_min);
    };

    // ─────────── GenerateScreen ───────────
    generate_screen_.on_done = [&] { Display(&generator_screen_); };

    // --
    frame = 0;
    start_time = window_.time();
  }

  void Display(Activity* activity) {
    if (activity_)
      activity_->OnQuit();
    if (activity)
      activity->OnEnter();
    std::swap(activity_, activity);
  }

  void Loop() {
    float new_time = window_.time();
    int new_frame = (new_time - start_time) * 120;

    if (new_frame > frame + 10) {
      std::cout << "Skip " << new_frame - frame << " frames" << std::endl;
      frame = 0;
      new_frame = 1;
      start_time = new_time;
    }

    // ---- Step part ----
    while (frame < new_frame) {
      window_.PoolEvents();
      activity_->Step();
      ++frame;
    }

    // ---- Draw part ----
    activity_->Draw();
    window_.Display();
#ifndef __EMSCRIPTEN__
    window_.LimitFrameRate(60.f);
#endif

    // ---- Background music
    if (frame % 120 && !background_music.IsPlaying()) {
      background_music = smk::Sound(sound_background);
      background_music.Play();
    }
  }

  void Run() {
    window_.ExecuteMainLoop([&] { Loop(); });
  }

 private:
  Activity* activity_;
  smk::Window window_;

  ResourceLoadingScreen resource_loading_screen_;
  IntroScreen intro_screen_;
  LevelSelector level_selector_;
  LevelScreen level_screen_;
  GeneratorScreen generator_screen_;
  GenerateScreen generate_screen_;

  smk::Sound background_music;

  int frame;
  float start_time;
};

int main() {
  static smk::Audio audio;
  std::locale::global(std::locale("C.UTF-8"));

#ifdef __EMSCRIPTEN__
  // clang-format off
  EM_ASM(
      FS.mkdir("/sav");
      FS.mount(IDBFS, {}, "/sav");
      FS.syncfs(true, function(err){console.log("IndexDB synced", err)});
  , 0);
  // clang-format on
#else
  std::filesystem::create_directory(SavePath());
  std::filesystem::create_directory(SavePath() + "/generated_level");
#endif

  static Main main;
  main.Run();

  return EXIT_SUCCESS;
}
