// Copyright 2019 Arthur Sonzogni. All rights reserved.
// Use of this source code is governed by the MIT license that can be found in
// the LICENSE file.

#include "Generator.hpp"

#include <algorithm>
#include <iostream>
#include <map>
#include <queue>
#include <random>
#include <set>
#include "plateau.h"

namespace {

struct position {
  int x, y;
  position(int X, int Y) : x(X), y(Y) {}
  position() : x(0), y(0) {}
};

struct PlateauEvaluation {
  PlateauEvaluation() = default;
  bool operator<(const PlateauEvaluation& b) const;

  std::unique_ptr<Plateau> plateau;
  int evaluation = 0;
  int time = 0;
};

class GeneratorImpl : public Generator {
 public:
  GeneratorImpl(int width, int height);
  ~GeneratorImpl() override = default;

 private:
  bool Next() override;
  std::unique_ptr<Plateau> Best() override;
  float Progress() override;

  void StartOptimize();
  void Optimize();
  void StartReduce();
  void Reduce();
  std::unique_ptr<Plateau> RandomPlateau();

  void Mutate(Plateau& plateau);
  void EraseUnconnexe(Plateau* plateau);

  std::default_random_engine random;
  int width;
  int height;

  std::vector<PlateauEvaluation> plateau_vector;
  int time = 0;
  int min_score = 0;

  int step = 0;
};

// int CountGround(const Plateau& plateau);
// int Evaluation(Plateau& p);
int EvaluationReduce(Plateau& p, int min_score);

#define nb_plateau 42
GeneratorImpl::GeneratorImpl(int width, int height)
    : random(std::random_device()()),
      width(width),
      height(height),
      plateau_vector(nb_plateau) {}

bool GeneratorImpl::Next() {
  int s = ++step;

  if ((s -= 1) == 0) {
    StartOptimize();
    return true;
  }

  if ((s -= 1000) < 0) {
    Optimize();
    return true;
  }

  if ((s -= 1) < 0) {
    StartReduce();
    return true;
  }

  if ((s -= 200) < 0) {
    Reduce();
    return true;
  }

  return false;
}

std::unique_ptr<Plateau> GeneratorImpl::RandomPlateau() {
  auto plateau = std::make_unique<Plateau>();
  plateau->simulation = true;
  plateau->LoadEmpty(width, height);

  // Build some ground.
  int med = std::uniform_int_distribution<int>(10, 90)(random);
  for (int x = 0; x < width; x++) {
    for (int y = 0; y < height; y++) {
      plateau->elements[x][y].type =
          std::uniform_int_distribution<int>(0, 100)(random) >= med
              ? Type::GROUND
              : Type::VOID;
    }
  }

  // Add the finish case.
  int finish_x = std::uniform_int_distribution<int>(0, width - 1)(random);
  int finish_y = std::uniform_int_distribution<int>(0, height - 1)(random);
  plateau->elements[finish_x][finish_y].type = Type::END;

  // Add the start block.
  int start_x = std::uniform_int_distribution<int>(0, width - 3)(random);
  int start_y = std::uniform_int_distribution<int>(0, height - 3)(random);
  start_x += (start_x == finish_x);
  start_y += (start_y == finish_y);
  plateau->block.x = start_x;
  plateau->block.y = start_y;
  plateau->elements[start_x][start_y].type = Type::GROUND;

  int direction = std::uniform_int_distribution<int>(1, 3)(random);
  switch (direction) {
    case 1:
      plateau->block.a = 2;
      plateau->block.b = 1;
      plateau->block.c = 1;
      plateau->elements[start_x][start_y + 1].type = Type::GROUND;
      break;
    case 2:
      plateau->block.a = 1;
      plateau->block.b = 2;
      plateau->block.c = 1;
      plateau->elements[start_x + 1][start_y].type = Type::GROUND;
      break;
    case 3:
      plateau->block.a = 1;
      plateau->block.b = 1;
      plateau->block.c = 2;
      break;
  }
  return plateau;
}

int CountGround(const Plateau& plateau) {
  int count = 0;
  for (auto& line : plateau.elements) {
    for (auto& c : line) {
      count += (c.type == Type::GROUND);
    }
  }
  return count;
}

int EvaluationReduce(Plateau& p, int min_score) {
  int value = Evaluation(p);
  if (value < min_score)
    return -1;

  return p.width * p.height - CountGround(p);
}

void GeneratorImpl::EraseUnconnexe(Plateau* plateau) {
  std::queue<position> todo;
  std::vector<std::vector<bool>> connexe(
      plateau->width, std::vector<bool>(plateau->height, false));
  position pos;
  pos.x = plateau->block.x;
  pos.y = plateau->block.y;
  todo.push(pos);
  while (!todo.empty()) {
    pos = todo.front();
    todo.pop();

    if (pos.x >= 0 && pos.y >= 0 && pos.x < plateau->width &&
        pos.y < plateau->height) {
      if (!connexe[pos.x][pos.y]) {
        if (plateau->elements[pos.x][pos.y].type == Type::GROUND) {
          connexe[pos.x][pos.y] = true;
          todo.push(position(pos.x - 1, pos.y));
          todo.push(position(pos.x + 1, pos.y));
          todo.push(position(pos.x, pos.y - 1));
          todo.push(position(pos.x, pos.y + 1));
        }
      }
    }
  }
  for (int x = 0; x < plateau->width; x++) {
    for (int y = 0; y < plateau->height; y++) {
      if (!connexe[x][y]) {
        if (plateau->elements[x][y].type == Type::GROUND) {
          plateau->elements[x][y].type = Type::VOID;
        }
      }
    }
  }
}

}  // namespace

