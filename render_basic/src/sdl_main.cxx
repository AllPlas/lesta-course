//
// Created by Алексей Крукович on 1.05.23.
//
#include <SDL3/SDL.h>
#include <chrono>

#include "gfx_program.hxx"
#include "triangle_interpolated.hxx"

using namespace std::literals;

namespace graphics {
class GfxProgram : public graphics::IGfx
{
private:
    Color m_color{};
    double m_scale{ 1 };
    double m_mouseX{};
    double m_mouseY{};
    double radius{ 20 };

public:
    void setUniforms(const graphics::Uniform& uniform) override {
        m_scale = uniform.p1;
        m_mouseX = uniform.p2;
        m_mouseY = uniform.p3;

        m_color.red = std::abs(uniform.p4);
        m_color.green = std::abs(uniform.p5);
        m_color.blue = std::abs(uniform.p6);

        radius = uniform.p7;
    };

    Color fragmentShader(const graphics::Vertex& vertex) override {
        Color color{ vertex.extractColor() };

        auto dist{ distance(vertex.extractPosition(), graphics::Position(m_mouseX, m_mouseY)) };
        if (dist <= radius * m_scale) return color;

        Color rand{ Color::generateRandom() };
        color.red = graphics::interpolate(vertex.r, rand.red, 0.5);
        color.green = graphics::interpolate(vertex.g, rand.green, 0.5);
        color.blue = graphics::interpolate(vertex.b, rand.blue, 0.5);

        return color;
    };

    Vertex vertexShader(const graphics::Vertex& vertex) override {
        Vertex vertex1{ vertex };

        auto distX{ 640 / 2 - vertex1.x };
        auto distY{ 480 / 2 - vertex1.y };

        distX *= m_scale;
        distY *= m_scale;

        vertex1.x = 640 / 2 - distX;
        vertex1.y = 480 / 2 - distY;

        return vertex1;
    };
};

} // namespace graphics

int main() {
    constexpr int width{ 640 };
    constexpr int height{ 480 };

    SDL_Init(SDL_INIT_EVERYTHING);
    auto window{ SDL_CreateWindow("render", width, height, SDL_WINDOW_OPENGL) };
    auto renderer{ SDL_CreateRenderer(window, nullptr, SDL_RENDERER_ACCELERATED) };

    graphics::Canvas canvas{ width, height };
    graphics::GfxProgram gfx;
    graphics::TriangleInterpolateRender render{ canvas, width, height, gfx };

    std::vector<graphics::Vertex> verticesBuffer{ { 0, 0, 255, 0, 0 },
                                                  { width - 1, height - 1, 0, 255, 0 },
                                                  { 0, height - 1, 0, 0, 255 },
                                                  { width - 1, 0, 0, 0, 255 } };
    std::vector<std::uint16_t> indicesBuffer{ 0, 1, 2, 0, 1, 3 };

    void* pixels{ canvas.getPixels().data() };
    constexpr int pitch{ sizeof(graphics::Color) * width };

    graphics::Uniform uniform{};

    double& scale = uniform.p1 = 1;
    double& mouseX = uniform.p2;
    double& mouseY = uniform.p3;
    double& redColor = uniform.p4;
    double& greenColor = uniform.p5;
    double& blueColor = uniform.p6;
    double& radius = uniform.p7 = 20;

    constexpr double scaleStep{ 0.05 };
    constexpr double radiusStep{ 5 };
    bool isLCtrl{ false };

    bool isExit{ false };

    while (!isExit) {
        SDL_Event event{};
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                isExit = true;
                break;
            }

            if (event.type == SDL_EVENT_KEY_DOWN) {
                if (event.key.keysym.sym == SDLK_LCTRL) isLCtrl = true;
            }

            if (event.type == SDL_EVENT_KEY_UP) {
                if (event.key.keysym.sym == SDLK_LCTRL) isLCtrl = false;
            }

            else if (event.type == SDL_EVENT_MOUSE_WHEEL) {
                if (isLCtrl) {
                    if (event.wheel.y > 0)
                        radius += radiusStep;
                    else if (event.wheel.y < 0)
                        radius -= radiusStep;
                }

                else if (event.wheel.y > 0)
                    scale += scaleStep;
                else if (event.wheel.y < 0)
                    scale -= scaleStep;
            }

            if (event.type == SDL_EVENT_MOUSE_MOTION) {
                mouseX = event.motion.x;
                mouseY = event.motion.y;
            }
        }

        canvas.clear();

        if (scale < 0.05)
            scale = 0.05;
        else if (scale > 1)
            scale = 1;

        if (mouseX > width - 1 - radius) { mouseX = width - 1 - radius; }
        else if (mouseX < radius) { mouseX = radius; }

        if (mouseY > height - 1 - radius) { mouseY = height - 1 - radius; }
        else if (mouseY < radius) { mouseY = radius; }

        if (radius >= height / 2) radius = height / 2;

        gfx.setUniforms(uniform);

        render.drawTriangles(verticesBuffer, indicesBuffer);

        auto bitmapSurface =
            SDL_CreateSurfaceFrom(pixels, width, height, pitch, SDL_PIXELFORMAT_RGB24);

        auto texture = SDL_CreateTextureFromSurface(renderer, bitmapSurface);

        SDL_DestroySurface(bitmapSurface);
        SDL_RenderClear(renderer);
        SDL_RenderTexture(renderer, texture, nullptr, nullptr);
        SDL_RenderPresent(renderer);
        SDL_DestroyTexture(texture);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}
