#include <iostream>

using namespace std::literals;

int main() {
  std::cout << "Hello, World!"sv << std::endl;
  return std::cout.good() ? EXIT_SUCCESS : EXIT_FAILURE;
}
