//
// Created by Алексей Крукович on 17.04.23.
//

#ifndef SDL_ENGINE_EXE_ENGINE_HXX
#define SDL_ENGINE_EXE_ENGINE_HXX

#include <glm/glm.hpp>

#include <array>
#include <cstdint>
#include <functional>
#include <imgui.h>
#include <iosfwd>
#include <memory>
#include <string>
#include <string_view>

#include "audio.hxx"
#include "buffer.hxx"
#include "shader_program.hxx"
#include "sprite.hxx"
#include "texture.hxx"
#include "view.hxx"

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
        touch_down,
        touch_up,
        touch_motion,
        window_resized,
        turn_off,

        not_event,
    };

    struct Keyboard
    {
        enum class Key
        {
            q,
            w,
            e,
            r,
            t,
            y,
            u,
            i,
            o,
            p,
            a,
            s,
            d,
            f,
            g,
            h,
            j,
            k,
            l,
            z,
            x,
            c,
            v,
            b,
            n,
            m,
            space,
            enter,
            l_control,
            l_shift,
            escape,
            backspace,

            // Number keys
            num_0,
            num_1,
            num_2,
            num_3,
            num_4,
            num_5,
            num_6,
            num_7,
            num_8,
            num_9,

            // Function keys
            f1,
            f2,
            f3,
            f4,
            f5,
            f6,
            f7,
            f8,
            f9,
            f10,
            f11,
            f12,

            // Arrow keys
            up_arrow,
            down_arrow,
            left_arrow,
            right_arrow,

            // Miscellaneous keys
            insert,
            home,
            page_up,
            delete_key,
            end,
            page_down,
            caps_lock,
            scroll_lock,
            num_lock,
            print_screen,
            pause,

            // Numpad keys
            numpad_0,
            numpad_1,
            numpad_2,
            numpad_3,
            numpad_4,
            numpad_5,
            numpad_6,
            numpad_7,
            numpad_8,
            numpad_9,
            numpad_decimal,
            numpad_divide,
            numpad_multiply,
            numpad_subtract,
            numpad_add,
            numpad_enter,

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

    struct Touch
    {
        std::size_t id{};
        Position pos{};
        float dx{};
        float dy{};
    };

    Type type{ Type::not_event };
    Keyboard keyboard{};
    Mouse mouse{};
    Touch touch{};
};

std::ostream& operator<<(std::ostream& out, const Event& event);

std::string_view keyToStr(Event::Keyboard::Key key);
Event::Keyboard::Key ImGuiKeyToEventKey(ImGuiKey key);

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
    struct WindowSize
    {
        int width{};
        int height{};
    };

    virtual ~IEngine() = default;
    virtual std::string initialize([[maybe_unused]] std::string_view config) = 0;
    virtual void uninitialize() = 0;
    virtual bool readInput(Event& event) = 0;
    virtual void swapBuffers() = 0;
    virtual void recompileShaders() = 0;
    virtual void render(const VertexBuffer<Vertex2>& vertexBuffer,
                        const IndexBuffer<std::uint16_t>& indexBuffer,
                        const Texture& texture) = 0;
    virtual void render(const VertexBuffer<Vertex2>& vertexBuffer,
                        const IndexBuffer<std::uint32_t>& indexBuffer,
                        const Texture& texture) = 0;
    virtual void render(const VertexBuffer<Vertex2>& vertexBuffer,
                        const IndexBuffer<std::uint32_t>& indexBuffer,
                        const Texture& texture,
                        const glm::mat3& matrix) = 0;
    virtual void render(const VertexBuffer<Vertex2>& vertexBuffer,
                        const IndexBuffer<std::uint32_t>& indexBuffer,
                        const Texture& texture,
                        const glm::mat3& matrix,
                        const View& view) = 0;
    virtual void render(const Sprite& sprite) = 0;
    virtual void render(const Sprite& sprite, const View& view) = 0;
    [[nodiscard]] virtual WindowSize getWindowSize() const noexcept = 0;
    virtual void setVSync(bool isEnable) = 0;
    [[nodiscard]] virtual bool getVSync() const noexcept = 0;
    virtual void setFramerate(int framerate) = 0;
    [[nodiscard]] virtual int getFramerate() const noexcept = 0;
    [[nodiscard]] virtual ImGuiContext* getImGuiContext() const noexcept = 0;
    [[nodiscard]] virtual std::vector<std::string> getAudioDeviceNames() const noexcept = 0;
    [[nodiscard]] virtual const std::string& getCurrentAudioDeviceName() const noexcept = 0;
    virtual void setAudioDevice(std::string_view audioDeviceName) = 0;
    [[nodiscard]] virtual int getAudioVolume() const noexcept = 0;
    virtual void setAudioVolume(int audioVolume) = 0;
    [[nodiscard]] virtual bool isFullscreen() const noexcept = 0;
    virtual void setFullscreen(bool isFullscreen) = 0;
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
    virtual void render() = 0;
};

extern "C" IGame* createGame(IEngine* engine);
extern "C" void destroyGame(IGame* game);

#endif // SDL_ENGINE_EXE_ENGINE_HXX
