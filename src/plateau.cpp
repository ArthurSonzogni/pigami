#include "plateau.h"
#include <cmath>
#include <fstream>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <smk/Shape.hpp>
#include <smk/Text.hpp>
#include <smk/View.hpp>
#include "Resources.hpp"

static glm::vec4 floorPlane = {0.f, 0.f, 1.f, -0.001f};
static glm::vec4 light_position = {5.0f, 10.0f, 15.0f, 1.0f};

namespace {

smk::Transformable3D Cylinder(int iteration) {
  std::vector<smk::Vertex3D> v;
  auto up = glm::vec3(0.f, 0.f, 0.5f);
  auto down = glm::vec3(0.f, 0.f, -0.5f);
  auto pa1 = glm::vec3(0.5f, 0.0f, 0.5f);
  auto pb1 = glm::vec3(0.5f, 0.0f, -0.5f);
  auto normal_1 = glm::vec3(1.f, 0.f, 0.f);

  auto texture_position = [](const glm::vec3 v) {
    return glm::vec2(0.5f + v.x * 0.5f, 0.5f + v.y * 0.5f);
  };

  for (int i = 1; i <= iteration; ++i) {
    float a = (2.0 * M_PI * i) / iteration;
    auto pa2 = glm::vec3(0.5f * cos(a), 0.5f * sin(a), +0.5f);
    auto pb2 = glm::vec3(0.5f * cos(a), 0.5f * sin(a), -0.5f);
    auto normal_2 = glm::vec3(cos(a), sin(a), 0.f);

    // Up face.
    v.push_back({up, up, texture_position(up)});
    v.push_back({pa1, up, texture_position(pa1)});
    v.push_back({pa2, up, texture_position(pa2)});

    // Cylinder face.
    v.push_back({pa1, normal_1, texture_position(pa1)});
    v.push_back({pb1, normal_1, texture_position(pb1)});
    v.push_back({pb2, normal_2, texture_position(pb2)});
    v.push_back({pa1, normal_1, texture_position(pa1)});
    v.push_back({pb2, normal_2, texture_position(pb2)});
    v.push_back({pa2, normal_2, texture_position(pa2)});

    // Down face.
    v.push_back({down, down, texture_position(down)});
    v.push_back({pb2, down, texture_position(pb2)});
    v.push_back({pb1, down, texture_position(pb1)});

    pa1 = pa2;
    pb1 = pb2;
    normal_1 = normal_2;
  }

  smk::Transformable3D transformable;
  transformable.SetVertexArray(std::move(v));
  return transformable;
}

glm::mat4 ShadowMatrix(glm::vec4 groundplane, glm::vec4 lightpos) {
  float dot = glm::dot(groundplane, lightpos);
  return dot * glm::mat4(1.f) - glm::outerProduct(light_position, groundplane);
}

}  // namespace

void Plateau::InitGl() {
  glEnable(GL_CULL_FACE);
  glEnable(GL_DEPTH_TEST);
}

Plateau::Plateau()
    : nb_move(0),
      nb_move_min(0),
      show_nb_move(false),
      simulation(false),
      width(10),
      height(10),
      block(2, 1, 1, 2, 2),
      NeedDelete(false) {}

void Plateau::LoadEmpty(int Width, int Height) {
  nb_move = 0;
  nb_move_min = 0;
  show_nb_move = false;
  block.x = 0;
  block.y = 0;
  block.a = 1;
  block.b = 1;
  block.c = 1;
  block.animation = wait;
  block.time = 0;
  block.direction = left;
  block.falling_position = 0;
  width = Width;
  height = Height;

  Plateau_element vide_element;
  vide_element.type = Type::VOID;

  // delete previous grid && get a new one;
  elements = std::vector<std::vector<Plateau_element>>(
      width, std::vector<Plateau_element>(height, vide_element));
  NeedDelete = true;
}

