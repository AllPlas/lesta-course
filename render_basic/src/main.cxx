#include "canvas.hxx"
#include "line_render.hxx"
#include "triangle_indexed_render.hxx"
#include "triangle_render.hxx"

int main() {
    constexpr int w{ 1920 };
    constexpr int h{ 1080 };
    graphics::Canvas canvas{ w, h };
    canvas.clear({ 0, 0, 0 });
    graphics::TriangleIndexedRender render{ canvas, w, h };

    constexpr int xMax{ 20 };
    constexpr int yMax{ 20 };
    constexpr int stepX{ (w - 1) / xMax };
    constexpr int stepY{ (h - 1) / yMax };

    std::vector<graphics::Position> verticesBuffer{};
    for (std::size_t i{}; i <= yMax; ++i)
        for (std::size_t j{}; j <= xMax; ++j) {
            verticesBuffer.emplace_back(j * stepX, i * stepY);
        }

    std::vector<std::uint16_t> indicesBuffer{};

    for (std::size_t i{}; i < yMax; ++i)
        for (std::size_t j{}; j < xMax; ++j) {
            std::uint16_t index0{ static_cast<uint16_t>(i * (xMax + 1) + j) };
            std::uint16_t index1{ static_cast<uint16_t>((i + 1) * (xMax + 1) + j + 1) };
            std::uint16_t index2{ static_cast<uint16_t>(index1 - 1) };
            std::uint16_t index3{ static_cast<uint16_t>(index0 + 1) };

            indicesBuffer.push_back(index0);
            indicesBuffer.push_back(index1);
            indicesBuffer.push_back(index2);

            indicesBuffer.push_back(index0);
            indicesBuffer.push_back(index1);
            indicesBuffer.push_back(index3);
        }

    render.drawTriangles(verticesBuffer, indicesBuffer, graphics::green);
    canvas.saveImage("drawLine.ppm");
}
