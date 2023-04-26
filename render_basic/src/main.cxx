#include "canvas.hxx"
#include "line_render.hxx"

int main() {
    constexpr int w{ 1920 };
    constexpr int h{ 1080 };
    graphics::Canvas canvas{ w, h };
    canvas.clear(graphics::red);
    graphics::LineRender lineRender{ canvas, canvas.getWidth(), canvas.getHeight() };

    for (std::size_t i{}; i < 100; ++i) {
        lineRender.drawLine(graphics::Position::generateRandom(w, h),
                            graphics::Position::generateRandom(w, h),
                            graphics::Color::generateRandom());
    }

    canvas.saveImage("drawLine.ppm");
}
