// Copyright 2019 Arthur Sonzogni. All rights reserved.
// Use of this source code is governed by the MIT license that can be found in
// the LICENSE file.

#include "Resources.hpp"
#include <cstdlib>
#include <fstream>
#include <smk/SoundBuffer.hpp>
#include <smk/Sound.hpp>
#include <smk/Texture.hpp>
#include <vector>

#include <iostream>

std::string GetEnvironmentVariable(const char* env) {
  auto value = std::getenv(env);
  if (value)
    return value;
  return std::string();
}

std::string ResourcePath() {
  static bool initialized = false;
  static std::string resource_path;
  if (initialized)
    return resource_path;
  initialized = true;

  auto SNAP = GetEnvironmentVariable("SNAP");

  std::vector<std::string> path_list = {
      // Application installed using snapcraft.
      SNAP + "/share/pigami/resources",
      SNAP + "/usr/share/pigami/resources",
      SNAP + "/usr/local/share/pigami/resources",

      // Application installed using "sudo make install"
      "/usr/share/pigami/resources",
      "/usr/local/share/pigami/resources",

      // Code build and run inside ${CMAKE_CURRENT_DIRECTORY}/build
      "../resources",

      // Code build and run inside ${CMAKE_CURRENT_DIRECTORY}
      "./resources",
  };

  for (auto& path : path_list) {
    std::ifstream file(path + "/pigami");
    std::string line;
    if (std::getline(file, line) && line == "pigami") {
      resource_path = path;
    }
  }

  std::cout << "Resource path = " << resource_path << std::endl;
  return resource_path;
}

std::string SavePath() {
  static bool initialized = false;
  static std::string save_path;

  if (initialized)
    return save_path;
  initialized = true;

#ifdef __EMSCRIPTEN__
  save_path = "/sav";
#else
  auto SNAP_USER_COMMON = GetEnvironmentVariable("SNAP_USER_COMMON");
  auto HOME = GetEnvironmentVariable("HOME");

  if (!SNAP_USER_COMMON.empty()) {
    save_path = SNAP_USER_COMMON;
  } else if (!HOME.empty()) {
    save_path = HOME + "/.config/pigami";
  } else {
    save_path = ".";
  }

#endif
  std::cout << "Save path = " << save_path << std::endl;
  return save_path;
}

smk::Texture texture_block;
smk::Texture texture_bouton1;
smk::Texture texture_bouton2;
smk::Texture texture_bouton3;
smk::Texture texture_box;
smk::Texture texture_box_press_enter;
smk::Texture texture_box_title;
smk::Texture texture_dalle;
smk::Texture texture_dalleback;
smk::Texture texture_fragile1;
smk::Texture texture_fragile2;
smk::Texture texture_fragile3;
smk::Texture texture_left_arrow;
smk::Texture texture_level_background;
smk::Texture texture_level_circle;
smk::Texture texture_number;
smk::Texture texture_retractable;
smk::Texture texture_right_arrow;
smk::Texture texture_skybox_back;
smk::Texture texture_skybox_bottom;
smk::Texture texture_skybox_front;
smk::Texture texture_skybox_left;
smk::Texture texture_skybox_right;
smk::Texture texture_skybox_top;

smk::SoundBuffer sound_background;
smk::SoundBuffer sound_fermeture;
smk::SoundBuffer sound_ouverture;
smk::SoundBuffer sound_win;
smk::SoundBuffer sound_lose;
smk::SoundBuffer sound_menu_change;
smk::SoundBuffer sound_menu_select;
smk::SoundBuffer sound_press_enter;
smk::SoundBuffer sound_success;

smk::Font font_arial;

std::map<smk::Font*, std::string> font_resources{
    {&font_arial, "/font/arial.ttf"},
};

std::map<smk::Texture*, std::string> image_resources{
    {&texture_block, "/img/block.png"},
    {&texture_bouton1, "/img/bouton1.png"},
    {&texture_bouton2, "/img/bouton2.png"},
    {&texture_bouton3, "/img/bouton3.png"},
    {&texture_box, "/img/box.png"},
    {&texture_box_press_enter, "/img/box_press_enter.png"},
    {&texture_box_title, "/img/box_title.png"},
    {&texture_dalle, "/img/dalle.png"},
    {&texture_dalleback, "/img/dalleback.png"},
    {&texture_fragile1, "/img/fragile1.jpg"},
    {&texture_fragile2, "/img/fragile2.png"},
    {&texture_fragile3, "/img/fragile3.png"},
    {&texture_left_arrow, "/img/left_arrow.png"},
    {&texture_level_background, "/img/level_background.png"},
    {&texture_level_circle, "/img/level_circle.png"},
    {&texture_number, "/img/number.png"},
    {&texture_retractable, "/img/retractable.png"},
    {&texture_right_arrow, "/img/right_arrow.png"},
    {&texture_skybox_back, "/img/skybox_back.png"},
    {&texture_skybox_bottom, "/img/skybox_bottom.png"},
    {&texture_skybox_front, "/img/skybox_front.png"},
    {&texture_skybox_left, "/img/skybox_left.png"},
    {&texture_skybox_right, "/img/skybox_right.png"},
    {&texture_skybox_top, "/img/skybox_top.png"},
};

std::map<smk::SoundBuffer*, std::string> sound_resources{
    {&sound_background, "/snd/background.ogg"},
    {&sound_fermeture, "/snd/fermeture.ogg"},
    {&sound_ouverture, "/snd/ouverture.ogg"},
    {&sound_win, "/snd/win.ogg"},
    {&sound_lose, "/snd/lose.wav"},
    {&sound_menu_change, "/snd/menu_change.wav"},
    {&sound_menu_select, "/snd/menu_select.wav"},
    {&sound_press_enter, "/snd/press_enter.wav"},
    {&sound_success, "/snd/success.wav"},
};

ResourceInitializer::ResourceInitializer() {
  for (auto& it : font_resources)
    resources.emplace_back(Resource{it.first, nullptr, nullptr, &it.second});
  for (auto& it : image_resources)
    resources.emplace_back(Resource{nullptr, it.first, nullptr, &it.second});
   for (auto& it : sound_resources)
   resources.emplace_back(Resource{nullptr, nullptr, it.first, &it.second});
}

void ResourceInitializer::Resource::Load() {
  // clang-format off
  if (font) *font = smk::Font(ResourcePath() + *path, 90);
  if (texture) *texture = smk::Texture(ResourcePath() + *path);
  if (soundbuffer) *soundbuffer = smk::SoundBuffer(ResourcePath() + *path);
  // clang-format on
}

namespace {
std::vector<smk::Sound> sounds;
int sound_index = 0;
}  // namespace

void PlaySound(const smk::SoundBuffer& snd) {
  sounds.resize(10);
  sound_index = (sound_index + 1) % sounds.size();
  sounds[sound_index] = smk::Sound(snd);
  sounds[sound_index].Play();
}

void SyncFilesystem() {
#ifdef __EMSCRIPTEN__
  EM_ASM(FS.syncfs(false, function(err){console.log(err)});, 0);
#endif
}
