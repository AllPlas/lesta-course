//
// Created by Алексей Крукович on 17.04.23.
//
#include "engine.hxx"

#include <iostream>
#include <unordered_map>
#include <stdexcept>
#include <optional>
#include <thread>
#include <filesystem>
#include <SDL3/SDL.h>

using namespace std::literals;
namespace fs = std::filesystem;

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

static std::unique_ptr<IGame, std::function<void(IGame* game)>> reloadGame(
    std::unique_ptr<IGame, std::function<void(IGame* game)>> oldGame, std::string_view libraryName,
    std::string_view tempLibraryName, IEngine& engine, void*& oldHandle) {
  if (oldGame) {
    auto ptr{ oldGame.release() };
    // destroyGame(ptr);
    delete ptr;
    SDL_UnloadObject(oldHandle);
  }

  fs::remove(tempLibraryName);
  fs::copy(libraryName, tempLibraryName);

  auto gameHandle{ SDL_LoadObject(tempLibraryName.data()) };
  if (gameHandle == nullptr) {
    std::cerr << "Failed SDL_LoadObject\n"sv;
    return nullptr;
  }

  oldHandle = gameHandle;

  auto createGameFuncPtr{ SDL_LoadFunction(gameHandle, "createGame") };
  if (createGameFuncPtr == nullptr) {
    std::cerr << "Failed SDL_LoadFunction\n"sv;
    return nullptr;
  }

  using CreateGame = decltype(&createGame);
  auto createGameLinked{ reinterpret_cast<CreateGame>(createGameFuncPtr) };

  return { createGameLinked(&engine), [](IGame* game) { delete game; } };
}

int main() {
  try {
    auto engine{ createEngine() };
    engine->initialize("");
    std::cout << "start app"sv << std::endl;

    std::string_view libraryName{ SDL_GetPlatform() == "Windows"s ? "game.dll" : "./game.so" };
    std::string_view tempLibraryName{ "./temp.dll" };
    void* gameLibraryHandle{};
    auto game{ reloadGame(nullptr, libraryName, tempLibraryName, *engine, gameLibraryHandle) };
    auto timeDuringLoading{ fs::last_write_time(libraryName) };
    game->initialize();

    bool isEnd{};
    while (!isEnd) {
      auto currentWriteTime{ fs::last_write_time(libraryName) };
      if (currentWriteTime != timeDuringLoading) {
        while (true) {
          std::this_thread::sleep_for(100ms);
          auto nextWriteTime{ fs::last_write_time(libraryName) };

          if (nextWriteTime != currentWriteTime)
            currentWriteTime = nextWriteTime;
          else
            break;
        }

        std::cout << "reloading game\n"sv;
        game =
            reloadGame(std::move(game), libraryName, tempLibraryName, *engine, gameLibraryHandle);

        if (game == nullptr) {
          std::cerr << "next attempt to reload game\n"sv;
          continue;
        }

        timeDuringLoading = currentWriteTime;
      }

      Event event{};
      while (engine->readInput(event)) {
        std::cout << event << '\n';

        if (event == Event::turn_off) {
          isEnd = true;
          break;
        }

        game->onEvent(event);
      }

      if (event == Event::turn_off) break;
      game->update();
      game->render();
    }

    engine->uninitialize();
    return EXIT_SUCCESS;
  }
  catch (const std::exception& e) {
    std::cerr << e.what() << '\n';
  }
  catch (...) {
    std::cerr << "Unknown error"sv << '\n';
  }
  return EXIT_FAILURE;
}