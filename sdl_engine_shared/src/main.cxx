#include <iostream>
#include <functional>

#include "engine.hxx"

using namespace std::literals;

int main() {
  try {
    auto engine{ createEngine() };
    engine->initialize("");

    bool isEnd{};
    while (!isEnd) {
      Event event{};

      while (engine->readInput(event)) {
        std::cout << event << '\n';

        if (event == Event::turn_off) {
          isEnd = true;
          break;
        }
      }
    }

    engine->uninitialize();
    return EXIT_SUCCESS;
  }
  catch (const std::exception& e) {
    std::cerr << e.what() << '\n';
  }
  catch (...) {
    std::cerr << "Unknown error"sv;
  }
  return EXIT_FAILURE;
}
