//
// Created by Алексей Крукович on 25.04.23.
//

#include <catch2/catch_test_macros.hpp>

#include <sstream>

#include "../src/canvas.hxx"
#include "../src/line_render.hxx"

namespace Catch {
template <>
struct StringMaker<graphics::Position>
{
    static std::string convert(const graphics::Position& position) {
        std::ostringstream oss{};
        oss << '[' << position.x << ", " << position.y << ']';
        return oss.str();
    }
};
} // namespace Catch

using namespace graphics;

Positions pixels_positions(Position start, Position end) {
    Positions result;
    int x0 = start.x;
    int y0 = start.y;
    int x1 = end.x;
    int y1 = end.y;

    auto plot_line_low = [&](int x0, int y0, int x1, int y1) {
        int dx = x1 - x0;
        int dy = y1 - y0;
        int yi = 1;
        if (dy < 0) {
            yi = -1;
            dy = -dy;
        }
        int D = 2 * dy - dx;
        int y = y0;

        for (int x = x0; x <= x1; ++x) {
            result.emplace_back(x, y);
            if (D > 0) {
                y += yi;
                D -= 2 * dx;
            }
            D += 2 * dy;
        }
    };

    auto plot_line_high = [&](int x0, int y0, int x1, int y1) {
        int dx = x1 - x0;
        int dy = y1 - y0;
        int xi = 1;
        if (dx < 0) {
            xi = -1;
            dx = -dx;
        }
        int D = 2 * dx - dy;
        int x = x0;

        for (int y = y0; y <= y1; ++y) {
            result.emplace_back(x, y);
            if (D > 0) {
                x += xi;
                D -= 2 * dy;
            }
            D += 2 * dx;
        }
    };

    if (abs(y1 - y0) < abs(x1 - x0)) {
        if (x0 > x1) { plot_line_low(x1, y1, x0, y0); }
        else { plot_line_low(x0, y0, x1, y1); }
    }
    else {
        if (y0 > y1) { plot_line_high(x1, y1, x0, y0); }
        else { plot_line_high(x0, y0, x1, y1); }
    }
    return result;
}

SCENARIO("Draw line tests", "[line]") {
    constexpr std::size_t width{ 640 };
    constexpr std::size_t height{ 480 };
    Canvas canvas{ width, height };
    LineRender lineRender{ width, height, canvas };

    Position start{};
    Position end{};

    SECTION("Horizontal line") {
        start = { 0, 0 };
        end = { width - 1, 0 };
    }

    SECTION("Vertical line") {
        start = { 0, 0 };
        end = { 0, height - 1 };
    }

    SECTION("Line lower than 45 degree") {
        start = { 0, height / 2 };
        end = { width - 1, height / 4 };
    }

    SECTION("Line upper than 45 degree") {
        start = { 0, height / 2 };
        end = { width / 4, height - 1 };
    }

    SECTION("45 degree line") {
        start = { 0, 0 };
        end = { 120, 120 };
    }

    SECTION("Random line") {
        start = Position::generateRandom(width, height);
        end = Position::generateRandom(width, height);
    }

    CHECK(lineRender.pixelsPositions(start, end) == pixels_positions(start, end));

    auto pixelPositions{ pixels_positions(end, start) };
    CHECK(lineRender.pixelsPositions(end, start) ==
          std::vector<Position>{ pixelPositions.rbegin(), pixelPositions.rend() });
}