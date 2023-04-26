//
// Created by Алексей Крукович on 26.04.23.
//

#ifndef RENDER_BASIC_TRIANGLE_RENDER_HXX
#define RENDER_BASIC_TRIANGLE_RENDER_HXX
#include "line_render.hxx"

namespace graphics {

struct TriangleVertexes
{
    Position v0{};
    Position v1{};
    Position v2{};

    auto operator<=>(const TriangleVertexes& triangleVertexes) const = default;

    static bool isTriangle(const TriangleVertexes& triangleVertexes) {
        double a{ distance(triangleVertexes.v0, triangleVertexes.v1) };
        double b{ distance(triangleVertexes.v1, triangleVertexes.v2) };
        double c{ distance(triangleVertexes.v2, triangleVertexes.v0) };

        return (a + b > c) && (a + c > b) && (b + c > a);
    }

    static TriangleVertexes
    generateRandom(std::size_t width, std::size_t height, unsigned seed = 0) {
        TriangleVertexes result{};
        result.v0 = Position::generateRandom(width, height, seed ? seed : 0);
        result.v1 = Position::generateRandom(width, height, seed ? seed + 10 : 0);
        result.v2 = Position::generateRandom(width, height, seed ? seed + 20 : 0);

        if (!isTriangle(result)) return generateRandom(width, height, seed);
        return result;
    }
};

class TriangleRender : LineRender
{
public:
    TriangleRender(Canvas& canvas, std::size_t width, std::size_t height)
        : LineRender{ canvas, width, height } {}

    [[nodiscard]] virtual PixelsPositions
    pixelsPositionsTriangle(Position v0, Position v1, Position v2) const {
        if (!TriangleVertexes::isTriangle({ v0, v1, v2 }))
            throw std::runtime_error{ "Error : pixelsPositionsTriangle : not a triangle"s };

        PixelsPositions result{};
        for (const auto [start, end] :
             { std::pair{ v0, v1 }, std::pair{ v1, v2 }, std::pair{ v2, v0 } }) {

            PixelsPositions line{ LineRender::pixelsPositions(start, end) };
            result.insert(result.end(), line.begin(), line.end());
        }

        return result;
    }

    void drawTriangles(const std::vector<Position>& vertexes, Color color) {
        if (vertexes.size() % 3 != 0)
            throw std::runtime_error{ "Error : drawTriangles : vertexes.size() % 3 != 0"s };

        for (std::size_t i{}; i < vertexes.size() / 3; ++i) {
            auto v0{ vertexes.at(i * 3 + 0) };
            auto v1{ vertexes.at(i * 3 + 1) };
            auto v2{ vertexes.at(i * 3 + 2) };

            auto trianglePositions{ pixelsPositionsTriangle(v0, v1, v2) };
            std::ranges::for_each(trianglePositions, [&](const Position position) {
                m_canvas.setPixel(position, color);
            });
        }
    }
};

} // namespace graphics

#endif // RENDER_BASIC_TRIANGLE_RENDER_HXX
