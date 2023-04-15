#include <iostream>
#include <stdexcept>
#include <unordered_map>

#include <SDL3/SDL.h>

using namespace std::literals;

void initSDL() {
  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) != 0)
    throw std::runtime_error{ "Error : failed call SDL_Init: "s + SDL_GetError() };
}

SDL_Window* createWindow() {
  if (auto window{ SDL_CreateWindow("SDL3 infinity loop", 640, 480, SDL_WINDOW_OPENGL) };
      window != nullptr)
    return window;

  SDL_Quit();
  throw std::runtime_error{ "Error : failed call SDL_CreateWindow: "s + SDL_GetError() };
}

SDL_Renderer* createRenderer(SDL_Window* window) {
  if (auto renderer{ SDL_CreateRenderer(window, "opengl", SDL_RENDERER_ACCELERATED) };
      renderer != nullptr)
    return renderer;

  SDL_DestroyWindow(window);
  SDL_Quit();
  throw std::runtime_error{ "Error : failed call SDL_CreateRenderer: "s + SDL_GetError() };
}

void checkKeyboardInput(SDL_Event& e) {
  std::unordered_map<SDL_Keycode, std::string_view> keymap{
    { SDLK_w, "up" },          { SDLK_s, "down" },      { SDLK_a, "left" },
    { SDLK_d, "right" },       { SDLK_SPACE, "space" }, { SDLK_LCTRL, "L CTRL" },
    { SDLK_RETURN, "return" },
  };

  if (auto found{ keymap.find(e.key.keysym.sym) }; found != keymap.end()) {
    if (e.type == SDL_EVENT_KEY_DOWN)
      std::cout << found->second << " pressed\n";
    else if (e.type == SDL_EVENT_KEY_UP)
      std::cout << found->second << " released\n";
  }
}

int main() {
  try {
    initSDL();
    auto window{ createWindow() };
    // auto renderer{ createRenderer(window) };
    // SDL_RenderPresent(renderer);

    bool isEnd{};
    while (!isEnd) {
      SDL_Event sdlEvent;

      while (SDL_PollEvent(&sdlEvent)) {
        switch (sdlEvent.type) {
          case SDL_EVENT_KEY_DOWN:
            [[fallthrough]];
          case SDL_EVENT_KEY_UP:
            checkKeyboardInput(sdlEvent);
            break;

          case SDL_EVENT_QUIT:
            isEnd = true;
            break;
        }
      }
    }

    SDL_DestroyWindow(window);
    SDL_Quit();
    return EXIT_SUCCESS;
  }
  catch (const std::exception& e) {
    std::cerr << e.what() << '\n';
    return EXIT_FAILURE;
  }
  catch (...) {
    std::cerr << "Unknown Error\n"sv;
    return EXIT_FAILURE;
  }
}