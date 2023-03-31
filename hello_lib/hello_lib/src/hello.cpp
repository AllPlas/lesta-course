//
// Created by Алексей Крукович on 31.03.23.
//
#include "hello.h"
#include <iostream>

void printHelloWorld() {
  using namespace std::literals;
  std::cout << "Hello, world!"sv << std::endl;
  if (std::cout.fail()) throw std::runtime_error{ "Error : print hello world"s };
}
