#ifndef PLATEAU_74VOZDJZ
#define PLATEAU_74VOZDJZ

#include <glm/gtc/matrix_transform.hpp>
#include <smk/OpenGL.hpp>
#include <smk/Screen.hpp>
#include <string>

constexpr int animation_step = 120;
constexpr int rolling_step = animation_step / 4;

namespace smk {
class SoundBuffer;
}

enum Direction {
  left = 0,
  up = 1,
  right = 2,
  down = 3,
  none = 4,
};

enum ButtonMode {
  Off = 0,
  On = 1,
  Invert = 2,
};

struct ButtonSignal {
  int x;
  int y;
  ButtonMode mode;
};

struct Button {
  int nbTarget;
  int minWeight;
  ButtonSignal* target;
};

struct Retractable {
  bool state;
  int time;
  Direction direction;
};

struct Fragile {
  int maxWeight;
  int time;
  bool state;
};

enum Type {
  VOID = 0,
  GROUND = 1,
  BUTTON = 2,
  END = 3,
  RETRACTABLE = 4,
  FRAGILE = 5,
};

struct Plateau_element {
  Type type;
  union {
    Button button;
    Retractable retractable;
    Fragile fragile;
  };
};

enum Animation {
  wait = 0,
  moving = 1,
  fall = 2,
  win = 3,
};

class Block {
 public:
  Block(int aaa, int bbb, int ccc, int xxx, int yyy);
  Block();

  // Position and dimension.
  int x, y;
  int a, b, c;

  // Animation data.
  int time = 0;
  Animation animation = wait;
  Direction direction = none;
  int falling_position = 0;

  void Move(Direction direction);
  void Draw(smk::Screen* screen, glm::mat4 view, bool shadow, bool reflet);
};

class Plateau {
 public:
  int nb_move;
  int nb_move_min;
  bool show_nb_move;
  bool simulation;
  int width;
  int height;

  Block block;

  bool NeedDelete;
  void PerformDelete();

  std::vector<std::vector<Plateau_element>> elements;

  Plateau();
  void LoadEmpty(int Width, int Height);
  void Load(const std::string& filename);
  void Save(const std::string& filename);
  void Step();
  void Draw(smk::Screen* screenn, float screen_width, float screen_height);
  void InitGl();

  bool GroundOn(int x,
                int y);  // test whether they are ground upon the position x,y;
  bool EquilibriumTest(Direction& direction, int& position);
  bool TestFinish();
  void ApplyWeight(int x, int y, int weight);
  void ActiveExtScreen(int max_move);
  bool IsLevelWinned();
  bool IsLevelLoosed();

  void copie(Plateau& p);

  smk::Screen* screen_ = nullptr;
  void DrawGround(glm::mat4 view);
  void DrawRetractable(Retractable& r, glm::mat4 view);
  void DrawButton(glm::mat4 view, int minWeight);
  void DrawSkybox(glm::mat4 view);
  void DrawFragile(glm::mat4 view, Fragile fragile);
  void DrawFinish(glm::mat4 view, int depth);

  void PlaySoundInternal(const smk::SoundBuffer& snd);

  glm::vec3 camera_position;
  glm::vec3 camera_speed;
};

#endif /* end of include guard: PLATEAU_74VOZDJZ */
