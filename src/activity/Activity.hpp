// Copyright 2019 Arthur Sonzogni. All rights reserved.
// Use of this source code is governed by the MIT license that can be found in
// the LICENSE file.

#ifndef ACTIVITY_H
#define ACTIVITY_H

#include <smk/Window.hpp>

class Activity {
 public:
  Activity(smk::Window& window) : window_(window) {}
  virtual ~Activity() = default;

  virtual void Animate() {}
  virtual void Step() = 0;
  virtual void Draw() = 0;

  virtual void OnEnter() {}
  virtual void OnQuit() {}

 protected:
  smk::Window& window() { return window_; }

 private:
  smk::Window& window_;
};

#endif /* end of include guard: ACTIVITY_H */
