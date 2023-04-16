//
// Created by Алексей Крукович on 16.04.23.
//

#ifndef SDL_ENGINE_STATIC_ENGINE_HXX
#define SDL_ENGINE_STATIC_ENGINE_HXX
#include <string_view>
#include <string>

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
  virtual ~IEngine();
  virtual std::string initialize(std::string_view config) = 0;
  virtual void uninitialize() = 0;
  virtual bool readInput(Event& event) = 0;
};

IEngine* createEngine();
void destroyEngine(IEngine* e);

#endif  // SDL_ENGINE_STATIC_ENGINE_HXX
