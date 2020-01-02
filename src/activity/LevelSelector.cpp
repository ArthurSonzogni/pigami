#include "activity/LevelSelector.hpp"
#include <cmath>
#include <iostream>
#include <smk/BlendMode.hpp>
#include <smk/Color.hpp>
#include <smk/Shape.hpp>
#include <smk/Sprite.hpp>
#include <smk/Text.hpp>
#include <smk/Vibrate.hpp>
#include "Resources.hpp"
#include "save.hpp"

void LevelSelector::OnEnter() {
  max_level = save::max_level();
  max_level = std::min(max_level, nb_level - 2);
  view_dy_ = 0.f;
}

void LevelSelector::Step() {
  background_activity_->Animate();
  background_alpha_ += (0.8 - background_alpha_) * 0.01;
  if (window().input().IsKeyReleased(GLFW_KEY_ENTER) ||
      window().input().IsKeyReleased(GLFW_KEY_SPACE) ||
      (selected_by_touch_ && window().input().IsCursorReleased())) {
    PlaySound(sound_menu_select);
    if (selection_level_selected == nb_level - 1)
      on_generator();
    else
      on_selected();
    smk::Vibrate(50);
  }

  int selection_level_selected_previous = selection_level_selected;

  float trigger = std::min(window().width(), window().height()) * 0.2f;
  bool must_select_by_touch = false;
  float view_dy_target = 0.f;
  for(auto& it : window().input().touches()) {
    auto& touch = it.second;
    auto delta =
        touch.data_points.back().position - touch.data_points.front().position;
    std::cerr << "delta = " << glm::length(delta) << std::endl;
    if (glm::length(delta) < trigger)
      continue;
    view_dy_target = delta.y;
    delta = glm::normalize(delta);
    if (std::abs(delta.y) > +0.5f) {
      if (!selected_by_touch_)
        smk::Vibrate(20);
      must_select_by_touch = true;
      continue;
    }
    if (delta.x > +0.5f) selection_level_selected++;
    if (delta.x < -0.5f) selection_level_selected--;
    // 'Consume' the datapoint.
    touch.data_points = {touch.data_points.back()};
  }
  selected_by_touch_ = must_select_by_touch;
  view_dy_ += (view_dy_target - view_dy_) * 0.1f;

  if (window().input().IsKeyPressed(GLFW_KEY_LEFT))
    selection_level_selected--;
  if (window().input().IsKeyPressed(GLFW_KEY_RIGHT))
    selection_level_selected++;

  selection_level_selected = std::max(selection_level_selected, 0);
  selection_level_selected = std::min(selection_level_selected, max_level + 1);

  if (selection_level_selected_previous != selection_level_selected) {
    PlaySound(sound_menu_change);
    smk::Vibrate(20);
  }

  // Compute alpha_ for every elements
  {
    for (int i = 0; i < nb_level; ++i) {
      float target = selection_level_selected >= i ? 1.f : 0.f;
      alpha_[i] += (target - alpha_[i]) * 0.1;
    } alpha_[nb_level] = alpha_[nb_level - 1];

    float left_arrow_alpha_target = selection_level_selected == 0 ? 0.f : 1.f;
    float right_arrow_alpha_target =
        selection_level_selected == max_level + 1 ? 0.f : 1.f;

    left_arrow_alpha_ += (left_arrow_alpha_target - left_arrow_alpha_) * 0.05;
    right_arrow_alpha_ += (right_arrow_alpha_target - right_arrow_alpha_) * 0.05;
  }
}

