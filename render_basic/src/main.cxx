#include <iostream>

#include "canvas.hxx"
#include "line_render.hxx"
#include "triangle_render.hxx"

int main() {
    constexpr int w{ 1920 };
    constexpr int h{ 1080 };
    graphics::Canvas canvas{ w, h };
    canvas.clear(graphics::red);
    graphics::TriangleRender render{ canvas, w, h };
    std::vector<graphics::Position> trianglesPos{};
    for (std::size_t i{}; i < 20; ++i) {
        auto rand{ graphics::TriangleVertexes::generateRandom(w, h) };
        trianglesPos.push_back(rand.v0);
        trianglesPos.push_back(rand.v1);
        trianglesPos.push_back(rand.v2);
    }
    render.drawTriangles(trianglesPos, graphics::green);

    canvas.saveImage("drawLine.ppm");
}