void Plateau::Load(const std::string& filename) {
  nb_move = 0;
  nb_move_min = 0;
  show_nb_move = false;
  int x, y, i;
  std::ifstream file("data.txt", std::ios::in);
  file.open(filename, std::ios::in);
  if (!file) {
    // exception
  }

  // load block
  file >> block.x >> block.y >> block.a >> block.b >> block.c;

  block.animation = wait;
  block.time = 0;
  block.direction = left;
  block.falling_position = 0;

  // load dimentions
  file >> width >> height;

  Plateau_element vide_element;
  vide_element.type = Type::VOID;

  elements = std::vector<std::vector<Plateau_element>>(
      width, std::vector<Plateau_element>(height, vide_element));

  // load element of the grid
  for (y = 0; y < height; ++y) {
    for (x = 0; x < width; ++x) {
      Plateau_element& p = elements[x][y];

      int kk;
      file >> kk;
      p.type = (Type)kk;

      switch (p.type) {
        case Type::BUTTON: {
          file >> p.button.nbTarget;
          file >> p.button.minWeight;
          p.button.target = new ButtonSignal[p.button.nbTarget];
          for (i = 0; i < p.button.nbTarget; ++i) {
            int x, y, mode;
            file >> x >> y >> mode;
            p.button.target[i].x = x;
            p.button.target[i].y = y;
            p.button.target[i].mode = (ButtonMode)mode;
          }
          p.button.minWeight = std::max(1, std::min(3, p.button.minWeight));
        } break;

        case Type::RETRACTABLE: {
          int state;
          int direction;
          file >> state;
          file >> direction;
          p.retractable.state = state;
          p.retractable.direction = (Direction)direction;
          p.retractable.time = 0;
        } break;

        case Type::FRAGILE: {
          int maxWeight;
          file >> maxWeight;
          p.fragile.maxWeight = maxWeight;
          p.fragile.time = 0;
          p.fragile.state = true;
        } break;

        default:
          break;
      }
    }
  }
  camera_position = glm::vec3(block.x, block.y, 0.f);
  camera_speed = glm::vec3(0.f);
}

void Plateau::Save(const std::string& filename) {
  int x, y, i;

  std::ofstream file(filename, std::ios::out | std::ios::trunc);

  // save block
  file << block.x << std::endl
       << block.y << std::endl
       << block.a << std::endl
       << block.b << std::endl
       << block.c << std::endl;

  // save dimentions
  file << width << std::endl << height << std::endl;

  NeedDelete = true;

  // save element of the grid
  for (y = 0; y < height; ++y)
    for (x = 0; x < width; ++x) {
      Plateau_element& p = elements[x][y];

      // tweak to do fs<<p.type
      file << p.type << std::endl;

      switch (p.type) {
        case Type::BUTTON: {
          file << p.button.nbTarget << std::endl;
          file << p.button.minWeight << std::endl;
          for (i = 0; i < p.button.nbTarget; ++i) {
            file << p.button.target[i].x << std::endl;
            file << p.button.target[i].y << std::endl;
            file << p.button.target[i].mode << std::endl;
          }
        } break;

        case Type::RETRACTABLE: {
          file << p.retractable.state << std::endl;
          file << p.retractable.direction << std::endl;
        } break;

        case Type::FRAGILE: {
          file << p.fragile.maxWeight << std::endl;
        } break;

        default:
          break;
      }
    }
}

void Plateau::Step() {

  auto camera_target = glm::vec3(block.x, block.y, 0.f);
  auto diff = (camera_target - camera_position) * 0.003f;
  camera_speed += diff * 0.05f;
  camera_speed *= 0.97f;
  camera_position += camera_speed + diff;

  if (block.animation == moving) {
    block.time--;
    if (block.time == 0) {
      block.animation = wait;

      nb_move++;
      int x, y;
      for (x = 0; x < block.a; x++) {
        for (y = 0; y < block.b; y++) {
          ApplyWeight(block.x + x, block.y + y, block.c);
        }
      }
    }
  }

  if (block.animation == wait) {
    Direction direction;
    int position;
    if (EquilibriumTest(direction, position)) {
      block.animation = fall;
      block.direction = direction;
      block.falling_position = position;
      block.time = 0;
    } else if (TestFinish()) {
      if (!simulation) {
        PlaySoundInternal(sound_win);
      }
      block.animation = win;
      block.time = 0;
    }
  }

  if (block.animation == fall) {
    if (block.time == 10)
      PlaySoundInternal(sound_lose);
    block.time++;
  }

  if (block.animation == win) {
    block.time++;
  }

  int x, y;
  for (x = 0; x < width; ++x) {
    for (y = 0; y < height; ++y) {
      switch (elements[x][y].type) {
        case Type::FRAGILE:
          if (elements[x][y].fragile.state == false) {
            elements[x][y].fragile.time++;
          }
          break;

        default:
          break;
      }
    }
  }
}

