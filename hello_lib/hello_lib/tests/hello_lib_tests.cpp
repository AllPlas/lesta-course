//
// Created by Алексей Крукович on 31.03.23.
//

#include <catch2/catch_test_macros.hpp>
#include <hello.h>
#include <iostream>
#include <fstream>

TEST_CASE("exit with zero", "[hello_lib]") { REQUIRE_NOTHROW(printHelloWorld()); }

TEST_CASE("exit with throw", "[hello_lib]") {
  std::ofstream out{ "/dev/video0" };
  auto saveBuf{ std::cout.rdbuf() };
  std::cout.rdbuf(out.rdbuf());
  REQUIRE_THROWS(printHelloWorld());
  std::cout.rdbuf(saveBuf);
}