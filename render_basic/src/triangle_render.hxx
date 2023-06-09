//
// Created by Алексей Крукович on 26.04.23.
//

#ifndef RENDER_BASIC_TRIANGLE_RENDER_HXX
#define RENDER_BASIC_TRIANGLE_RENDER_HXX
#include "line_render.hxx"

namespace graphics {

struct TriangleVertices
{
    Position v0{};
    Position v1{};
    Position v2{};

    auto operator<=>(const TriangleVertices& triangleVertices) const = default;

    static bool isTriangle(const TriangleVertices& triangleVertices) {
        double a{ distance(triangleVertices.v0, triangleVertices.v1) };
        double b{ distance(triangleVertices.v1, triangleVertices.v2) };
        double c{ distance(triangleVertices.v2, triangleVertices.v0) };

        return (a + b > c) && (a + c > b) && (b + c > a);
    }

    static TriangleVertices
    generateRandom(std::size_t width, std::size_t height, unsigned seed = 0) {
        TriangleVertices result{};
        result.v0 = Position::generateRandom(width, height, seed ? seed : 0);
        result.v1 = Position::generateRandom(width, height, seed ? seed + 10 : 0);
        result.v2 = Position::generateRandom(width, height, seed ? seed + 20 : 0);

        if (!isTriangle(result)) return generateRandom(width, height, seed);
        return result;
    }
};

class TriangleRender : public LineRender
{
public:
    TriangleRender(Canvas& canvas, std::size_t width, std::size_t height)
        : LineRender{ canvas, width, height } {}

    [[nodiscard]] virtual PixelPositions
    pixelPositionsTriangle(Position v0, Position v1, Position v2) const {
        if (!TriangleVertices::isTriangle({ v0, v1, v2 }))
            throw std::runtime_error{ "Error : pixelPositionsTriangle : not a triangle"s };

        PixelPositions result{};
        for (const auto [start, end] :
             { std::pair{ v0, v1 }, std::pair{ v1, v2 }, std::pair{ v2, v0 } }) {

            PixelPositions line{ LineRender::pixelPositions(start, end) };
            result.insert(result.end(), line.begin(), line.end());
        }

        return result;
    }

    void drawTriangles(const std::vector<Position>& vertices, Color color) {
        if (vertices.size() % 3 != 0)
            throw std::runtime_error{ "Error : drawTriangles : vertices.size() % 3 != 0"s };

        for (std::size_t i{}; i < vertices.size() / 3; ++i) {
            auto v0{ vertices.at(i * 3 + 0) };
            auto v1{ vertices.at(i * 3 + 1) };
            auto v2{ vertices.at(i * 3 + 2) };

            auto trianglePositions{ pixelPositionsTriangle(v0, v1, v2) };
            std::ranges::for_each(trianglePositions, [&](const Position position) {
                m_canvas.setPixel(position, color);
            });
        }
    }
};

} // namespace graphics

#endif // RENDER_BASIC_TRIANGLE_RENDER_HXX