void Plateau::Draw(smk::Screen* screen,
                   float screen_width,
                   float screen_height) {
  screen_ = screen;
  glm::mat4 projection =
      glm::perspective(70.f, float(screen_width / screen_height), 5.f, 30.f);

  glm::vec3 eye = camera_position + glm::vec3(-3.0, -10.f, 10.f);
  glm::vec3 up_direction = {0.f, 0.f, 1.f};
  auto view = glm::lookAt(eye, camera_position, up_direction);

  screen_->SetShaderProgram(screen_->shader_program_3d());
  glClear(GL_STENCIL_BUFFER_BIT);

  glEnable(GL_CULL_FACE);
  glDisable(GL_STENCIL_TEST);
  glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
  glStencilFunc(GL_ALWAYS, 0, 0xffffffff);
  glFrontFace(GL_CCW);

  screen_->SetView(projection);
  glm::mat4 view_untranslated = glm::scale(view, glm::vec3(20.f));
  for (int i = 0; i < 3; ++i)
    view_untranslated[3][i] = 0.f;
  DrawSkybox(view_untranslated);

  glClear(GL_STENCIL_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  screen_->shader_program_3d()->SetUniform("light_position",
                                           view * light_position);
  screen_->shader_program_3d()->SetUniform("ambient", 0.4f);
  screen_->shader_program_3d()->SetUniform("diffuse", 0.4f);
  screen_->shader_program_3d()->SetUniform("specular", 0.2f);

  static float finish_animation_transformed = 0;
  finish_animation_transformed++;

  int x, y;
  // On écrit dans le stencil buffer pour l'ombre du block
  glEnable(GL_DEPTH_TEST);
  // glDisable(GL_STENCIL_TEST);
  // glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
  // glStencilFunc(GL_ALWAYS, 1, 0xffffffff);

  // if (block.animation == win && block.time > 20) {
  // glTranslatef(0.0, 0.0, -(block.time - 19) * (block.time - 19) * 0.001);
  //}

  for (y = 0; y < height; ++y) {
    for (x = 0; x < width; ++x) {
      switch (elements[x][y].type) {
        case Type::GROUND:
          DrawGround(glm::translate(view, {x, y, 0.f}));
          break;

        case Type::RETRACTABLE: {
          DrawRetractable(elements[x][y].retractable,
                          glm::translate(view, {x, y, 0.f}));
        }; break;

        case Type::BUTTON: {
          DrawButton(glm::translate(view, {x, y, 0.f}),
                     elements[x][y].button.minWeight);
        }; break;

        case Type::FRAGILE: {
          DrawFragile(glm::translate(view, {x, y, 0.f}),
                      elements[x][y].fragile);
        }; break;

        case Type::END:
        case Type::VOID:
          break;
      }
    }
  }

  glm::mat4 shadow_mat = ShadowMatrix(floorPlane, light_position);
  // Shadow
  block.Draw(screen_, view, false, true);
  // Reflet
  block.Draw(screen_, view * shadow_mat, true, false);
  // Normal
  block.Draw(screen_, view, false, false);

  for (int d = 0; d < 2; ++d) {
    for (int y = 0; y < height; ++y) {
      for (int x = 0; x < width; ++x) {
        switch (elements[x][y].type) {
          case Type::END:
            DrawFinish(glm::translate(view, {x, y, 0.f}), d);
            break;
          case Type::GROUND:
          case Type::RETRACTABLE:
          case Type::BUTTON:
          case Type::FRAGILE:
          case Type::VOID:
            break;
        }
      }
    }
  }


  if (show_nb_move) {
    float reduced = std::min(screen_->width(), screen_->height());
    float reduced_x = screen_->width() / reduced;
    float reduced_y = screen_->height() / reduced;

    auto view = smk::View();
    view.SetCenter(reduced_x * 0.5f, reduced_y * 0.5f);
    view.SetSize(reduced_x, reduced_y);
    screen_->SetView(view);

    screen_->SetShaderProgram(screen_->shader_program_2d());
    glClear(GL_STENCIL_BUFFER_BIT);
    auto text = smk::Text(font_arial, std::to_string(nb_move) + "/" +
                                          std::to_string(nb_move_min));
    static float scale = reduced_x * 0.001f;
    text.SetScale(scale, scale);
    text.SetPosition(scale * 25.f, scale * 25.f);
    screen_->Draw(text);
  }
}

Block::Block(int _a, int _b, int _c, int _x, int _y)
    : x(_x), y(_y), a(_a), b(_b), c(_c) {}

Block::Block() : Block(1, 1, 1, 1, 1) {}

void Block::Move(Direction dir) {
  if (dir == none)
    return;

  if (animation != wait)
    return;

  direction = dir;
  Block temp = *this;

  animation = moving;
  time = rolling_step;
  switch (direction) {
    case left: {
      a = temp.c;
      b = temp.b;
      c = temp.a;
      x = temp.x - temp.c;
      y = temp.y;
    } break;

    case right: {
      a = temp.c;
      b = temp.b;
      c = temp.a;
      x = temp.x + temp.a;
      y = temp.y;
    } break;

    case down: {
      a = temp.a;
      b = temp.c;
      c = temp.b;
      x = temp.x;
      y = temp.y - temp.c;
    } break;

    case up: {
      a = temp.a;
      b = temp.c;
      c = temp.b;
      x = temp.x;
      y = temp.y + temp.b;
    } break;

    case none:
      break;
  }
}

void Plateau::DrawGround(glm::mat4 view) {
  glEnable(GL_STENCIL_TEST);
  {
    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
    glStencilFunc(GL_ALWAYS, 42, 0xffffffff);
    auto plane = smk::Shape::Plane();
    plane.SetTransformation(glm::translate(view, {+0.5, +0.5, +0.f}));
    plane.SetTexture(texture_dalle);
    screen_->Draw(plane);
  }

  {
    glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
    glStencilFunc(GL_ALWAYS, 0, 0xffffffff);
    auto cube = smk::Shape::Cube();
    view = glm::translate(view, {0.f, 0.f, -0.25f});
    view = glm::scale(view, {1.f, 1.f, 0.25f});
    view = glm::translate(view, {+0.5, +0.5, +0.5f});
    cube.SetTransformation(view);
    cube.SetTexture(texture_dalle);
    screen_->Draw(cube);
  }
}

void Plateau::DrawFragile(glm::mat4 view, Fragile fragile) {
  float momentum = fragile.time * 4.8f / animation_step;
  float gravity = momentum * momentum;

  view = glm::translate(view, {0.f, 0.f, -gravity - 0.5f * momentum});
  static const smk::Texture* textures[] = {
      &texture_fragile1,
      &texture_fragile2,
      &texture_fragile3,
  };

  glEnable(GL_STENCIL_TEST);
  {
    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
    glStencilFunc(GL_ALWAYS, 42, 0xffffffff);
    auto plane = smk::Shape::Plane();
    plane.SetTransformation(glm::translate(view, {+0.5, +0.5, +0.f}));
    plane.SetTexture(*textures[fragile.maxWeight - 1]);
    screen_->Draw(plane);
  }

  {
    glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
    glStencilFunc(GL_ALWAYS, 0, 0xffffffff);
    auto cube = smk::Shape::Cube();
    view = glm::translate(view, {0.f, 0.f, -0.25f});
    view = glm::scale(view, {1.f, 1.f, 0.25f});
    view = glm::translate(view, {+0.5, +0.5, +0.5f});
    cube.SetTransformation(view);
    cube.SetTexture(*textures[fragile.maxWeight - 1]);
    screen_->Draw(cube);
  }
}

void Plateau::DrawButton(glm::mat4 view, int minWeight) {
  DrawGround(view);

  glEnable(GL_STENCIL_TEST);
  glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
  glStencilFunc(GL_ALWAYS, 0, 0xffffffff);

  view = glm::translate(view, {0.5f, 0.5f, 0.f});
  view = glm::scale(view, {0.5, 0.5, 0.25});
  view = glm::translate(view, {0.f, 0.f, 0.5f});

  auto cylinder = Cylinder(10);
  cylinder.SetTransformation(view);
  static const smk::Texture* textures[] = {
      &texture_bouton1,
      &texture_bouton2,
      &texture_bouton3,
  };
  cylinder.SetTexture(*textures[minWeight - 1]);
  screen_->Draw(cylinder);
}

void Plateau::DrawFinish(glm::mat4 view, int depth) {
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_CULL_FACE);
  float margin = 0.02f;
  if (depth == 0) {
    glDisable(GL_STENCIL_TEST);
    glm::mat4 v = view;
    auto cube = smk::Shape::Cube();
    v = glm::translate(v, {0.f, 0.f, -0.25f});
    v = glm::scale(v, {1.f, 1.f, 0.25f});
    v = glm::translate(v, {+0.5, +0.5, +0.5f});
    cube.SetTransformation(v);
    cube.SetColor({0.8, 0.7, 0.3, 1.f});
    screen_->Draw(cube);
  }

  if (depth == 1) {
    glEnable(GL_STENCIL_TEST);
    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
    glStencilFunc(GL_NOTEQUAL, 80, 0xffffffff);
    glm::mat4 v = view;
    v = glm::scale(v, {1.f + margin, 1.f + margin, 10.f});
    v = glm::translate(v, {0.5 - margin * 0.5f, 0.5 - margin * 0.5f, 0.5});
    auto shape = smk::Shape::Cube();
    shape.SetColor({0.8, 0.7, 0.3, 1.f});
    shape.SetTransformation(v);
    shape.SetBlendMode(smk::BlendMode::Add);
    screen_->Draw(shape);
  }
}

