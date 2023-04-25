#include <iostream>

#include "canvas.hxx"
#include "line_render.hxx"

int main() {
    graphics::Canvas canvas{ 1920, 1080 };
    canvas.clear(graphics::red);

    graphics::LineRender lineRender{ canvas.getWidth(), canvas.getHeight(), canvas };

    lineRender.drawLine({ 0, 0 }, { 1079, 1079 }, graphics::green);
    canvas.saveImage("drawLine.ppm");
}