void LevelSelector::Draw() {
  background_activity_->Draw();

  window().SetShaderProgram(window().shader_program_2d());

  glDisable(GL_DEPTH_TEST);
  glDisable(GL_STENCIL_TEST);
  glFrontFace(GL_CCW);
  glDisable(GL_CULL_FACE);

  // Background slightly black.
  {
    smk::View view;
    view.SetCenter(0.5f, 0.5f);
    view.SetSize(1.f, 1.f);
    window().SetView(view);

    auto square = smk::Shape::Square();
    square.SetColor({0.f, 0.f, 0.f, background_alpha_});
    window().Draw(square);
  }

  // Set the view.
  float margin = 10.f;
  float width = window().width();
  float height = window().height();
  float zoom = std::min(width / 640.f, height / 480.f);
  width /= zoom;
  height /= zoom;
  smk::View view;
  view.SetCenter(320.f, 240.f - view_dy_ / zoom);
  view.SetSize(width, height);
  window().SetView(view);

  // Display the level number.
  float number_dx = texture_number.width / 4.f;
  {
    auto number_sprite = smk::Sprite(texture_number);
    number_sprite.SetBlendMode(smk::BlendMode::Add);
    int x = (selection_level_selected) % 4;
    int y = (selection_level_selected) / 4;
    float number_dx = texture_number.width / 4.f;
    float number_dy = texture_number.height / 3.f;
    number_sprite.SetTextureRectangle({number_dx * x, number_dy * y,
                                       number_dx * (x + 1),
                                       number_dy * (y + 1)});
    number_sprite.SetPosition(320.f - number_dx * 0.5f,
                              240.f - number_dy * 0.5f);
    window().Draw(number_sprite);
  }

  // Helper function. Returns the positions of the circles.
  auto circle_position = [&](int i) {
    float angle = 2.0 * M_PI * (float(i) / nb_level) + M_PI;
    return glm::vec2(320 + 200 * cos(angle),
                     240 + 200 * sin(angle));
  };

  // Lines around circles.
  {
    for (int i = 1; i < nb_level + 1; ++i) {
      for (int width : {48}) {
        auto line =
            smk::Shape::Line(circle_position(i - 1), circle_position(i), width);
        float a = std::max(0.f, alpha_[i] * 2.f - 1.f) * 0.1;
        line.SetColor({1.f, 1.f, 1.f, a});
        line.SetBlendMode(smk::BlendMode::Add);
        window().Draw(line);
      }
    }
  }

  // Circles.
  {
    auto sprite_level_circle = smk::Sprite(texture_level_circle);
    sprite_level_circle.SetBlendMode(smk::BlendMode::Add);
    float scale = 0.5;
    for (int i = 0; i < nb_level; ++i) {
      sprite_level_circle.SetScale(scale, scale);
      sprite_level_circle.SetPosition(circle_position(i));
      sprite_level_circle.Move(-texture_level_circle.width * 0.5 * scale,
                               -texture_level_circle.height * 0.5 * scale);
      float a = std::min(1.f, alpha_[i] * 2.f / 1.f);
      sprite_level_circle.SetColor({1.f, 1.f, 1.f, a});
      window().Draw(sprite_level_circle);
    }
  }

  // Left arrow.
  {
    auto sprite = smk::Sprite(texture_left_arrow);
    sprite.SetBlendMode(smk::BlendMode::Add);
    sprite.SetPosition(320 - number_dx * 0.5, 240);
    sprite.SetColor({1.0, 1.0, 1.0, left_arrow_alpha_});
    sprite.Move(-texture_left_arrow.width, -texture_left_arrow.height * 0.5);
    window().Draw(sprite);
  }

  // Right arrow.
  {
    auto sprite = smk::Sprite(texture_right_arrow);
    sprite.SetBlendMode(smk::BlendMode::Add);
    sprite.SetPosition(320 + number_dx * 0.5, 240);
    sprite.SetColor({1.0, 1.0, 1.0, right_arrow_alpha_});
    sprite.Move(0, -texture_right_arrow.height * 0.5);
    window().Draw(sprite);
  }

  // Press enter.
  {
    auto sprite = smk::Sprite(texture_press_enter);
    sprite.SetBlendMode(smk::BlendMode::Add);
    sprite.SetPosition(
        320 + width * 0.5 - texture_press_enter.width - margin,
        240 + height * 0.5 - texture_press_enter.height - margin);
    float alpha_ = 0.5 + 0.5 * sin(window().time() * 8.f);
    sprite.SetColor({1.f, 1.f, 1.f, alpha_});
    sprite.SetBlendMode(smk::BlendMode::Add);
    window().Draw(sprite);
  }
}
