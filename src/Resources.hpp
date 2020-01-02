// Copyright 2019 Arthur Sonzogni. All rights reserved.
// Use of this source code is governed by the MIT license that can be found in
// the LICENSE file.

#ifndef RESOURCE_HPP
#define RESOURCE_HPP

#include <list>
#include <smk/Font.hpp>
#include <smk/OpenGL.hpp>
#include <smk/SoundBuffer.hpp>
#include <smk/Texture.hpp>
#include <variant>

std::string ResourcePath();
std::string SavePath();
void PlaySound(const smk::SoundBuffer& snd);
void SyncFilesystem();

extern smk::Font font_arial;

extern smk::Texture texture_block;
extern smk::Texture texture_bouton1;
extern smk::Texture texture_bouton2;
extern smk::Texture texture_bouton3;
extern smk::Texture texture_box;
extern smk::Texture texture_box_press_enter;
extern smk::Texture texture_box_title;
extern smk::Texture texture_dalle;
extern smk::Texture texture_dalleback;
extern smk::Texture texture_fragile1;
extern smk::Texture texture_fragile2;
extern smk::Texture texture_fragile3;
extern smk::Texture texture_left_arrow;
extern smk::Texture texture_level_background;
extern smk::Texture texture_level_circle;
extern smk::Texture texture_number_little;
extern smk::Texture texture_number;
extern smk::Texture texture_press_enter;
extern smk::Texture texture_retractable;
extern smk::Texture texture_right_arrow;
extern smk::Texture texture_skybox_back;
extern smk::Texture texture_skybox_bottom;
extern smk::Texture texture_skybox_front;
extern smk::Texture texture_skybox_left;
extern smk::Texture texture_skybox_right;
extern smk::Texture texture_skybox_top;

extern smk::SoundBuffer sound_background;
extern smk::SoundBuffer sound_fermeture;
extern smk::SoundBuffer sound_ouverture;
extern smk::SoundBuffer sound_win;
extern smk::SoundBuffer sound_lose;
extern smk::SoundBuffer sound_menu_change;
extern smk::SoundBuffer sound_menu_select;
extern smk::SoundBuffer sound_press_enter;
extern smk::SoundBuffer sound_success;

class ResourceInitializer {
 public:
  ResourceInitializer();

  struct Resource {
    smk::Font* font = nullptr;
    smk::Texture* texture = nullptr;
    smk::SoundBuffer* soundbuffer = nullptr;
    std::string* path = nullptr;
    void Load();
  };
  std::list<Resource> resources;
};

#endif  // RESOURCE_HPP
