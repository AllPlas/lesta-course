//
// Created by Алексей Крукович on 26.04.23.
//

#ifndef RENDER_BASIC_TRIANGLE_RENDER_HXX
#define RENDER_BASIC_TRIANGLE_RENDER_HXX
#include "line_render.hxx"

namespace graphics {

class TriangleRender : LineRender
{
public:
    TriangleRender(Canvas& canvas, std::size_t width, std::size_t height)
        : LineRender{ canvas, width, height } {}


};
} // namespace graphics

#endif // RENDER_BASIC_TRIANGLE_RENDER_HXX