std::unique_ptr<Plateau> GeneratorImpl::Best() {
  return std::move(plateau_vector.front().plateau);
}

float GeneratorImpl::Progress() {
  return step / 1200.0;
}

// clang-format off
bool operator<(const Block& a, const Block& b) {
  if (a.x < b.x) return true;
  if (a.x > b.x) return false;

  if (a.y < b.y) return true;
  if (a.y > b.y) return false;

  if (a.a < b.a) return true;
  if (a.a > b.a) return false;

  if (a.b < b.b) return true;
  if (a.b > b.b) return false;

  if (a.c < b.c) return true;
  if (a.c > b.c) return false;

  return false;
}
// clang-format on

bool PlateauEvaluation::operator<(const PlateauEvaluation& other) const {
  if (evaluation == other.evaluation)
    return time > other.time;
  else
    return evaluation > other.evaluation;
}

void GeneratorImpl::StartOptimize() {
  for (int i = 0; i < nb_plateau; i++) {
    plateau_vector[i].plateau = RandomPlateau();
    plateau_vector[i].evaluation = Evaluation(*(plateau_vector[i].plateau));
  }
}

void GeneratorImpl::Optimize() {
  float l = std::uniform_real_distribution<float>(0.f, 1.f)(random);
  l = l * l;
  int level = std::floor(l * nb_plateau);

  auto plateau = std::make_unique<Plateau>(*(plateau_vector[level].plateau));
  Mutate(*(plateau));
  plateau_vector.back().time = time++;
  plateau_vector.back().evaluation = Evaluation(*(plateau));
  plateau_vector.back().plateau = std::move(plateau);
  std::sort(plateau_vector.begin(), plateau_vector.end());
}

void GeneratorImpl::StartReduce() {
  min_score = plateau_vector.front().evaluation;
  for (auto& it : plateau_vector)
    it.evaluation = EvaluationReduce(*(it.plateau), min_score);
  std::sort(plateau_vector.begin(), plateau_vector.end());
}

void GeneratorImpl::Reduce() {
  float l = std::uniform_real_distribution<float>(0.f, 1.f)(random);
  int level = std::floor(l * nb_plateau);

  auto plateau = std::make_unique<Plateau>(*(plateau_vector[level].plateau));
  Mutate(*plateau);
  EraseUnconnexe(plateau.get());
  plateau_vector.back().time = time++;
  plateau_vector.back().evaluation = EvaluationReduce(*plateau, min_score);
  plateau_vector.back().plateau = std::move(plateau);
  std::sort(plateau_vector.begin(), plateau_vector.end());
}

void GeneratorImpl::Mutate(Plateau& plateau) {
  int n = std::uniform_int_distribution<int>(1, 4)(random);
  auto random_x = std::uniform_int_distribution<int>(0, plateau.width - 1);
  auto random_y = std::uniform_int_distribution<int>(0, plateau.height - 1);
  for (int i = 0; i < n; i++) {
    int x = random_x(random);
    int y = random_y(random);
    switch (plateau.elements[x][y].type) {
      case Type::VOID:
        plateau.elements[x][y].type = Type::GROUND;
        break;
      case Type::GROUND:
        if (plateau.block.x != x || plateau.block.y != y)
          plateau.elements[x][y].type = Type::VOID;
        break;
      default:
        break;
    }
  }
}

// static
std::unique_ptr<Generator> Generator::New(int width, int height) {
  return std::make_unique<GeneratorImpl>(width, height);
}

int Evaluation(Plateau& p) {
  p.simulation = true;
  std::set<Block> done;
  std::queue<Block> todo;
  std::queue<int> score;

  Block initial_block = p.block;

  done.insert(p.block);
  todo.push(p.block);
  score.push(0);

  // test que le départ est bon
  for (int i = 0; i < 3; ++i)
    p.Step();

  switch (p.block.animation) {
    case wait:
      break;
    case win:
      return 0;
    case moving:
    case fall:
      return -1;
  }

  while (!todo.empty()) {
    Block current_block = todo.front();
    todo.pop();

    int current_score = score.front();
    score.pop();

    if (current_block.animation == win) {
      p.block = initial_block;
      return current_score;
    }

    if (current_block.animation == fall)
      continue;

    // pour chaque possibilité de déplacement
    for (Direction dir : {left, up, right, down}) {
      // Restore the block and start moving.
      p.block = current_block;
      p.block.Move(dir);
      p.block.time = 1;
      p.Step();

      if (done.find(p.block) != done.end())
        continue;
      todo.push(p.block);
      done.insert(p.block);
      score.push(current_score + 1);
    }
  }

  p.block = initial_block;
  return -1;
}
