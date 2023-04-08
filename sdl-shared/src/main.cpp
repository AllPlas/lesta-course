#include <iostream>
#include <SDL3/SDL_version.h>

std::ostream& operator<<(std::ostream& out, const SDL_version& version) {
  std::cout << static_cast<int>(version.major) << '.' << static_cast<int>(version.minor) << '.'
            << static_cast<int>(version.patch);
  return out;
}

int main() {
  SDL_version compiled{};
  SDL_version linked{};

  SDL_VERSION(&compiled);
  SDL_GetVersion(&linked);

  std::cout << "compiled: " << compiled << '\n';
  std::cout << "linked: " << linked << std::endl;

  return std::cout.good() ? EXIT_SUCCESS : EXIT_FAILURE;
}
