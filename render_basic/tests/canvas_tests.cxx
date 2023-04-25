//
// Created by Алексей Крукович on 25.04.23.
//

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers.hpp>
#include <catch2/matchers/catch_matchers_contains.hpp>
#include <catch2/matchers/catch_matchers_predicate.hpp>
#include <catch2/matchers/catch_matchers_quantifiers.hpp>

#include <algorithm>
#include <filesystem>

#include "../src/canvas.hxx"

using namespace std::literals;
using Catch::Matchers::Contains;
using Catch::Matchers::Predicate;
using namespace Catch::Matchers;
namespace fs = std::filesystem;

void plainColorTest(graphics::Color color) {
    constexpr std::size_t width{ 640 };
    constexpr std::size_t height{ 480 };
    graphics::Canvas canvas{ width, height };

    std::for_each(canvas.begin(), canvas.end(), [color](graphics::Color& c) {
        c = color;
    });

    REQUIRE_THAT(canvas.getPixels(),
                 AllMatch(Predicate<graphics::Color>(
                     [color](const graphics::Color c) { return c == color; })));

    std::string_view filename{ "test.ppm" };
    canvas.saveImage(filename);

    REQUIRE(fs::exists(filename));

    canvas.clear();

    CHECK_THAT(canvas.getPixels(),
               AllMatch(Predicate<graphics::Color>([](const graphics::Color c) {
                   return c == graphics::Color{};
               })));

    canvas.loadImage(filename);

    REQUIRE_THAT(canvas.getPixels(),
                 AllMatch(Predicate<graphics::Color>(
                     [color](const graphics::Color c) { return c == color; })));

    fs::remove(filename);
}

SCENARIO("Plain red canvas test", "[canvas]") { plainColorTest(graphics::red); }

SCENARIO("Plain green canvas test", "[canvas]") {
    plainColorTest(graphics::green);
}

SCENARIO("Plain blue canvas test", "[canvas]") {
    plainColorTest(graphics::blue);
}