void Plateau::DrawRetractable(Retractable& r, glm::mat4 view) {
  float angle = r.state ? M_PI * float(r.time) / rolling_step
                        : M_PI * float(rolling_step - r.time) / rolling_step;

  if (r.time > 0)
    r.time--;

  switch (r.direction) {
    case left: {
      view = glm::translate(view, {0.f, 0.f, -0.25f});
      view = glm::rotate(view, angle, {0.0f, 1.0, 0.0f});
      view = glm::translate(view, {0.f, 0.f, +0.25f});
    }; break;

    case right: {
      view = glm::translate(view, {1.f, 0.f, -0.25f});
      view = glm::rotate(view, angle, {0.0f, -1.0, 0.0f});
      view = glm::translate(view, {-1.f, 0.f, +0.25f});
    }; break;

    case down: {
      view = glm::translate(view, {0.f, 0.f, -0.25f});
      view = glm::rotate(view, angle, {-1.0f, 0.f, 0.0f});
      view = glm::translate(view, {0.f, 0.f, +0.25f});
    }; break;

    case up: {
      view = glm::translate(view, {0.f, 1.f, -0.25f});
      view = glm::rotate(view, angle, {1.0f, 0.f, 0.0f});
      view = glm::translate(view, {0.f, -1.f, +0.25f});
    }; break;

    case none:
      break;
  }

  view = glm::translate(view, {0.f, 0.f, -0.25f});
  view = glm::scale(view, {1.f, 1.f, 0.25f});
  view = glm::translate(view, {0.5f, 0.5f, 0.5f});

  glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
  glStencilFunc(GL_ALWAYS, 0, 0xffffffff);

  auto cube = smk::Shape::Cube();
  cube.SetTransformation(view);
  cube.SetTexture(texture_retractable);
  screen_->Draw(cube);
}

