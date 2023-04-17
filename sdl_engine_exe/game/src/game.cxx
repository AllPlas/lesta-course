#include <stdexcept>
#include <array>
#include <thread>
#include <iostream>
#include <engine.hxx>

using namespace std::literals;

class ConsoleGame : public IGame
{
private:
  uint32_t rotation_index = 0;
  const std::array<char, 4> rotations_chars{ '-', '/', '|', '\\' };

public:
  void initialize() override {}
  void onEvent(Event event) override {}

  void update() override {
    using namespace std;
    ++rotation_index;
    rotation_index %= rotations_chars.size();
    using namespace std::chrono;
    std::this_thread::sleep_for(milliseconds(40));
  }

  void render() const override {
    const char current_symbol = rotations_chars.at(rotation_index);
    std::cout << "\b" << current_symbol << std::flush;
  }
};

IGame* createGame(IEngine* engine) {
  if (engine == nullptr) throw std::runtime_error{ "Error : createGame : engine is nullptr"s };
  return new ConsoleGame{};
}

// void destroyGame(IGame* game) {
//   if (game == nullptr) throw std::runtime_error{ "Error : destroyGame : game is nullptr"s };
//   delete game;
// }