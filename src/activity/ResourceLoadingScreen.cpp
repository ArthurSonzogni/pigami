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

  smk::View view;
  view.SetCenter(320, 240);
  view.SetSize(640, 480);
  window().SetView(view);

  smk::Text text;
  text.SetFont(font_arial);
  text.SetString("Decoding: " + *resource.path);
  text.SetColor(smk::Color::White);
  text.SetPosition({10.f, 480.f - 60.f});
  text.SetScale(0.33,0.33);
  window().Draw(text);

  resource.Load();
}