void Plateau::ActiveExtScreen(int max_move) {
  nb_move_min = max_move;
  show_nb_move = true;
}

void Block::Draw(smk::Screen* screen,
                 glm::mat4 view,
                 bool shadow,
                 bool reflet) {
  glEnable(GL_CULL_FACE);
  if (reflet) {
    glFrontFace(GL_CW);
    view = glm::scale(view, {1.f, 1.f, -1.f});
  } else {
    glFrontFace(GL_CCW);
  }

  // if (reflet) {
  // glFrontFace(GL_CCW);
  // glCullFace(GL_FRONT);
  // glScalef(1.0, 1.0, -1.0);
  // glEnable(GL_LIGHTING);
  //}

  float rotation = time * M_PI * 0.5f / rolling_step;
  view = glm::translate(view, {x, y, 0.f});
  if (animation == moving) {
    switch (direction) {
      case left:
        view = glm::translate(view, {+a, 0.f, 0.f});
        view = glm::rotate(view, rotation, {0.0, +1.0, 0.0});
        view = glm::translate(view, {-a, 0.f, 0.f});
        break;

      case right:
        view = glm::rotate(view, rotation, {0.0, -1.0, 0.0});
        break;

      case down:
        view = glm::translate(view, {0.f, +b, 0.f});
        view = glm::rotate(view, rotation, {-1.0, 0.0, 0.0});
        view = glm::translate(view, {0.f, -b, 0.f});
        break;

      case up:
        view = glm::rotate(view, rotation, {+1.0, 0.0, 0.0});
        break;

      case none:
        break;
    }
  } else if (animation == wait) {
    // Nothing.
  } else if (animation == fall) {
    // Gravity
    float momentum = time * 4.8f / animation_step;
    float gravity = momentum * momentum;
    switch (direction) {
      case left: {
        view = glm::translate(
            view, {-0.5 * momentum, 0.f, -gravity + 0.5f * momentum});
        view = glm::translate(view, {falling_position, 0.f, 0.f});
        view = glm::rotate(view, rotation, {0.0, -1.0, 0.0});
        view = glm::translate(view, {-falling_position, 0.f, 0.f});
      }; break;

      case right: {
        view = glm::translate(
            view, {+0.5 * momentum, 0.f, -gravity + 0.5f * momentum});
        view = glm::translate(view, {falling_position + 1.f, 0.f, 0.f});
        view = glm::rotate(view, rotation, {0.0, +1.0, 0.0});
        view = glm::translate(view, {-falling_position - 1.f, 0.f, 0.f});
      }; break;

      case down: {
        view = glm::translate(
            view, {0.f, -0.5 * momentum, -gravity + 0.5f * momentum});
        view = glm::translate(view, {0.f, falling_position, 0.f});
        view = glm::rotate(view, rotation, {+1.0, 0.0, 0.0});
        view = glm::translate(view, {0.f, -falling_position, 0.f});
      }; break;

      case up: {
        view = glm::translate(
            view, {0.f, +0.5 * momentum, -gravity + 0.5f * momentum});
        view = glm::translate(view, {0.f, falling_position + 1.f, 0.f});
        view = glm::rotate(view, rotation, {-1.0, 0.f, 0.0});
        view = glm::translate(view, {0.f, -falling_position - 1.f, 0.f});
      }; break;

      case none: {
        view = glm::translate(view, {0.f, 0.f, -gravity - 0.5f * momentum});
      }; break;
    }
  }

  if (animation == win) {
    float z = time * 0.01f;
    z = z * z * 10.f + z;
    view = glm::translate(view, {0.f, 0.f, z});
  }

  // if (!shadow && !reflet) {
  // glStencilFunc(GL_ALWAYS, 0, 0xffffffff);
  // glStencilOp(GL_KEEP, GL_REPLACE, GL_REPLACE);
  // glBindTexture(GL_TEXTURE_2D,  texture_block);
  //} else if (shadow) {
  // glEnable(GL_DEPTH_TEST);
  // glStencilFunc(GL_LEQUAL, 1, 0xffffffff);
  // glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
  // glEnable(GL_STENCIL_TEST);
  // glEnable(GL_BLEND);
  // glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  // glDisable(GL_LIGHTING);
  // glDisable(GL_TEXTURE_2D);
  // float alpha =
  //(animation == fall) ? ((time < animation_step) ? 0.5 - float(time) / 30.0 :
  // 0) : (0.;

  // glColor4f(0.0, 0.0, 0.0, alpha);
  //} else if (reflet) {
  // glDisable(GL_DEPTH_TEST);
  // glStencilFunc(GL_EQUAL, 1, 0xffffffff);
  // glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
  // glEnable(GL_STENCIL_TEST);
  // glEnable(GL_BLEND);
  // glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  // float alpha =
  //(animation == fall) ? ((time < animation_step) ? 0.25 - float(time) / 60.0 :
  // 0; (0.25);

  // glColor4f(1.0, 1.0, 1.0, alpha);
  // glFrontFace(GL_CCW);
  //}

  // if (shadow) {
  // glDisable(GL_STENCIL_TEST);
  // glEnable(GL_LIGHTING);
  // glEnable(GL_TEXTURE_2D);
  // glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
  //} else if (reflet) {
  // glEnable(GL_DEPTH_TEST);
  // glEnable(GL_CULL_FACE);
  // glDisable(GL_STENCIL_TEST);
  // glFrontFace(GL_CW);
  // glCullFace(GL_FRONT);
  //}
  auto cube = smk::Shape::Cube();

  if (shadow) {
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_STENCIL_TEST);
    glStencilOp(GL_KEEP, GL_KEEP, GL_ZERO);
    glStencilFunc(GL_EQUAL, 42, 0xffffffff);
    cube.SetColor({0.f, 0.f, 0.f, 0.3f});
  } else if (reflet) {
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_STENCIL_TEST);
    glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
    glStencilFunc(GL_EQUAL, 42, 0xffffffff);
    cube.SetColor({1.f, 1.f, 1.f, 0.3f});
  } else {
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_STENCIL_TEST);
    glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);
    glStencilFunc(GL_ALWAYS, 0, 0xffffffff);
  }

  view = glm::scale(view, {a, b, c});
  view = glm::translate(view, {0.5f, 0.5f, 0.5f});
  cube.SetTransformation(view);
  cube.SetTexture(texture_block);
  screen->Draw(cube);
}

