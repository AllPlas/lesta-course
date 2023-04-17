//
// Created by Алексей Крукович on 16.04.23.
//
#include "engine.hxx"

#include <unordered_map>
#include <stdexcept>
#include <optional>
#include <functional>
#include <SDL3/SDL.h>

using namespace std::literals;

static const std::unordered_map<Event, std::string_view> s_eventToStringView{
  { Event::up_pressed, "up_pressed" },
  { Event::up_released, "up_released" },
  { Event::down_pressed, "down_pressed" },
  { Event::down_released, "down_released" },
  { Event::left_pressed, "left_pressed" },
  { Event::left_released, "left_released" },
  { Event::right_pressed, "right_pressed" },
  { Event::right_released, "right_released" },
  { Event::space_pressed, "space_pressed" },
  { Event::space_released, "space_released" },
  { Event::return_pressed, "return_pressed" },
  { Event::return_released, "return_released" },
  { Event::lctrl_pressed, "lctrl_pressed" },
  { Event::lctrl_released, "lctrl_released" },
  { Event::turn_off, "turn_off" }
};

std::ostream& operator<<(std::ostream& out, Event event) {
  out << s_eventToStringView.at(event);
  return out;
}

struct EventBind
{
  Event eventPressed{};
  Event eventReleased{};
};

static std::optional<Event> checkKeyboardInput(SDL_Event& event) {
  static const std::unordered_map<SDL_Keycode, EventBind> keymap{
    { SDLK_w, EventBind{ Event::up_pressed, Event::up_released } },
    { SDLK_s, EventBind{ Event::down_pressed, Event::down_released } },
    { SDLK_a, EventBind{ Event::left_pressed, Event::left_released } },
    { SDLK_d, EventBind{ Event::right_pressed, Event::right_released } },
    { SDLK_SPACE, EventBind{ Event::space_pressed, Event::space_released } },
    { SDLK_RETURN, EventBind{ Event::return_pressed, Event::return_released } },
    { SDLK_LCTRL, EventBind{ Event::lctrl_pressed, Event::lctrl_released } }
  };

  if (auto found{ keymap.find(event.key.keysym.sym) }; found != keymap.end()) {
    if (event.type == SDL_EVENT_KEY_DOWN)
      return found->second.eventPressed;
    else if (event.type == SDL_EVENT_KEY_UP)
      return found->second.eventReleased;
  }

  return std::nullopt;
}

IEngine::~IEngine() = default;

class EngineImpl final : public IEngine
{
private:
  SDL_Window* m_window{};

public:
  EngineImpl() = default;

  std::string initialize(std::string_view config) override {
    initSDL();
    m_window = createWindow();
    return "";
  }

  void uninitialize() override {
    SDL_DestroyWindow(m_window);
    SDL_Quit();
  }

  bool readInput(Event& event) override {
    SDL_Event sdlEvent;
    if (SDL_PollEvent(&sdlEvent)) {
      if (sdlEvent.type == SDL_EVENT_QUIT) {
        event = Event::turn_off;
        return true;
      }

      if (sdlEvent.type == SDL_EVENT_KEY_DOWN || sdlEvent.type == SDL_EVENT_KEY_UP) {
        if (auto e{ checkKeyboardInput(sdlEvent) }) {
          event = *e;
          return true;
        }
      }
    }

    return false;
  }

private:
  static void initSDL() {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) != 0)
      throw std::runtime_error{ "Error : failed call SDL_Init: "s + SDL_GetError() };
  }

  static SDL_Window* createWindow() {
    if (auto window{ SDL_CreateWindow("SDL3 infinity loop", 640, 480, SDL_WINDOW_OPENGL) };
        window != nullptr)
      return window;

    SDL_Quit();
    throw std::runtime_error{ "Error : failed call SDL_CreateWindow: "s + SDL_GetError() };
  }

  [[maybe_unused]] static SDL_Renderer* createRenderer(SDL_Window* window) {
    if (auto renderer{ SDL_CreateRenderer(window, "opengl", SDL_RENDERER_ACCELERATED) };
        renderer != nullptr)
      return renderer;

    SDL_DestroyWindow(window);
    SDL_Quit();
    throw std::runtime_error{ "Error : failed call SDL_CreateRenderer: "s + SDL_GetError() };
  }
};

static bool s_alreadyExist{ false };

// IEngine* createEngine() {
//   if (s_alreadyExist) throw std::runtime_error{ "Error : engine already exist"s };
//   s_alreadyExist = true;
//   return new EngineImpl{};
// }

std::unique_ptr<IEngine, std::function<void(IEngine*)>> createEngine() {
  if (s_alreadyExist) throw std::runtime_error{ "Error : engine already exist"s };
  s_alreadyExist = true;
  return { new EngineImpl{}, destroyEngine };
}

void destroyEngine(IEngine* e) {
  if (!s_alreadyExist) throw std::runtime_error{ "Error : engine not exist"s };
  if (e == nullptr) throw std::runtime_error{ "Error : nullptr"s };
  delete e;
  s_alreadyExist = false;
}