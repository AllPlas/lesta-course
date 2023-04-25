#include <iostream>

#include "canvas.hxx"
#include "line_render.hxx"

int main() {
    graphics::Canvas canvas{ 3840, 2160 };
    canvas.clear(graphics::red);

    graphics::LineRender lineRender{ canvas.getWidth(), canvas.getHeight(), canvas };

    lineRender.drawLine({ 606, 424 }, { 426, 253 }, graphics::green);
    canvas.saveImage("drawLine.ppm");
}
