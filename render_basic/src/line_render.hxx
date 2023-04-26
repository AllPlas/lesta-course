//
// Created by Алексей Крукович on 25.04.23.
//

#ifndef RENDER_BASIC_LINE_RENDER_HXX
#define RENDER_BASIC_LINE_RENDER_HXX
#include <algorithm>
#include <cmath>
#include <ranges>

#include "canvas.hxx"

namespace graphics {

class LineRender : public IRender
{
private:
    Canvas& m_canvas;
    std::size_t m_width{};
    std::size_t m_height{};

public:
    LineRender(Canvas& canvas, std::size_t width, std::size_t height)
        : m_canvas{ canvas }, m_width{ width }, m_height{ height } {}

    [[nodiscard]] Positions pixelsPositions(graphics::Position start,
                                            graphics::Position end) const override {
        Positions positions;

        std::int64_t x0 = start.x;
        std::int64_t y0 = start.y;
        std::int64_t x1 = end.x;
        std::int64_t y1 = end.y;

        std::int64_t dx{ std::abs(x1 - x0) };
        std::int64_t dy{ std::abs(y1 - y0) };

        int sx{ x0 < x1 ? 1 : -1 };
        int sy{ y0 < y1 ? 1 : -1 };

        std::int64_t error{ dx > dy ? dx / 2 : dy / 2 };
        if (dx > dy) {
            while (x0 != x1) {
                positions.emplace_back(x0, y0);
                x0 += sx;
                error -= dy;
                if (error < 0) {
                    y0 += sy;
                    error += dx;
                }
            }
        }
        else {
            while (y0 != y1) {
                positions.emplace_back(x0, y0);
                y0 += sy;
                error -= dx;
                if (error < 0) {
                    x0 += sx;
                    error += dy;
                }
            }
        }

        positions.emplace_back(x0, y0); // отрезок включая последнюю точку [start, end]

        return positions;
    }

    void drawLine(Position start, Position end, Color color) {
        auto positions{ pixelsPositions(start, end) };
        std::ranges::for_each(
            positions, [&](const Position& position) { m_canvas.setPixel(position, color); });
    }
};

} // namespace graphics

#endif // RENDER_BASIC_LINE_RENDER_HXX