bool Plateau::GroundOn(int x, int y) {
  if (x < 0 || y < 0 || x >= width || y >= height)
    return false;
  else {
    Plateau_element& e = elements[x][y];
    switch (e.type) {
      case Type::VOID:
        return false;
        break;
      case Type::GROUND:
      case Type::BUTTON:
      case Type::END:
        return true;
        break;
      case Type::RETRACTABLE:
        return e.retractable.state;
        break;
      case Type::FRAGILE:
        return e.fragile.state;
        break;
      default:
        return false;
        break;
    }
  }
}

bool Plateau::EquilibriumTest(Direction& direction, int& position) {
  int fall_left = block.a;
  int fall_right = -1;
  int fall_down = block.b;
  int fall_up = -1;
  bool fall_statically = true;

  for (int x = 0; x < block.a; ++x) {
    for (int y = 0; y < block.b; ++y) {
      if (GroundOn(block.x + x, block.y + y)) {
        fall_left = std::min(fall_left, x);
        fall_right = std::max(fall_right, x);
        fall_down = std::min(fall_down, y);
        fall_up = std::max(fall_up, y);
        fall_statically = false;
      }
    }
  }

  if (fall_statically) {
    direction = none;
    position = 0;
    return true;
  }

  if (2 * fall_left >= block.a) {
    direction = left;
    position = fall_left;
    return true;
  }

  if (2 * fall_down >= block.b) {
    direction = down;
    position = fall_down;
    return true;
  }

  if (2 * fall_right + 1 < block.a) {
    direction = right;
    position = fall_right;
    return true;
  }

  if (2 * fall_up + 1 < block.b) {
    direction = up;
    position = fall_up;
    return true;
  }

  return false;
}

