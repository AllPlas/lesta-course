//
// Created by Алексей Крукович on 1.05.23.
//
#ifndef RENDER_BASIC_GFX_PROGRAM_HXX
#define RENDER_BASIC_GFX_PROGRAM_HXX

namespace graphics {

struct Vertex;
struct Color;

struct Uniform
{
    double p1{};
    double p2{};
    double p3{};
    double p4{};
    double p5{};
    double p6{};
    double p7{};
};

class IGfx
{
public:
    virtual ~IGfx() = default;
    virtual void setUniforms(const Uniform& uniform) = 0;
    virtual Vertex vertexShader(const Vertex& vertex) = 0;
    virtual Color fragmentShader(const Vertex& vertex) = 0;
};

} // namespace graphics
#endif // RENDER_BASIC_GFX_PROGRAM_HXX