// Copyright 2019 Arthur Sonzogni. All rights reserved.
// Use of this source code is governed by the MIT license that can be found in
// the LICENSE file.

#include "activity/Backbutton.hpp"
#include <cmath>
#include <smk/Color.hpp>
#include <smk/Shape.hpp>
#include <smk/Text.hpp>
#include "Resources.hpp"

void Backbutton::Step() {
  float width = window().width();
  float height = window().height();
  float zoom = std::min(width, height);
  glm::vec2 center = glm::vec2(width, height) * 0.5f;
  back_button_position =
      glm::vec2(-width, height) * 0.5f / zoom + glm::vec2(0.15f, -0.1f);
  auto delta =
      back_button_position - (window().input().cursor() - center) / zoom;
  delta.x /= 0.1f;
  delta.y /= 0.07f;
  back_button_hover = glm::length(delta) < 1.f;

  if (window().input().IsCursorReleased() && back_button_hover)
    on_quit();
}

void Backbutton::Draw() {
  glDisable(GL_CULL_FACE);
  glDisable(GL_DEPTH_TEST);
  glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
  glStencilFunc(GL_ALWAYS, 0, 0xffffffff);
  glFrontFace(GL_CCW);
  glClear(GL_STENCIL_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // Set the view.
  float width = window().width();
  float height = window().height();
  float zoom = std::min(width, height);
  smk::View view;
  view.SetCenter(0, 0);
  view.SetSize(width / zoom, height / zoom);
  window().SetShaderProgram(window().shader_program_2d());
  window().SetView(view);

  auto circle = smk::Shape::Circle(1.f, 90);
  circle.SetPosition(back_button_position);
  circle.SetColor(back_button_hover ? smk::Color::White : smk::Color::Black);
  circle.SetScale(0.1f, 0.07f);
  window().Draw(circle);

  circle.SetColor(back_button_hover ? smk::Color::Black : smk::Color::White);
  circle.SetScale(0.095f, 0.065f);
  window().Draw(circle);

  auto text = smk::Text(font_arial, L"←");
  text.SetPosition(back_button_position);
  float scale = 0.0015f;
  text.SetColor(back_button_hover ? smk::Color::White : smk::Color::Black);
  text.SetScale(scale, scale);
  text.Move(-0.07f, -0.1f);
  window().Draw(text);
}
