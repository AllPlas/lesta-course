//
// Created by Алексей Крукович on 27.04.23.
//

#ifndef RENDER_BASIC_TRIANGLE_INDEXED_RENDER_HXX
#define RENDER_BASIC_TRIANGLE_INDEXED_RENDER_HXX
#include "triangle_render.hxx"

namespace graphics {

class TriangleIndexedRender : public TriangleRender
{
public:
    TriangleIndexedRender(Canvas& canvas, std::size_t width, std::size_t height)
        : TriangleRender(canvas, width, height) {}

    void drawTriangles(const std::vector<Position>& vertices,
                       const std::vector<std::uint16_t>& indices,
                       Color color) {
        if (indices.size() % 3 != 0)
            throw std::runtime_error{ "Error : drawTriangles : indices.size() % 3 != 0"s };

        for (std::size_t i{}; i < indices.size() / 3; ++i) {
            auto index0{ indices.at(i * 3 + 0) };
            auto index1{ indices.at(i * 3 + 1) };
            auto index2{ indices.at(i * 3 + 2) };

            auto v0{ vertices.at(index0) };
            auto v1{ vertices.at(index1) };
            auto v2{ vertices.at(index2) };

            auto trianglePosition{ pixelPositionsTriangle(v0, v1, v2) };
            std::ranges::for_each(trianglePosition, [&](const Position position) {
                m_canvas.setPixel(position, color);
            });
        }
    }
};

} // namespace graphics

#endif // RENDER_BASIC_TRIANGLE_INDEXED_RENDER_HXX