bool Plateau::TestFinish() {
  for (int x = 0; x < block.a; ++x) {
    for (int y = 0; y < block.b; ++y) {
      if (block.x + x >= 0 && block.x + x < width && block.y + y >= 0 &&
          block.y + y < height)
        if (elements[block.x + x][block.y + y].type != Type::END)
          return false;
    }
  }

  return true;
}

void Plateau::ApplyWeight(int x, int y, int weight) {
  if (x < 0 || y < 0 || x >= width || y >= height)
    return;
  Plateau_element& e = elements[x][y];
  switch (e.type) {
    case Type::BUTTON: {
      if (e.button.minWeight <= weight) {
        for (int i = 0; i < e.button.nbTarget; i++) {
          int x = e.button.target[i].x;
          int y = e.button.target[i].y;
          ButtonMode mode = e.button.target[i].mode;
          switch (mode) {
            case Off:
              if (elements[x][y].retractable.state == true) {
                // if (!simulation)
                PlaySoundInternal(sound_fermeture);
                elements[x][y].retractable.state = false;
                elements[x][y].retractable.time = rolling_step;
              }
              break;
            case On:
              if (elements[x][y].retractable.state == false) {
                PlaySoundInternal(sound_ouverture);
                elements[x][y].retractable.state = true;
                elements[x][y].retractable.time = rolling_step;
              }
              break;
            case Invert: {
              if (elements[x][y].retractable.state)
                PlaySoundInternal(sound_fermeture);
              else
                PlaySoundInternal(sound_ouverture);

              elements[x][y].retractable.state =
                  !elements[x][y].retractable.state;
              elements[x][y].retractable.time = rolling_step;
            } break;
          }
        }
      }
    } break;

    case Type::FRAGILE: {
      if (e.fragile.maxWeight < weight) {
        e.fragile.state = false;
      }
    }; break;

    default:
      break;
  }
}

