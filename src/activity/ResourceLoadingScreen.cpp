// Copyright 2019 Arthur Sonzogni. All rights reserved.
// Use of this source code is governed by the MIT license that can be found in
// the LICENSE file.

#include "activity/ResourceLoadingScreen.hpp"
#include <cmath>
#include <smk/Color.hpp>
#include <smk/Text.hpp>

void ResourceLoadingScreen::Step() {
}

void ResourceLoadingScreen::Draw() {
  window().Clear(smk::Color::Black);

  if (initializer.resources.empty()) {
    on_quit();
    return;
  }

  ResourceInitializer::Resource resource = initializer.resources.front();
  initializer.resources.pop_front();

  float width = window().width();
  float height = window().height();
  float zoom = std::min(width / 640.f, height / 480.f);
  width /= zoom;
  height /= zoom;
  smk::View view;
  view.SetCenter(320.f, 240.f);
  view.SetSize(width, height);
  window().SetView(view);

  smk::Text text;
  text.SetFont(font_arial);
  text.SetString("Decoding: " + *resource.path);
  text.SetColor(smk::Color::White);
  text.SetPosition({10.f, 240.f + height * 0.5 - 60.f});
  text.SetScale(0.33 * zoom ,0.33 * zoom);
  window().Draw(text);

  resource.Load();
}
