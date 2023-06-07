//
// Created by Алексей Крукович on 17.04.23.
//

#ifndef SDL_ENGINE_EXE_ENGINE_HXX
#define SDL_ENGINE_EXE_ENGINE_HXX

#include <cstdint>
#include <functional>
#include <iosfwd>
#include <memory>
#include <string>
#include <string_view>

#include "../src/buffer.hxx"

enum class Event
{
    up_pressed,
    up_released,
    down_pressed,
    down_released,
    left_pressed,
    left_released,
    right_pressed,
    right_released,
    space_pressed,
    space_released,
    return_pressed,
    return_released,
    lctrl_pressed,
    lctrl_released,
    turn_off,

    max_events,
};

std::ostream& operator<<(std::ostream& out, Event event);

struct Triangle
{
    std::array<Vertex, 3> vertices{};
};

std::ifstream& operator>>(std::ifstream& in, Triangle& triangle);

class Texture;

class IEngine
{
public:
    virtual ~IEngine() = default;
    virtual std::string initialize([[maybe_unused]] std::string_view config) = 0;
    virtual void uninitialize() = 0;
    virtual bool readInput(Event& event) = 0;
    virtual void renderTriangle(const Triangle& triangle) = 0;
    virtual void renderTriangle(const Triangle& triangle, const Texture& texture) = 0;
    virtual void renderFromBuffer() = 0;
    virtual void swapBuffers() = 0;
    virtual void recompileShaders(std::string_view vertexPath, std::string_view fragmentPath) = 0;
    virtual void reloadVerticesBuffer(std::string_view verticesPath) = 0;
    virtual void reloadIndicesBuffer(std::string_view indicesPath) = 0;

    virtual void render(const VertexBuffer<Vertex2>& vertexBuffer,
                        const IndexBuffer<std::uint16_t>& indexBuffer,
                        Texture* texture,
                        std::uint16_t startIndex,
                        std::size_t numVertices) = 0;
};

using EnginePtr = std::unique_ptr<IEngine, std::function<void(IEngine*)>>;

EnginePtr createEngine();
void destroyEngine(IEngine* e);

IEngine* getEngineInstance();

class IGame
{
public:
    virtual ~IGame() = default;
    virtual void initialize() = 0;
    virtual void onEvent(Event event) = 0;
    virtual void update() = 0;
    virtual void render() const = 0;
};

extern "C" IGame* createGame(IEngine* engine);
extern "C" void destroyGame(IGame* game);

#endif // SDL_ENGINE_EXE_ENGINE_HXX
