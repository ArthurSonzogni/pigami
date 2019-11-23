#include "activity/GenerateScreen.hpp"
#include <cmath>
#include <fstream>
#include <iostream>
#include <smk/BlendMode.hpp>
#include <smk/Color.hpp>
#include <smk/Screen.hpp>
#include <smk/Shape.hpp>
#include <smk/Sprite.hpp>
#include <smk/Text.hpp>
#include "Generator.hpp"
#include "Resources.hpp"
#include "plateau.h"

std::string minimot[3][20] = {
    {
        "In",  "De",   "Me",   "Contra", "Non-",  "A",      "anti",
        "Pre", "Post", "Ex",   "Inter",  "Trans", "Circon", "Epi",
        "Dia", "Para", "Péri", "Inter",  "Tri",   "Bi",
    },
    {
        "soleil", "soupl", "enfant", "vert",  "chat", "meubl",   "vend",
        "franc",  "real",  "trav",   "fonct", "bij",  "gargar",  "imprim",
        "sal",    "mer",   "clic",   "sour",  "pain", "retrait",
    },
    {
        "ette",  "on",   "",     "able", "ible", "uble", "logie",
        "ation", "ard",  "atre", "isme", "esse", "age",  "eur",
        "euse",  "onde", "ise",  "ere",  "",     "ique",
    },
};

void GenerateScreen::OnEnter() {
  generator = Generator::New(size_x, size_y);
}

void GenerateScreen::Animate() {
  background_activity_->Animate();
  background_alpha += (0.9 - background_alpha) * 0.01;
  background_alpha =
      std::min(background_alpha, 10.f * (1.f - generator->Progress()));
}

void GenerateScreen::Step() {
  Animate();
}

void GenerateScreen::Draw() {
  while (glfwGetTime() - screen().time() < 1.0 / 60.0 && generator->Next()) {
    // nothing
  }

  if (!generator->Next()) {
    Save();
    PlaySound(sound_menu_select);
    on_done();
  }

  background_activity_->Draw();

  {
    auto view = smk::View();
    view.SetCenter(screen().width() * 0.5, screen().height() * 0.5);
    view.SetSize(screen().width(), screen().height());
    screen().SetView(view);
  }

  {
    auto square = smk::Shape::Square();
    square.SetColor({0.f, 0.f, 0.f, background_alpha});
    square.SetScale(screen().width(), screen().height());
    screen().Draw(square);
  }

  float margin_x = screen().width() * 0.05;
  float margin_y = screen().height() * 0.05;
  {
    auto square = smk::Shape::Square();
    square.SetColor(smk::Color::White);
    square.SetScale((screen().width() - 2 * margin_x) * generator->Progress(),
                    2 * margin_y);
    square.SetPosition(margin_x + margin_y, screen().height() - 3 * margin_y);
    screen().Draw(square);

    auto circle = smk::Shape::Circle(margin_y);
    circle.SetColor(smk::Color::White);
    circle.SetPosition(margin_x + margin_y, screen().height() - 2 * margin_y);
    screen().Draw(circle);

    circle.SetPosition(
        margin_x + margin_y +
            (screen().width() - 2 * margin_x) * generator->Progress(),
        screen().height() - 2 * margin_y);
    screen().Draw(circle);
  }

  {
    auto text = smk::Text(
        font_arial, std::to_string(int(generator->Progress() * 100)) + "%");
    text.SetColor(smk::Color::White);
    text.SetBlendMode(smk::BlendMode::Invert);
    text.SetColor(smk::Color::Black);
    text.SetScale(margin_y / 55.f, margin_y / 55.f);
    glm::vec2 position = {margin_x + margin_y,
                          screen().height() - 3 * margin_y};
    for (float angle = 0.f; angle < 2 * M_PI; angle += 0.4) {
      glm::vec2 dir = {cos(angle), sin(angle)};
      text.SetPosition(position + margin_y / 20.f * dir);
      screen().Draw(text);
    }
    text.SetPosition(position);
    text.SetColor(smk::Color::White);
    screen().Draw(text);
  }
}

void GenerateScreen::Save() {
  std::vector<std::string> level_list;
  std::vector<std::string> level_score;
  {
    std::ifstream file(SavePath() + "/generated_level/list");
    std::string level;
    std::string score;
    while (std::getline(file, level) && std::getline(file, score)) {
      level_list.push_back(level);
      level_score.push_back(score);
    }
  }
  std::string level_name;
  level_name += minimot[0][level_list.size()];
  level_name += minimot[1][level_list.size()];
  level_name += minimot[2][level_list.size()];
  level_list.push_back(level_name);
  level_score.push_back("-1");
  generator->Best()->Save(SavePath() + "/generated_level/" + level_name);
  {
    std::ofstream file(SavePath() + "/generated_level/list");
    for (int i = 0; i < (int)level_list.size(); ++i) {
      file << level_list[i] << std::endl;
      file << level_score[i] << std::endl;
    }
  }
  SyncFilesystem();
}