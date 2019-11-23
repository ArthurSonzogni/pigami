#ifndef GENERATOR_HPP
#define GENERATOR_HPP

#include <memory>

class Plateau;

int Evaluation(Plateau& p);

class Generator {
 public:
  static std::unique_ptr<Generator> New(int width, int height);
  virtual ~Generator() = default;

  virtual bool Next() = 0;
  virtual std::unique_ptr<Plateau> Best() = 0;
  virtual float Progress() = 0;
};

#endif /* end of include guard: GENERATOR_HPP */
