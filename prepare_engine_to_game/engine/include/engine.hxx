//
// Created by Алексей Крукович on 17.04.23.
//

#ifndef SDL_ENGINE_EXE_ENGINE_HXX
#define SDL_ENGINE_EXE_ENGINE_HXX

#include <glm/glm.hpp>

#include <array>
#include <cstdint>
#include <functional>
#include <iosfwd>
#include <memory>
#include <string>
#include <string_view>

#include "buffer.hxx"
#include "sprite.hxx"
#include "texture.hxx"

struct Event
{
    enum class Type
    {
        key_down,
        key_up,
        mouse_down,
        mouse_up,
        mouse_wheel,
        mouse_motion,
        turn_off,

        not_event,
    };

    struct Keyboard
    {
        enum class Key
        {
            w,
            a,
            s,
            d,
            space,
            enter,
            l_control,
            l_shift,
            escape,
            backspace,

            not_key,
        };

        Key key{ Key::not_key };
    };

    struct Mouse
    {
        enum class Button
        {
            left,
            right,
            middle,

            not_button,
        };

        struct Pos
        {
            float x{};
            float y{};
        };

        struct Wheel
        {
            float x{};
            float y{};
        };

        Button button{ Button::not_button };
        Pos pos{};
        Wheel wheel{};
    };

    Type type{ Type::not_event };
    Keyboard keyboard{};
    Mouse mouse{};
};

std::ostream& operator<<(std::ostream& out, const Event& event);

struct Triangle
{
    std::array<Vertex, 3> vertices{};
};

std::ifstream& operator>>(std::ifstream& in, Triangle& triangle);

struct Triangle2
{
    std::array<Vertex2, 3> vertices{};
};

std::ifstream& operator>>(std::ifstream& in, Triangle2& triangle2);

class IEngine
{
public:
    virtual ~IEngine() = default;
    virtual std::string initialize([[maybe_unused]] std::string_view config) = 0;
    virtual void uninitialize() = 0;
    virtual bool readInput(Event& event) = 0;
    virtual void renderTriangle(const Triangle& triangle) = 0;
    virtual void renderTriangle(const Triangle& triangle, const Texture& texture) = 0;
    virtual void swapBuffers() = 0;
    virtual void recompileShaders(std::string_view vertexPath, std::string_view fragmentPath) = 0;
    virtual void render(const VertexBuffer<Vertex2>& vertexBuffer,
                        const IndexBuffer<std::uint16_t>& indexBuffer,
                        const Texture& texture) = 0;
    virtual void render(const Sprite& sprite) = 0;
    [[nodiscard]] virtual std::pair<int, int> getWindowSize() const noexcept = 0;
    virtual void setVSync(bool isEnable) = 0;
    [[nodiscard]] virtual bool getVSync() const noexcept = 0;
    virtual void setFramerate(int framerate) = 0;
    [[nodiscard]] virtual int getFramerate() const noexcept = 0;
};

using EnginePtr = std::unique_ptr<IEngine, std::function<void(IEngine*)>>;

void createEngine();
void destroyEngine(IEngine* e);

const EnginePtr& getEngineInstance();

class IGame
{
public:
    virtual ~IGame() = default;
    virtual void initialize() = 0;
    virtual void onEvent(const Event& event) = 0;
    virtual void update() = 0;
    virtual void render() const = 0;
};

extern "C" IGame* createGame(IEngine* engine);
extern "C" void destroyGame(IGame* game);

#endif // SDL_ENGINE_EXE_ENGINE_HXX