bool Plateau::IsLevelWinned() {
  return (block.animation == win && block.time > 200);
}

bool Plateau::IsLevelLoosed() {
  return (block.animation == fall && block.time > 200);
}

void Plateau::copie(Plateau& p) {
  LoadEmpty(p.width, p.height);
  block = p.block;
  elements = p.elements;
}

void Plateau::DrawSkybox(glm::mat4 view) {
  screen_->shader_program_3d()->SetUniform("light_position",
                                           {0.f, 0.f, 0.f, 1.f});
  screen_->shader_program_3d()->SetUniform("ambient", 1.f);
  screen_->shader_program_3d()->SetUniform("diffuse", 0.f);
  screen_->shader_program_3d()->SetUniform("specular", 0.f);
  glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);
  glStencilFunc(GL_ALWAYS, 42, 0xffffffff);

  glDisable(GL_CULL_FACE);
  auto face = smk::Shape::Plane();
  {
    auto v = glm::rotate(view, float(M_PI), {0.f, 0.f, 1.f});
    face.SetTransformation(glm::translate(v, {0.f, 0.f, 0.5f}));
    face.SetTexture(texture_skybox_top);
    screen_->Draw(face);
  }

  {
    auto v = glm::rotate(view, float(M_PI), {1.f, 0.f, 0.f});
    face.SetTransformation(glm::translate(v, {0.f, 0.f, +0.5f}));
    face.SetTexture(texture_skybox_bottom);
    screen_->Draw(face);
  }

  {
    auto v = glm::rotate(view, float(M_PI * 0.5f), {-1.f, 0.f, 0.f});
    face.SetTransformation(glm::translate(v, {0.f, 0.f, 0.5f}));
    face.SetTexture(texture_skybox_front);
    screen_->Draw(face);
  }

  {
    auto v = glm::rotate(view, float(M_PI * 0.5f), {0.f, 0.f, 1.f});
    v = glm::rotate(v, float(M_PI * 0.5f), {-1.f, 0.f, 0.f});
    face.SetTransformation(glm::translate(v, {0.f, 0.f, 0.5f}));
    face.SetTexture(texture_skybox_left);
    screen_->Draw(face);
  }

  {
    auto v = glm::rotate(view, float(M_PI), {0.f, 0.f, 1.f});
    v = glm::rotate(v, float(M_PI * 0.5f), {-1.f, 0.f, 0.f});
    face.SetTransformation(glm::translate(v, {0.f, 0.f, 0.5f}));
    face.SetTexture(texture_skybox_back);
    screen_->Draw(face);
  }

  {
    auto v = glm::rotate(view, float(M_PI * 0.5f), {0.f, 0.f, -1.f});
    v = glm::rotate(v, float(M_PI * 0.5f), {-1.f, 0.f, 0.f});
    face.SetTransformation(glm::translate(v, {0.f, 0.f, 0.5f}));
    face.SetTexture(texture_skybox_right);
    screen_->Draw(face);
  }
}

void Plateau::PlaySoundInternal(const smk::SoundBuffer& snd) {
  if (!simulation)
    PlaySound(snd);
}
