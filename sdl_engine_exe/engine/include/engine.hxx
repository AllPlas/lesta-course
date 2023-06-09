//
// Created by Алексей Крукович on 17.04.23.
//

#ifndef SDL_ENGINE_EXE_ENGINE_HXX
#define SDL_ENGINE_EXE_ENGINE_HXX

#include <iosfwd>
#include <string_view>
#include <string>
#include <memory>
#include <functional>

enum class Event
{
  up_pressed,
  up_released,
  down_pressed,
  down_released,
  left_pressed,
  left_released,
  right_pressed,
  right_released,
  space_pressed,
  space_released,
  return_pressed,
  return_released,
  lctrl_pressed,
  lctrl_released,
  turn_off,

  max_events,
};

std::ostream& operator<<(std::ostream& out, Event event);

class IEngine
{
public:
  virtual ~IEngine() = default;
  virtual std::string initialize([[maybe_unused]] std::string_view config) = 0;
  virtual void uninitialize() = 0;
  virtual bool readInput(Event& event) = 0;
};

std::unique_ptr<IEngine, std::function<void(IEngine*)>> createEngine();
void destroyEngine(IEngine* e);

class IGame
{
public:
  virtual ~IGame() = default;
  virtual void initialize() = 0;
  virtual void onEvent(Event event) = 0;
  virtual void update() = 0;
  virtual void render() const = 0;
};

extern "C" IGame* createGame(IEngine* engine);
extern "C" void destroyGame(IGame* game);

#endif  // SDL_ENGINE_EXE_ENGINE_HXX
