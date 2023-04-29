#include "canvas.hxx"
#include "line_render.hxx"
#include "triangle_indexed_render.hxx"
#include "triangle_interpolated.hxx"
#include "triangle_render.hxx"

std::vector<graphics::Vertex> createCircleVertices(const unsigned segments,
                                                   const double centerX,
                                                   const double centerY,
                                                   const double radius) {
    std::vector<graphics::Vertex> vertices;
    vertices.push_back({ centerX, centerY, 255.0, 255.0, 255.0 });

    for (int i = 0; i <= segments; ++i) {
        double angle =
            2.0 * std::numbers::pi * static_cast<double>(i) / static_cast<double>(segments);
        double x = centerX + radius * std::cos(angle);
        double y = centerY + radius * std::sin(angle);
        graphics::Color color{ graphics::Color::generateRandom() };
        vertices.emplace_back(x, y, color.red, color.green, color.blue);
    }

    return vertices;
}

std::vector<std::uint16_t> createCircleIndices(const unsigned segments) {
    std::vector<std::uint16_t> indices;

    for (int i = 1; i <= segments; ++i) {
        indices.push_back(0);
        indices.push_back(i);
        indices.push_back(i + 1);
    }

    // замыкаем круг
    indices.push_back(0);
    indices.push_back(segments + 1);
    indices.push_back(1);

    return indices;
}

void drawInterpolatedTriangle() {
    constexpr int w{ 1920 };
    constexpr int h{ 1080 };

    graphics::Canvas canvas{ w, h };
    canvas.clear({});
    graphics::TriangleInterpolateRender render{ canvas, 1920, 1080 };
    graphics::Vertex v0{ 0, 0, 255, 0, 0 };
    graphics::Vertex v1{ 1919, 1079, 0, 255, 0 };
    graphics::Vertex v2{ 0, 1079, 0, 0, 255 };
    graphics::Vertex v3{ 1919, 0, 0, 0, 255 };
    std::vector<graphics::Vertex> verticesBuf{ v0, v1, v2, v3 };
    std::vector<std::uint16_t> indicesBuf{ 0, 1, 2, 0, 1, 3 };
    render.drawTriangles(verticesBuf, indicesBuf);
    canvas.saveImage("inter.ppm");
}

void drawHorizontalLine() {
    constexpr int w{ 1920 };
    constexpr int h{ 1080 };

    graphics::Canvas canvas{ w, h };
    canvas.clear({ 0, 0, 0 });
    graphics::LineRender render{ canvas, w, h };
    render.drawLine({ 0, 0 }, { 1079, 1079 }, graphics::green);
    canvas.saveImage("line.ppm");
}

void drawCircle() {
    const double centerX = 300.0;
    const double centerY = 300.0;
    const double radius = 200.0;

    std::vector<graphics::Vertex> verticesBuffer =
        createCircleVertices(60, centerX, centerY, radius);
    std::vector<std::uint16_t> indicesBuffer = createCircleIndices(60);

    graphics::Canvas canvas{ 1920, 1080 };
    graphics::TriangleInterpolateRender render{ canvas, 1920, 1080 };
    render.drawTriangles(verticesBuffer, indicesBuffer);
    canvas.saveImage("circle.ppm");
}

int main() {
    drawInterpolatedTriangle();
    drawHorizontalLine();
    drawCircle();
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
