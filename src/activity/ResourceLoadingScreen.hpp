// Copyright 2019 Arthur Sonzogni. All rights reserved.
// Use of this source code is governed by the MIT license that can be found in
// the LICENSE file.

#ifndef ACTIVITY_RESOURCE_LOADING_SCREEN_HPP
#define ACTIVITY_RESOURCE_LOADING_SCREEN_HPP

#include "Resources.hpp"
#include "activity/Activity.hpp"

class ResourceLoadingScreen : public Activity {
 public:
  ResourceLoadingScreen(smk::Window& window) : Activity(window) {}
  ~ResourceLoadingScreen() override = default;

  void Step() override;
  void Draw() override;
  std::function<void()> on_quit = [] {};

 private:
  ResourceInitializer initializer;
  float time = 0.f;
};

#endif /* end of include guard: ACTIVITY_RESOURCE_LOADING_SCREEN_HPP */
