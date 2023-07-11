#include "engine.hxx"

#include <glm/glm.hpp>

#include <SDL3/SDL.h>
#include <cassert>
#include <filesystem>
#include <fstream>
#include <glad/glad.h>
#include <iostream>
#include <mutex>
#include <optional>
#include <stdexcept>
#include <thread>
#include <unordered_map>

#include "hot_reload_provider.hxx"
#include "imgui_impl_opengl3.hxx"
#include "imgui_impl_sdl3.hxx"
#include "opengl_check.hxx"

#ifndef __ANDROID__
#    include <boost/json.hpp>
#    include <boost/program_options.hpp>

namespace json = boost::json;
#else
#    include <GLES3/gl3.h>
#    include <SDL3/SDL_main.h>
#    include <android/log.h>
#endif

using namespace std::literals;
namespace fs = std::filesystem;

static const std::unordered_map<Event::Type, std::string_view> s_eventTypeToStringView{
    { Event::Type::key_down, "key_down" },
    { Event::Type::key_up, "key_up" },
    { Event::Type::mouse_down, "button_down" },
    { Event::Type::mouse_up, "button_up" },
    { Event::Type::mouse_wheel, "spin_wheel" },
    { Event::Type::mouse_motion, "mouse_motion" },
    { Event::Type::touch_down, "touch_down" },
    { Event::Type::touch_up, "touch_up" },
    { Event::Type::touch_motion, "touch_motion" },
    { Event::Type::window_resized, "window_resized" },
    { Event::Type::turn_off, "turn_off" },
    { Event::Type::not_event, "" }
};

static const std::unordered_map<Event::Keyboard::Key, std::string_view> s_eventKeysToStringView{
    { Event::Keyboard::Key::q, "q_" },
    { Event::Keyboard::Key::w, "w_" },
    { Event::Keyboard::Key::e, "e_" },
    { Event::Keyboard::Key::r, "r_" },
    { Event::Keyboard::Key::t, "t_" },
    { Event::Keyboard::Key::y, "y_" },
    { Event::Keyboard::Key::u, "u_" },
    { Event::Keyboard::Key::i, "i_" },
    { Event::Keyboard::Key::o, "o_" },
    { Event::Keyboard::Key::p, "p_" },
    { Event::Keyboard::Key::a, "a_" },
    { Event::Keyboard::Key::s, "s_" },
    { Event::Keyboard::Key::d, "d_" },
    { Event::Keyboard::Key::f, "f_" },
    { Event::Keyboard::Key::g, "g_" },
    { Event::Keyboard::Key::h, "h_" },
    { Event::Keyboard::Key::j, "j_" },
    { Event::Keyboard::Key::k, "k_" },
    { Event::Keyboard::Key::l, "l_" },
    { Event::Keyboard::Key::z, "z_" },
    { Event::Keyboard::Key::x, "x_" },
    { Event::Keyboard::Key::c, "c_" },
    { Event::Keyboard::Key::v, "v_" },
    { Event::Keyboard::Key::b, "b_" },
    { Event::Keyboard::Key::n, "n_" },
    { Event::Keyboard::Key::m, "m_" },
    { Event::Keyboard::Key::space, "space_" },
    { Event::Keyboard::Key::enter, "enter_" },
    { Event::Keyboard::Key::l_control, "left_control_" },
    { Event::Keyboard::Key::l_shift, "left_shift_" },
    { Event::Keyboard::Key::escape, "escape_" },
    { Event::Keyboard::Key::backspace, "backspace_" },

    // Number keys
    { Event::Keyboard::Key::num_0, "num_0_" },
    { Event::Keyboard::Key::num_1, "num_1_" },
    { Event::Keyboard::Key::num_2, "num_2_" },
    { Event::Keyboard::Key::num_3, "num_3_" },
    { Event::Keyboard::Key::num_4, "num_4_" },
    { Event::Keyboard::Key::num_5, "num_5_" },
    { Event::Keyboard::Key::num_6, "num_6_" },
    { Event::Keyboard::Key::num_7, "num_7_" },
    { Event::Keyboard::Key::num_8, "num_8_" },
    { Event::Keyboard::Key::num_9, "num_9_" },

    // Function keys
    { Event::Keyboard::Key::f1, "f1_" },
    { Event::Keyboard::Key::f2, "f2_" },
    { Event::Keyboard::Key::f3, "f3_" },
    { Event::Keyboard::Key::f4, "f4_" },
    { Event::Keyboard::Key::f5, "f5_" },
    { Event::Keyboard::Key::f6, "f6_" },
    { Event::Keyboard::Key::f7, "f7_" },
    { Event::Keyboard::Key::f8, "f8_" },
    { Event::Keyboard::Key::f9, "f9_" },
    { Event::Keyboard::Key::f10, "f10_" },
    { Event::Keyboard::Key::f11, "f11_" },
    { Event::Keyboard::Key::f12, "f12_" },

    // Arrow keys
    { Event::Keyboard::Key::up_arrow, "up_arrow_" },
    { Event::Keyboard::Key::down_arrow, "down_arrow_" },
    { Event::Keyboard::Key::left_arrow, "left_arrow_" },
    { Event::Keyboard::Key::right_arrow, "right_arrow_" },

    // Miscellaneous keys
    { Event::Keyboard::Key::insert, "insert_" },
    { Event::Keyboard::Key::home, "home_" },
    { Event::Keyboard::Key::page_up, "page_up_" },
    { Event::Keyboard::Key::delete_key, "delete_key_" },
    { Event::Keyboard::Key::end, "end_" },
    { Event::Keyboard::Key::page_down, "page_down_" },
    { Event::Keyboard::Key::caps_lock, "caps_lock_" },
    { Event::Keyboard::Key::scroll_lock, "scroll_lock_" },
    { Event::Keyboard::Key::num_lock, "num_lock_" },
    { Event::Keyboard::Key::print_screen, "print_screen_" },
    { Event::Keyboard::Key::pause, "pause_" },

    // Numpad keys
    { Event::Keyboard::Key::numpad_0, "numpad_0_" },
    { Event::Keyboard::Key::numpad_1, "numpad_1_" },
    { Event::Keyboard::Key::numpad_2, "numpad_2_" },
    { Event::Keyboard::Key::numpad_3, "numpad_3_" },
    { Event::Keyboard::Key::numpad_4, "numpad_4_" },
    { Event::Keyboard::Key::numpad_5, "numpad_5_" },
    { Event::Keyboard::Key::numpad_6, "numpad_6_" },
    { Event::Keyboard::Key::numpad_7, "numpad_7_" },
    { Event::Keyboard::Key::numpad_8, "numpad_8_" },
    { Event::Keyboard::Key::numpad_9, "numpad_9_" },
    { Event::Keyboard::Key::numpad_decimal, "numpad_decimal_" },
    { Event::Keyboard::Key::numpad_divide, "numpad_divide_" },
    { Event::Keyboard::Key::numpad_multiply, "numpad_multiply_" },
    { Event::Keyboard::Key::numpad_subtract, "numpad_subtract_" },
    { Event::Keyboard::Key::numpad_add, "numpad_add_" },
    { Event::Keyboard::Key::numpad_enter, "numpad_enter_" },

    { Event::Keyboard::Key::not_key, "" }
};

static const std::unordered_map<Event::Mouse::Button, std::string_view> s_eventButtonsToStringView{
    { Event::Mouse::Button::left, "left_" },
    { Event::Mouse::Button::right, "right_" },
    { Event::Mouse::Button::middle, "middle_" },
    { Event::Mouse::Button::not_button, "" }
};

std::ostream& operator<<(std::ostream& out, const Event& event) {
    out << s_eventKeysToStringView.at(event.keyboard.key)
        << s_eventButtonsToStringView.at(event.mouse.button)
        << s_eventTypeToStringView.at(event.type);

    return out;
}

std::string_view keyToStr(Event::Keyboard::Key key) { return s_eventKeysToStringView.at(key); }

Event::Keyboard::Key ImGuiKeyToEventKey(ImGuiKey key) {
    switch (key) {
    case ImGuiKey_LeftArrow:
        return Event::Keyboard::Key::left_arrow;
    case ImGuiKey_RightArrow:
        return Event::Keyboard::Key::right_arrow;
    case ImGuiKey_UpArrow:
        return Event::Keyboard::Key::up_arrow;
    case ImGuiKey_DownArrow:
        return Event::Keyboard::Key::down_arrow;
    case ImGuiKey_KeyPadEnter:
        return Event::Keyboard::Key::numpad_enter;
    case ImGuiKey_KeypadMultiply:
        return Event::Keyboard::Key::numpad_multiply;
    case ImGuiKey_KeypadAdd:
        return Event::Keyboard::Key::numpad_add;
    case ImGuiKey_KeypadSubtract:
        return Event::Keyboard::Key::numpad_subtract;
    case ImGuiKey_KeypadDivide:
        return Event::Keyboard::Key::numpad_divide;
    case ImGuiKey_0:
        return Event::Keyboard::Key::num_0;
    case ImGuiKey_1:
        return Event::Keyboard::Key::num_1;
    case ImGuiKey_2:
        return Event::Keyboard::Key::num_2;
    case ImGuiKey_3:
        return Event::Keyboard::Key::num_3;
    case ImGuiKey_4:
        return Event::Keyboard::Key::num_4;
    case ImGuiKey_5:
        return Event::Keyboard::Key::num_5;
    case ImGuiKey_6:
        return Event::Keyboard::Key::num_6;
    case ImGuiKey_7:
        return Event::Keyboard::Key::num_7;
    case ImGuiKey_8:
        return Event::Keyboard::Key::num_8;
    case ImGuiKey_9:
        return Event::Keyboard::Key::num_9;
    case ImGuiKey_F1:
        return Event::Keyboard::Key::f1;
    case ImGuiKey_F2:
        return Event::Keyboard::Key::f2;
    case ImGuiKey_F3:
        return Event::Keyboard::Key::f3;
    case ImGuiKey_F4:
        return Event::Keyboard::Key::f4;
    case ImGuiKey_F5:
        return Event::Keyboard::Key::f5;
    case ImGuiKey_F6:
        return Event::Keyboard::Key::f6;
    case ImGuiKey_F7:
        return Event::Keyboard::Key::f7;
    case ImGuiKey_F8:
        return Event::Keyboard::Key::f8;
    case ImGuiKey_F9:
        return Event::Keyboard::Key::f9;
    case ImGuiKey_F10:
        return Event::Keyboard::Key::f10;
    case ImGuiKey_F11:
        return Event::Keyboard::Key::f11;
    case ImGuiKey_F12:
        return Event::Keyboard::Key::f12;
    case ImGuiKey_Pause:
        return Event::Keyboard::Key::pause;
    case ImGuiKey_Backspace:
        return Event::Keyboard::Key::backspace;
    case ImGuiKey_Enter:
        return Event::Keyboard::Key::enter;
    case ImGuiKey_Escape:
        return Event::Keyboard::Key::escape;
    case ImGuiKey_A:
        return Event::Keyboard::Key::a;
    case ImGuiKey_B:
        return Event::Keyboard::Key::b;
    case ImGuiKey_C:
        return Event::Keyboard::Key::c;

    case ImGuiKey_D:
        return Event::Keyboard::Key::d;
    case ImGuiKey_E:
        return Event::Keyboard::Key::e;
    case ImGuiKey_F:
        return Event::Keyboard::Key::f;
    case ImGuiKey_G:
        return Event::Keyboard::Key::g;
    case ImGuiKey_H:
        return Event::Keyboard::Key::h;
    case ImGuiKey_I:
        return Event::Keyboard::Key::i;
    case ImGuiKey_J:
        return Event::Keyboard::Key::j;
    case ImGuiKey_K:
        return Event::Keyboard::Key::k;
    case ImGuiKey_L:
        return Event::Keyboard::Key::l;
    case ImGuiKey_M:
        return Event::Keyboard::Key::m;
    case ImGuiKey_N:
        return Event::Keyboard::Key::n;
    case ImGuiKey_O:
        return Event::Keyboard::Key::o;
    case ImGuiKey_P:
        return Event::Keyboard::Key::p;
    case ImGuiKey_Q:
        return Event::Keyboard::Key::q;
    case ImGuiKey_R:
        return Event::Keyboard::Key::r;
    case ImGuiKey_S:
        return Event::Keyboard::Key::s;
    case ImGuiKey_T:
        return Event::Keyboard::Key::t;
    case ImGuiKey_U:
        return Event::Keyboard::Key::u;
    case ImGuiKey_V:
        return Event::Keyboard::Key::v;
    case ImGuiKey_W:
        return Event::Keyboard::Key::w;
    case ImGuiKey_X:
        return Event::Keyboard::Key::x;
    case ImGuiKey_Y:
        return Event::Keyboard::Key::y;
    case ImGuiKey_Z:
        return Event::Keyboard::Key::z;
    case ImGuiKey_Space:
        return Event::Keyboard::Key::space;
    case ImGuiKey_Insert:
        return Event::Keyboard::Key::insert;
    case ImGuiKey_Home:
        return Event::Keyboard::Key::home;
    case ImGuiKey_PageUp:
        return Event::Keyboard::Key::page_up;
    case ImGuiKey_Delete:
        return Event::Keyboard::Key::delete_key;
    case ImGuiKey_End:
        return Event::Keyboard::Key::end;
    case ImGuiKey_PageDown:
        return Event::Keyboard::Key::page_down;
    case ImGuiKey_CapsLock:
        return Event::Keyboard::Key::caps_lock;
    case ImGuiKey_ScrollLock:
        return Event::Keyboard::Key::scroll_lock;
    case ImGuiKey_NumLock:
        return Event::Keyboard::Key::num_lock;
    case ImGuiKey_PrintScreen:
        return Event::Keyboard::Key::print_screen;
    case ImGuiKey_LeftShift:
        return Event::Keyboard::Key::l_shift;
    case ImGuiKey_LeftCtrl:
        return Event::Keyboard::Key::l_control;
    default:
        return Event::Keyboard::Key::not_key;
    }
}

std::ifstream& operator>>(std::ifstream& in, Triangle& triangle) {
    for (auto& vertex : triangle.vertices)
        in >> vertex;

    return in;
}

std::ifstream& operator>>(std::ifstream& in, Triangle2& triangle2) {
    for (auto& vertex : triangle2.vertices)
        in >> vertex;

    return in;
}

static std::optional<Event> checkKeyboardInput(SDL_Event& sdlEvent) {
    static const std::unordered_map<SDL_Keycode, Event::Keyboard::Key> keymap{
        { SDLK_q, Event::Keyboard::Key::q },
        { SDLK_w, Event::Keyboard::Key::w },
        { SDLK_e, Event::Keyboard::Key::e },
        { SDLK_r, Event::Keyboard::Key::r },
        { SDLK_t, Event::Keyboard::Key::t },
        { SDLK_y, Event::Keyboard::Key::y },
        { SDLK_u, Event::Keyboard::Key::u },
        { SDLK_i, Event::Keyboard::Key::i },
        { SDLK_o, Event::Keyboard::Key::o },
        { SDLK_p, Event::Keyboard::Key::p },
        { SDLK_a, Event::Keyboard::Key::a },
        { SDLK_s, Event::Keyboard::Key::s },
        { SDLK_d, Event::Keyboard::Key::d },
        { SDLK_f, Event::Keyboard::Key::f },
        { SDLK_g, Event::Keyboard::Key::g },
        { SDLK_h, Event::Keyboard::Key::h },
        { SDLK_j, Event::Keyboard::Key::j },
        { SDLK_k, Event::Keyboard::Key::k },
        { SDLK_l, Event::Keyboard::Key::l },
        { SDLK_z, Event::Keyboard::Key::z },
        { SDLK_x, Event::Keyboard::Key::x },
        { SDLK_c, Event::Keyboard::Key::c },
        { SDLK_v, Event::Keyboard::Key::v },
        { SDLK_b, Event::Keyboard::Key::b },
        { SDLK_n, Event::Keyboard::Key::n },
        { SDLK_m, Event::Keyboard::Key::m },
        { SDLK_SPACE, Event::Keyboard::Key::space },
        { SDLK_RETURN, Event::Keyboard::Key::enter },
        { SDLK_LCTRL, Event::Keyboard::Key::l_control },
        { SDLK_LSHIFT, Event::Keyboard::Key::l_shift },
        { SDLK_ESCAPE, Event::Keyboard::Key::escape },
        { SDLK_BACKSPACE, Event::Keyboard::Key::backspace },

        // Number keys
        { SDLK_0, Event::Keyboard::Key::num_0 },
        { SDLK_1, Event::Keyboard::Key::num_1 },
        { SDLK_2, Event::Keyboard::Key::num_2 },
        { SDLK_3, Event::Keyboard::Key::num_3 },
        { SDLK_4, Event::Keyboard::Key::num_4 },
        { SDLK_5, Event::Keyboard::Key::num_5 },
        { SDLK_6, Event::Keyboard::Key::num_6 },
        { SDLK_7, Event::Keyboard::Key::num_7 },
        { SDLK_8, Event::Keyboard::Key::num_8 },
        { SDLK_9, Event::Keyboard::Key::num_9 },

        // Function keys
        { SDLK_F1, Event::Keyboard::Key::f1 },
        { SDLK_F2, Event::Keyboard::Key::f2 },
        { SDLK_F3, Event::Keyboard::Key::f3 },
        { SDLK_F4, Event::Keyboard::Key::f4 },
        { SDLK_F5, Event::Keyboard::Key::f5 },
        { SDLK_F6, Event::Keyboard::Key::f6 },
        { SDLK_F7, Event::Keyboard::Key::f7 },
        { SDLK_F8, Event::Keyboard::Key::f8 },
        { SDLK_F9, Event::Keyboard::Key::f9 },
        { SDLK_F10, Event::Keyboard::Key::f10 },
        { SDLK_F11, Event::Keyboard::Key::f11 },
        { SDLK_F12, Event::Keyboard::Key::f12 },

        // Arrow keys
        { SDLK_UP, Event::Keyboard::Key::up_arrow },
        { SDLK_DOWN, Event::Keyboard::Key::down_arrow },
        { SDLK_LEFT, Event::Keyboard::Key::left_arrow },
        { SDLK_RIGHT, Event::Keyboard::Key::right_arrow },

        // Miscellaneous keys
        { SDLK_INSERT, Event::Keyboard::Key::insert },
        { SDLK_HOME, Event::Keyboard::Key::home },
        { SDLK_PAGEUP, Event::Keyboard::Key::page_up },
        { SDLK_DELETE, Event::Keyboard::Key::delete_key },
        { SDLK_END, Event::Keyboard::Key::end },
        { SDLK_PAGEDOWN, Event::Keyboard::Key::page_down },
        { SDLK_CAPSLOCK, Event::Keyboard::Key::caps_lock },
        { SDLK_SCROLLLOCK, Event::Keyboard::Key::scroll_lock },
        { SDLK_NUMLOCKCLEAR, Event::Keyboard::Key::num_lock },
        { SDLK_PRINTSCREEN, Event::Keyboard::Key::print_screen },
        { SDLK_PAUSE, Event::Keyboard::Key::pause },

        // Numpad keys
        { SDLK_KP_0, Event::Keyboard::Key::numpad_0 },
        { SDLK_KP_1, Event::Keyboard::Key::numpad_1 },
        { SDLK_KP_2, Event::Keyboard::Key::numpad_2 },
        { SDLK_KP_3, Event::Keyboard::Key::numpad_3 },
        { SDLK_KP_4, Event::Keyboard::Key::numpad_4 },
        { SDLK_KP_5, Event::Keyboard::Key::numpad_5 },
        { SDLK_KP_6, Event::Keyboard::Key::numpad_6 },
        { SDLK_KP_7, Event::Keyboard::Key::numpad_7 },
        { SDLK_KP_8, Event::Keyboard::Key::numpad_8 },
        { SDLK_KP_9, Event::Keyboard::Key::numpad_9 },
        { SDLK_KP_PERIOD, Event::Keyboard::Key::numpad_decimal },
        { SDLK_KP_DIVIDE, Event::Keyboard::Key::numpad_divide },
        { SDLK_KP_MULTIPLY, Event::Keyboard::Key::numpad_multiply },
        { SDLK_KP_MINUS, Event::Keyboard::Key::numpad_subtract },
        { SDLK_KP_PLUS, Event::Keyboard::Key::numpad_add },
        { SDLK_KP_ENTER, Event::Keyboard::Key::numpad_enter },

        { SDLK_UNKNOWN, Event::Keyboard::Key::not_key }
    };

    if (auto found{ keymap.find(sdlEvent.key.keysym.sym) }; found != keymap.end()) {
        Event event{};
        event.keyboard.key = found->second;
        event.mouse.pos.x = sdlEvent.motion.x;
        event.mouse.pos.y = sdlEvent.motion.y;

        if (sdlEvent.type == SDL_EVENT_KEY_DOWN)
            event.type = Event::Type::key_down;
        else if (sdlEvent.type == SDL_EVENT_KEY_UP)
            event.type = Event::Type::key_up;
        else
            return std::nullopt;

        return event;
    }

    return std::nullopt;
}

static std::optional<Event> checkMouseInput(SDL_Event& sdlEvent) {
    static const std::unordered_map<int, Event::Mouse::Button> mousemap{
        { SDL_BUTTON_LEFT, Event::Mouse::Button::left },
        { SDL_BUTTON_RIGHT, Event::Mouse::Button::right },
        { SDL_BUTTON_MIDDLE, Event::Mouse::Button::middle }
    };

    Event event{};
    event.mouse.pos.x = sdlEvent.motion.x;
    event.mouse.pos.y = sdlEvent.motion.y;

    if (sdlEvent.type == SDL_EVENT_MOUSE_BUTTON_DOWN) {
        if (auto found{ mousemap.find(sdlEvent.button.button) }; found != mousemap.end()) {
            event.mouse.button = found->second;
            event.type = Event::Type::mouse_down;
            return event;
        }
    }

    if (sdlEvent.type == SDL_EVENT_MOUSE_BUTTON_UP) {
        if (auto found{ mousemap.find(sdlEvent.button.button) }; found != mousemap.end()) {
            event.mouse.button = found->second;
            event.type = Event::Type::mouse_up;
            return event;
        }
    }

    if (sdlEvent.type == SDL_EVENT_MOUSE_MOTION) {
        event.type = Event::Type::mouse_motion;
        return event;
    }

    if (sdlEvent.type == SDL_EVENT_MOUSE_WHEEL) {
        event.type = Event::Type::mouse_wheel;
        event.mouse.wheel.x = sdlEvent.wheel.x;
        event.mouse.wheel.y = sdlEvent.wheel.y;
        return event;
    }

    return std::nullopt;
}

static std::optional<Event> checkTouchInput(SDL_Event& sdlEvent) {
    Event event{};

    static std::unordered_map<std::int64_t, std::pair<float, float>> fingersMap{};

    switch (sdlEvent.type) {
    case SDL_EVENT_FINGER_DOWN:
        event.type = Event::Type::touch_down;
        fingersMap[sdlEvent.tfinger.fingerId].first =
            sdlEvent.tfinger.x * static_cast<float>(getEngineInstance()->getWindowSize().width);
        fingersMap[sdlEvent.tfinger.fingerId].second =
            (1.0f - sdlEvent.tfinger.y) *
            static_cast<float>(getEngineInstance()->getWindowSize().height);
        break;
    case SDL_EVENT_FINGER_UP:
        event.type = Event::Type::touch_up;
        break;
    case SDL_EVENT_FINGER_MOTION:
        event.type = Event::Type::touch_motion;
        break;
    default:
        event.type = Event::Type::not_event;
    }

    event.touch.id = sdlEvent.tfinger.fingerId;
    event.touch.pos.x =
        sdlEvent.tfinger.x * static_cast<float>(getEngineInstance()->getWindowSize().width);
    event.touch.pos.y = (1.0f - sdlEvent.tfinger.y) *
                        static_cast<float>(getEngineInstance()->getWindowSize().height);

    if (event.type == Event::Type::touch_motion) {
        event.touch.dx = event.touch.pos.x - fingersMap[sdlEvent.tfinger.fingerId].first;
        event.touch.dy = event.touch.pos.y - fingersMap[sdlEvent.tfinger.fingerId].second;
    }

    if (event.type == Event::Type::not_event) return std::nullopt;
    return event;
}

static std::string readFile(const fs::path& path) {
    std::ifstream in{ path };
    if (!in.is_open()) throw std::runtime_error{ "Error : readFile : bad open file"s };

    std::string result{};
    while (in) {
        std::string buf{};
        std::getline(in, buf);
        if (!in) break;
        result += buf;
        result += '\n';
    }

    return result;
}

static std::mutex g_audioMutex{};

class EngineImpl final : public IEngine
{
public:
    friend class Audio;

private:
    SDL_Window* m_window{};
    SDL_GLContext m_glContext{};
    GLuint m_verticesArray{};

    SDL_AudioSpec m_audioSpec{};
    SDL_AudioDeviceID m_audioDevice{};
    std::string m_currentAudioDeviceName{};
    int m_audioVolume{ SDL_MIX_MAXVOLUME };

    ShaderProgram m_shaderProgram{};
    ShaderProgram m_shaderProgramWithView{};

    std::reference_wrapper<ShaderProgram> m_program{ m_shaderProgram };

    std::vector<std::reference_wrapper<Audio>> m_sounds{};

    int m_framerate{ 150 };

public:
    EngineImpl() = default;

    ~EngineImpl() override = default;

    std::string initialize(std::string_view config) override;

    void uninitialize() override;

    bool readInput(Event& event) override;

    void swapBuffers() override;

    void recompileShaders() override;

    void render(const VertexBuffer<Vertex2>& vertexBuffer,
                const IndexBuffer<std::uint16_t>& indexBuffer,
                const Texture& texture) override;

    void render(const VertexBuffer<Vertex2>& vertexBuffer,
                const IndexBuffer<std::uint32_t>& indexBuffer,
                const Texture& texture) override;

    void render(const VertexBuffer<Vertex2>& vertexBuffer,
                const IndexBuffer<std::uint32_t>& indexBuffer,
                const Texture& texture,
                const glm::mat3& matrix) override;

    void render(const VertexBuffer<Vertex2>& vertexBuffer,
                const IndexBuffer<std::uint32_t>& indexBuffer,
                const Texture& texture,
                const glm::mat3& matrix,
                const View& view) override;

    void render(const Sprite& sprite) override;

    void render(const Sprite& sprite, const View& view) override;

    [[nodiscard]] WindowSize getWindowSize() const noexcept override {
        int width{};
        int height{};
        SDL_GetWindowSize(m_window, &width, &height);
        return { width, height };
    }

    void setVSync(bool isEnable) override { SDL_GL_SetSwapInterval(isEnable); }

    [[nodiscard]] bool getVSync() const noexcept override {
        int isVSync{};
        SDL_GL_GetSwapInterval(&isVSync);
        return isVSync;
    }

    void setFramerate(int framerate) override { m_framerate = framerate; }

    [[nodiscard]] int getFramerate() const noexcept override { return m_framerate; }

    [[nodiscard]] ImGuiContext* getImGuiContext() const noexcept override {
        return ImGui::GetCurrentContext();
    }

    [[nodiscard]] std::vector<std::string> getAudioDeviceNames() const noexcept override;
    [[nodiscard]] const std::string& getCurrentAudioDeviceName() const noexcept override;
    void setAudioDevice(std::string_view audioDeviceName) override;

    [[nodiscard]] int getAudioVolume() const noexcept override;
    void setAudioVolume(int audioVolume) override;

    [[nodiscard]] bool isFullscreen() const noexcept override;
    void setFullscreen(bool isFullscreen) override;

private:
    static void initSDL() {
        if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS | SDL_INIT_AUDIO | SDL_INIT_GAMEPAD |
                     SDL_INIT_TIMER) != 0)
            throw std::runtime_error{ "Error : failed call SDL_Init: "s + SDL_GetError() };
    }

    static SDL_Window*
    createWindow(std::string_view name, int width, int height, SDL_WindowFlags flags) {
        if (auto window{ SDL_CreateWindow(name.data(), width, height, flags) }; window != nullptr)
            return window;

        SDL_Quit();
        throw std::runtime_error{ "Error : failed call SDL_CreateWindow: "s + SDL_GetError() };
    }

    [[maybe_unused]] static SDL_Renderer* createRenderer(SDL_Window* window) {
        if (auto renderer{ SDL_CreateRenderer(window, "opengl", SDL_RENDERER_ACCELERATED) };
            renderer != nullptr)
            return renderer;

        SDL_DestroyWindow(window);
        SDL_Quit();
        throw std::runtime_error{ "Error : failed call SDL_CreateRenderer: "s + SDL_GetError() };
    }

    void createGLContext() {
        const std::string_view platform{ SDL_GetPlatform() };

        int gl_major_ver{ 3 };
        int gl_minor_ver{ 0 };
        int gl_context_profile{ SDL_GL_CONTEXT_PROFILE_ES };
        std::string glslVersion{ "#version 300 es" };

        if (platform == "macOS") {
            gl_major_ver = 4;
            gl_minor_ver = 1;
            gl_context_profile = SDL_GL_CONTEXT_PROFILE_CORE;
            glslVersion = "#version 410 core"s;
        }

        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, gl_context_profile);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, gl_major_ver);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, gl_minor_ver);
        ShaderProgram::setGLSLVersion(glslVersion);

        m_glContext = SDL_GL_CreateContext(m_window);
        if (m_glContext == nullptr)
            throw std::runtime_error{ "Error : createGLContext : bad create gl context"s };

        auto load_gl_pointer = [](const char* functionName) {
            auto functionPointer{ SDL_GL_GetProcAddress(functionName) };
            return reinterpret_cast<void*>(functionPointer);
        };

        if (gladLoadGLES2Loader(load_gl_pointer) == 0)
            throw std::runtime_error{ "Error : createGLContext : bad gladLoad"s };
    }

    static void audioCallback(void* engine_ptr, std::uint8_t* stream, int streamSize);
};

std::string EngineImpl::initialize(std::string_view config) {
#ifdef __ANDROID__
    if (m_window) return "";
#endif

    initSDL();

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS,
                        SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG); // Always required on Mac
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);

#ifndef __ANDROID__

    auto jsonValue(json::parse(config));

    auto windowName{ jsonValue.as_object().contains("window_name")
                         ? jsonValue.as_object().at("window_name").as_string()
                         : "SDL + OPENGL"sv };

    auto width{ jsonValue.as_object().contains("window_width")
                    ? jsonValue.as_object().at("window_width").as_int64()
                    : 800 };

    auto height{ jsonValue.as_object().contains("window_height")
                     ? jsonValue.as_object().at("window_height").as_int64()
                     : 600 };

    auto isWindowResizable{ jsonValue.as_object().contains("is_window_resizable") &&
                            jsonValue.as_object().at("is_window_resizable").as_bool() };

    auto minWidth{ jsonValue.as_object().contains("window_min_width")
                       ? jsonValue.as_object().at("window_min_width").as_int64()
                       : 640 };

    auto minHeight{ jsonValue.as_object().contains("window_min_height")
                        ? jsonValue.as_object().at("window_min_height").as_int64()
                        : 480 };

    int flags{};
    flags |= SDL_WINDOW_OPENGL | SDL_WINDOW_HIDDEN;
    if (isWindowResizable) flags |= SDL_WINDOW_RESIZABLE;

    m_window = createWindow(windowName,
                            static_cast<int>(width),
                            static_cast<int>(height),
                            static_cast<SDL_WindowFlags>(flags));

    SDL_SetWindowPosition(m_window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
    SDL_SetWindowMinimumSize(m_window, static_cast<int>(minWidth), static_cast<int>(minHeight));
    SDL_ShowWindow(m_window);

#else
    const SDL_DisplayMode* displayMode{ SDL_GetCurrentDisplayMode(1) };
    m_window = createWindow("android", displayMode->w, displayMode->h, SDL_WINDOW_OPENGL);
#endif

    createGLContext();

    glEnable(GL_DEPTH_TEST);
    openGLCheck();

    glEnable(GL_BLEND);
    openGLCheck();

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    openGLCheck();

    glGenVertexArrays(1, &m_verticesArray);
    openGLCheck();

    glBindVertexArray(m_verticesArray);
    openGLCheck();

    m_audioSpec.freq = 48000;
    m_audioSpec.format = SDL_AUDIO_S16LSB;
    m_audioSpec.channels = 2;
    m_audioSpec.samples = 512;
    m_audioSpec.callback = audioCallback;
    m_audioSpec.userdata = this;

    std::string defaultAudioDeviceName{};
    const int numAudioDevices{ SDL_GetNumAudioDevices(SDL_FALSE) };
    if (numAudioDevices > 0)
        defaultAudioDeviceName = SDL_GetAudioDeviceName(numAudioDevices - 1, 0);

    m_currentAudioDeviceName = defaultAudioDeviceName;

    m_audioDevice = SDL_OpenAudioDevice(defaultAudioDeviceName.c_str(),
                                        SDL_FALSE,
                                        &m_audioSpec,
                                        &m_audioSpec,
                                        SDL_AUDIO_ALLOW_ANY_CHANGE);

    if (m_audioDevice == 0)
        throw std::runtime_error{ "Error : EngineImpl::initialize : failed open audio device: "s +
                                  SDL_GetError() };

    std::cout << "audio device selected: "sv << defaultAudioDeviceName << '\n'
              << "freq: "sv << m_audioSpec.freq << '\n'
              << "format: "sv << m_audioSpec.format << '\n'
              << "channels: "sv << static_cast<int>(m_audioSpec.channels) << '\n'
              << "samples: "sv << m_audioSpec.samples << '\n'
              << std::flush;

    SDL_PlayAudioDevice(m_audioDevice);

    recompileShaders();
    SDL_GL_SetSwapInterval(1);

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    ( void )io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;  // Enable Gamepad Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

    // Setup Platform/Renderer backends
    ImGui_ImplSDL3_InitForOpenGL(m_window, m_glContext);
#ifdef __APPLE__
    const char* glsl_version = "#version 150";
#else
    const char* glsl_version = "#version 300 es";
#endif
    ImGui_ImplOpenGL3_Init(glsl_version);

    return "";
}

void EngineImpl::uninitialize() {
    SDL_CloseAudioDevice(m_audioDevice);
    glDeleteVertexArrays(1, &m_verticesArray);
    openGLCheck();

    m_shaderProgram.clear();
    m_shaderProgramWithView.clear();

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();

    if (m_glContext) SDL_GL_DeleteContext(m_glContext);
    if (m_window) SDL_DestroyWindow(m_window);

    SDL_Quit();
}

bool EngineImpl::readInput(Event& event) {
    SDL_Event sdlEvent;
    if (SDL_PollEvent(&sdlEvent)) {
        ImGui_ImplSDL3_ProcessEvent(&sdlEvent);

        switch (sdlEvent.type) {
        case SDL_EVENT_QUIT:
            event.type = Event::Type::turn_off;
            return true;

        case SDL_EVENT_KEY_DOWN:
        case SDL_EVENT_KEY_UP:
            if (auto e{ checkKeyboardInput(sdlEvent) }) {
                event = *e;
                return true;
            }
            break;

        case SDL_EVENT_MOUSE_MOTION:
        case SDL_EVENT_MOUSE_BUTTON_DOWN:
        case SDL_EVENT_MOUSE_BUTTON_UP:
        case SDL_EVENT_MOUSE_WHEEL:
            if (auto e{ checkMouseInput(sdlEvent) }) {
                event = *e;
                return true;
            }
            break;

        case SDL_EVENT_WINDOW_RESIZED:
            event.type = Event::Type::window_resized;
            return true;

        case SDL_EVENT_FINGER_DOWN:
        case SDL_EVENT_FINGER_UP:
        case SDL_EVENT_FINGER_MOTION:
            if (auto e{ checkTouchInput(sdlEvent) }) {
                event = *e;
                return true;
            }
            break;
        }
    }

    return false;
}

void EngineImpl::swapBuffers() {
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    int width{}, height{};
    SDL_GetWindowSizeInPixels(m_window, &width, &height);
    glViewport(0, 0, width, height);
    openGLCheck();

    SDL_GL_SwapWindow(m_window);

    glClearColor(0.0f, 0.0f, 0.f, 1.f);
    openGLCheck();

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    openGLCheck();
}

void EngineImpl::recompileShaders() {
#ifndef __ANDROID__
    m_shaderProgram.recompileShaders(
        HotReloadProvider::getInstance().getPath("vertex_shader_without_view"),
        HotReloadProvider::getInstance().getPath("fragment_shader"));

    m_shaderProgramWithView.recompileShaders(
        HotReloadProvider::getInstance().getPath("vertex_shader_with_view"),
        HotReloadProvider::getInstance().getPath("fragment_shader"));
#else
    m_shaderProgram.recompileShaders("data/shaders/vertex_shader_without_view.vert",
                                     "data/shaders/fragment_shader.frag");

    m_shaderProgramWithView.recompileShaders("data/shaders/vertex_shader_with_view.vert",
                                             "data/shaders/fragment_shader.frag");
#endif
    m_program.get().use();
}

void EngineImpl::render(const VertexBuffer<Vertex2>& vertexBuffer,
                        const IndexBuffer<std::uint16_t>& indexBuffer,
                        const Texture& texture) {
    m_program.get().use();
    m_program.get().setUniform("texSampler", texture);

    texture.bind();
    vertexBuffer.bind();
    indexBuffer.bind();

    glEnableVertexAttribArray(0);
    openGLCheck();

    glEnableVertexAttribArray(1);
    openGLCheck();

    glEnableVertexAttribArray(2);
    openGLCheck();

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex2), nullptr);
    openGLCheck();

    glVertexAttribPointer(1,
                          2,
                          GL_FLOAT,
                          GL_FALSE,
                          sizeof(Vertex2),
                          reinterpret_cast<const GLvoid*>(offsetof(Vertex2, texX)));
    openGLCheck();

    glVertexAttribPointer(2,
                          4,
                          GL_UNSIGNED_BYTE,
                          GL_FALSE,
                          sizeof(Vertex2),
                          reinterpret_cast<const GLvoid*>(offsetof(Vertex2, rgba)));
    openGLCheck();

    glDrawElements(
        GL_TRIANGLES, static_cast<GLsizei>(indexBuffer.size()), GL_UNSIGNED_SHORT, nullptr);
    openGLCheck();

    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(2);
}

void EngineImpl::render(const VertexBuffer<Vertex2>& vertexBuffer,
                        const IndexBuffer<std::uint32_t>& indexBuffer,
                        const Texture& texture) {
    m_program.get().use();
    m_program.get().setUniform("texSampler", texture);

    texture.bind();
    vertexBuffer.bind();
    indexBuffer.bind();

    glEnableVertexAttribArray(0);
    openGLCheck();

    glEnableVertexAttribArray(1);
    openGLCheck();

    glEnableVertexAttribArray(2);
    openGLCheck();

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex2), nullptr);
    openGLCheck();

    glVertexAttribPointer(1,
                          2,
                          GL_FLOAT,
                          GL_FALSE,
                          sizeof(Vertex2),
                          reinterpret_cast<const GLvoid*>(offsetof(Vertex2, texX)));
    openGLCheck();

    glVertexAttribPointer(2,
                          4,
                          GL_UNSIGNED_BYTE,
                          GL_FALSE,
                          sizeof(Vertex2),
                          reinterpret_cast<const GLvoid*>(offsetof(Vertex2, rgba)));
    openGLCheck();

    glDrawElements(
        GL_TRIANGLES, static_cast<GLsizei>(indexBuffer.size()), GL_UNSIGNED_INT, nullptr);
    openGLCheck();

    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(2);
}

void EngineImpl::render(const VertexBuffer<Vertex2>& vertexBuffer,
                        const IndexBuffer<std::uint32_t>& indexBuffer,
                        const Texture& texture,
                        const glm::mat3& matrix) {
    m_program.get().use();
    m_program.get().setUniform("matrix", matrix);
    render(vertexBuffer, indexBuffer, texture);
}

void EngineImpl::render(const VertexBuffer<Vertex2>& vertexBuffer,
                        const IndexBuffer<std::uint32_t>& indexBuffer,
                        const Texture& texture,
                        const glm::mat3& matrix,
                        const View& view) {
    ShaderProgram& lastProgram{ m_program.get() };
    m_program = m_shaderProgramWithView;
    m_program.get().use();
    m_program.get().setUniform("viewMatrix", view.getViewMatrix());
    render(vertexBuffer, indexBuffer, texture, matrix);
    m_program = lastProgram;
}

void EngineImpl::render(const Sprite& sprite) {
    m_program.get().use();
    m_program.get().setUniform("matrix", sprite.getResultMatrix());

    VertexBuffer vertexBuffer{ sprite.getVertices() };
    IndexBuffer indexBuffer{ sprite.getIndices() };

    render(vertexBuffer, indexBuffer, sprite.getTexture());
}

void EngineImpl::render(const Sprite& sprite, const View& view) {
    ShaderProgram& lastProgram{ m_program.get() };
    m_program = m_shaderProgramWithView;
    m_program.get().use();
    m_program.get().setUniform("viewMatrix", view.getViewMatrix());
    render(sprite);
    m_program = lastProgram;
}

void EngineImpl::audioCallback(void* engine_ptr, std::uint8_t* stream, int streamSize) {
    std::lock_guard lock{ g_audioMutex };
    auto engine{ static_cast<EngineImpl*>(engine_ptr) };

    std::fill_n(stream, streamSize, '\0');

    for (Audio& sound : engine->m_sounds) {
        if (sound.m_isPlaying) {
            std::uint32_t rest{ sound.m_size - sound.m_currentPos };
            std::uint8_t* current_buf{ sound.m_start + sound.m_currentPos };

            if (rest <= static_cast<std::uint32_t>(streamSize)) {
                SDL_MixAudioFormat(
                    stream, current_buf, engine->m_audioSpec.format, rest, engine->m_audioVolume);
                sound.m_currentPos += rest;
            }
            else {
                SDL_MixAudioFormat(stream,
                                   current_buf,
                                   engine->m_audioSpec.format,
                                   streamSize,
                                   engine->m_audioVolume);
                sound.m_currentPos += streamSize;
            }

            if (sound.m_currentPos == sound.m_size) {
                if (sound.m_isLooped)
                    sound.m_currentPos = 0;
                else
                    sound.m_isPlaying = false;
            }
        }
    }
}

std::vector<std::string> EngineImpl::getAudioDeviceNames() const noexcept {
    std::vector<std::string> names{};
    const int count{ SDL_GetNumAudioDevices(SDL_FALSE) };
    for (int i{}; i < count; ++i)
        names.emplace_back(SDL_GetAudioDeviceName(i, SDL_FALSE));

    return names;
}

const std::string& EngineImpl::getCurrentAudioDeviceName() const noexcept {
    return m_currentAudioDeviceName;
}

void EngineImpl::setAudioDevice(std::string_view audioDeviceName) {
    SDL_CloseAudioDevice(m_audioDevice);
    m_audioDevice =
        SDL_OpenAudioDevice(audioDeviceName.data(), SDL_FALSE, &m_audioSpec, &m_audioSpec, 0);

    if (m_audioDevice == 0)
        throw std::runtime_error{ "Error : setAudioDevice : can't open audio device"s };
    m_currentAudioDeviceName = audioDeviceName;

    std::cout << "audio device selected: "sv << m_currentAudioDeviceName << '\n'
              << "freq: "sv << m_audioSpec.freq << '\n'
              << "format: "sv << m_audioSpec.format << '\n'
              << "channels: "sv << static_cast<int>(m_audioSpec.channels) << '\n'
              << "samples: "sv << m_audioSpec.samples << '\n'
              << std::flush;

    std::lock_guard lock{ g_audioMutex };
    for (Audio& sound : m_sounds) {
        std::uint8_t* newStart{};
        int newSize{};
        int convertStatus{ SDL_ConvertAudioSamples(sound.m_format,
                                                   sound.m_channels,
                                                   sound.m_freq,
                                                   sound.m_start,
                                                   sound.m_size,
                                                   m_audioSpec.format,
                                                   m_audioSpec.channels,
                                                   m_audioSpec.freq,
                                                   &newStart,
                                                   &newSize) };
        if (convertStatus != 0)
            throw std::runtime_error{ "Error : setAudioDevice : failed convert audio"s };

        SDL_free(sound.m_start);
        sound.m_start = newStart;
        sound.m_size = newSize;
        sound.m_format = m_audioSpec.format;
        sound.m_channels = m_audioSpec.channels;
        sound.m_freq = m_audioSpec.freq;
    }

    SDL_PlayAudioDevice(m_audioDevice);
}

int EngineImpl::getAudioVolume() const noexcept { return m_audioVolume; }

void EngineImpl::setAudioVolume(int audioVolume) {
    if (audioVolume < 0 || audioVolume > 128)
        throw std::runtime_error{ "Error : setAudioVolume : volume should be in range [0, 128] "s };
    m_audioVolume = audioVolume;
}

bool EngineImpl::isFullscreen() const noexcept {
    return SDL_WINDOW_FULLSCREEN & SDL_GetWindowFlags(m_window);
}

void EngineImpl::setFullscreen(bool isFullscreen) {
    SDL_SetWindowFullscreen(m_window, isFullscreen ? SDL_TRUE : SDL_FALSE);
}

static bool g_alreadyExist{ false };
static EnginePtr g_engine{};

void createEngine() {
    if (g_alreadyExist) throw std::runtime_error{ "Error : engine already exist"s };
    g_alreadyExist = true;
    g_engine = { new EngineImpl{}, destroyEngine };
}

void destroyEngine(IEngine* e) {
    if (!g_alreadyExist) throw std::runtime_error{ "Error : engine not exist"s };
    if (e == nullptr) throw std::runtime_error{ "Error : nullptr"s };
    delete e;
    g_alreadyExist = false;
}

const EnginePtr& getEngineInstance() {
    if (!g_alreadyExist) throw std::runtime_error{ "Error : engine not exist"s };
    return g_engine;
}

Audio::Audio(const fs::path& path) {
#ifndef __WIN32__
    SDL_RWops* file{ SDL_RWFromFile(path.c_str(), "rb") };
#else
    SDL_RWops* file{ SDL_RWFromFile(path.string().c_str(), "rb") };
#endif
    if (file == nullptr) throw std::runtime_error{ "Error : Audio : failed read file"s };

    SDL_AudioSpec fileAudioSpec;
    if (SDL_LoadWAV_RW(file, SDL_TRUE, &fileAudioSpec, &m_start, &m_size) == nullptr)
        throw std::runtime_error{ "Error : Audio : failed load wav"s };
    m_format = fileAudioSpec.format;
    m_channels = fileAudioSpec.channels;
    m_freq = fileAudioSpec.freq;

    auto& engine{ dynamic_cast<EngineImpl&>(*getEngineInstance().get()) };
    auto& requiredAudioSpec{ engine.m_audioSpec };

    if (engine.m_audioSpec.freq != fileAudioSpec.freq ||
        engine.m_audioSpec.format != fileAudioSpec.format ||
        engine.m_audioSpec.channels != fileAudioSpec.channels) {
        std::uint8_t* newStart{};
        int newSize{};
        int convertStatus{ SDL_ConvertAudioSamples(fileAudioSpec.format,
                                                   fileAudioSpec.channels,
                                                   fileAudioSpec.freq,
                                                   m_start,
                                                   m_size,
                                                   requiredAudioSpec.format,
                                                   requiredAudioSpec.channels,
                                                   requiredAudioSpec.freq,
                                                   &newStart,
                                                   &newSize) };
        if (convertStatus != 0) throw std::runtime_error{ "Error : Audio : failed convert audio"s };
        SDL_free(m_start);
        m_start = newStart;
        m_size = newSize;
        m_format = requiredAudioSpec.format;
        m_channels = requiredAudioSpec.channels;
        m_freq = requiredAudioSpec.freq;
    }
    std::lock_guard lock{ g_audioMutex };
    engine.m_sounds.emplace_back(*this);
}

Audio::~Audio() {
    if (m_start) SDL_free(m_start);
}

void Audio::play(bool isLooped) {
    std::lock_guard lock{ g_audioMutex };
    m_currentPos = 0;
    m_isPlaying = true;
    m_isLooped = isLooped;
}

#ifndef __ANDROID__
static std::unique_ptr<IGame, std::function<void(IGame* game)>>
reloadGame(std::unique_ptr<IGame, std::function<void(IGame* game)>> oldGame,
           std::string_view libraryName,
           std::string_view tempLibraryName,
           IEngine& engine,
           void*& oldHandle) {
    if (oldGame) {
        oldGame.reset(nullptr);
        SDL_UnloadObject(oldHandle);
    }

    fs::remove(tempLibraryName);
    fs::copy(libraryName, tempLibraryName);

    auto gameHandle{ SDL_LoadObject(tempLibraryName.data()) };
    if (gameHandle == nullptr) {
        std::cerr << "Failed SDL_LoadObject\n"sv;
        return nullptr;
    }

    oldHandle = gameHandle;

    auto createGameFuncPtr{ SDL_LoadFunction(gameHandle, "createGame") };
    if (createGameFuncPtr == nullptr) {
        std::cerr << "Failed : SDL_LoadFunction : createGame\n"sv;
        return nullptr;
    }

    using CreateGame = decltype(&createGame);
    auto createGameLinked{ reinterpret_cast<CreateGame>(createGameFuncPtr) };

    auto destroyGameFuncPtr{ SDL_LoadFunction(gameHandle, "destroyGame") };
    if (destroyGameFuncPtr == nullptr) {
        std::cerr << "Failed : SDL_LoadFunction : destroyGame\n"sv;
        return nullptr;
    }

    using DestroyGame = decltype(&destroyGame);
    auto destroyGameLinked{ reinterpret_cast<DestroyGame>(destroyGameFuncPtr) };

    return { createGameLinked(&engine), [destroyGameLinked](IGame* game) {
                destroyGameLinked(game);
            } };
}

struct Args
{
    std::string configFilePath{};
};

std::optional<Args> parseCommandLine(int argc, const char* argv[]) {
    namespace po = boost::program_options;

    po::options_description description{ "Allowed options"s };
    Args args{};
    description.add_options()("help,h", "produce help message") //
        ("config-file,c",
         po::value(&args.configFilePath)->value_name("file"),
         "set config file path");

    po::variables_map vm{};
    po::store(po::parse_command_line(argc, argv, description), vm);
    po::notify(vm);

    if (!vm.contains("config-file")) throw std::runtime_error{ "Config file not been specified"s };

    return args;
}

Vertex blendVertex(const Vertex& v1, const Vertex& v2, const float a) {
    Vertex r{};
    r.x = (1.0f - a) * v1.x + a * v2.x;
    r.y = (1.0f - a) * v1.y + a * v2.y;

    r.texX = (1.0f - a) * v1.texX + a * v2.texX;
    r.texY = (1.0f - a) * v1.texY + a * v2.texY;

    return r;
}

Triangle blendTriangle(const Triangle& tl, const Triangle& tr, const float a) {
    Triangle triangle;
    triangle.vertices[0] = blendVertex(tl.vertices[0], tr.vertices[0], a);
    triangle.vertices[1] = blendVertex(tl.vertices[1], tr.vertices[1], a);
    triangle.vertices[2] = blendVertex(tl.vertices[2], tr.vertices[2], a);

    return triangle;
}

Triangle getTransformedTriangle(const Triangle& t, const glm::mat3& matrix) {
    Triangle result{ t };

    std::ranges::for_each(result.vertices, [&](Vertex& v) {
        glm::vec3 posSource{ v.x, v.y, 1.f };
        glm::vec3 posResult = matrix * posSource;

        v.x = posResult[0];
        v.y = posResult[1];
    });

    return result;
}

int main(int argc, const char* argv[]) {
    try {
        if (auto args{ parseCommandLine(argc, argv) }) {
            HotReloadProvider::setPath(args->configFilePath);

            createEngine();
            auto& engine{ getEngineInstance() };
            auto answer{ engine->initialize("{}") };
            if (!answer.empty()) { return EXIT_FAILURE; }
            std::cout << "start app"sv << std::endl;

            std::string_view tempLibraryName{ "./temp.dll" };
            void* gameLibraryHandle{};
            std::unique_ptr<IGame, std::function<void(IGame * game)>> game;

            HotReloadProvider::getInstance().addToCheck("game", [&]() {
                std::cout << "changing game\n"sv;
                game = reloadGame(std::move(game),
                                  HotReloadProvider::getInstance().getPath("game"),
                                  tempLibraryName,
                                  *engine,
                                  gameLibraryHandle);
                engine->uninitialize();
                game->initialize();
            });

            HotReloadProvider::getInstance().addToCheck("vertex_shader_with_view", [&]() {
                std::cout << "recompile shaders\n"sv;
                engine->recompileShaders();
            });

            HotReloadProvider::getInstance().addToCheck("vertex_shader_without_view", [&]() {
                std::cout << "recompile shaders\n"sv;
                engine->recompileShaders();
            });

            HotReloadProvider::getInstance().addToCheck("fragment_shader", [&]() {
                std::cout << "recompile shaders\n"sv;
                engine->recompileShaders();
            });

            HotReloadProvider::getInstance().check();

            bool isEnd{};
            while (!isEnd) {
                std::uint64_t frameStart{ SDL_GetTicks() };
                HotReloadProvider::getInstance().check();
                Event event{};
                while (engine->readInput(event)) {
                    std::cout << event << '\n';
                    if (event.type == Event::Type::turn_off) {
                        std::cout << "exiting"sv << std::endl;
                        isEnd = true;
                        break;
                    }

                    game->onEvent(event);
                }

                ImGui_ImplSDL3_NewFrame();
                ImGui_ImplOpenGL3_NewFrame();
                ImGui::NewFrame();

                game->update();
                game->render();

                engine->swapBuffers();

                if (!engine->getVSync() && engine->getFramerate() < 300) {
                    int frameDelay = 1000 / engine->getFramerate();
                    std::uint64_t frameTime{ SDL_GetTicks() - frameStart };
                    if (frameTime < frameDelay) SDL_Delay(frameDelay - frameTime);
                }
            }

            engine->uninitialize();
            return EXIT_SUCCESS;
        }
    }
    catch (const std::exception& e) {
        std::cerr << e.what() << '\n';
    }
    catch (...) {
        std::cerr << "Unknown error"sv << '\n';
    }

    return EXIT_FAILURE;
}
#else
int main(int argc, char* argv[]) {
    try {
        createEngine();
        auto& engine{ getEngineInstance() };
        if (!engine->initialize("{}").empty())
            throw std::runtime_error{ "Error: main : initializing engine."s };

        std::string libName{ "libgame.so" };

        auto gameHandle{ SDL_LoadObject(libName.c_str()) };
        if (gameHandle == nullptr) throw std::runtime_error{ "Error: main : load game."s };

        auto createGameFuncPtr{ SDL_LoadFunction(gameHandle, "createGame") };
        if (createGameFuncPtr == nullptr)
            throw std::runtime_error{ "Error: main : load function createGame."s };

        using CreateGame = decltype(&createGame);
        auto createGameLinked{ reinterpret_cast<CreateGame>(createGameFuncPtr) };

        auto destroyGameFuncPtr{ SDL_LoadFunction(gameHandle, "destroyGame") };
        if (destroyGameFuncPtr == nullptr)
            throw std::runtime_error{ "Error: main : load function destroyGame."s };

        using DestroyGame = decltype(&destroyGame);
        auto destroyGameLinked{ reinterpret_cast<DestroyGame>(destroyGameFuncPtr) };

        std::unique_ptr<IGame, std::function<void(IGame * game)>> game{
            createGameLinked(engine.get()),
            [destroyGameLinked](IGame* game) {
                destroyGameLinked(game);
            }
        };

        game->initialize();

        bool isEnd{};
        while (!isEnd) {
            std::uint64_t frameStart{ SDL_GetTicks() };
            Event event{};
            while (engine->readInput(event)) {
                std::cout << event << '\n';
                if (event.type == Event::Type::turn_off) {
                    std::cout << "exiting"sv << std::endl;
                    isEnd = true;
                    break;
                }

                game->onEvent(event);
            }

            ImGui_ImplSDL3_NewFrame();
            ImGui_ImplOpenGL3_NewFrame();
            ImGui::NewFrame();

            game->update();
            game->render();

            engine->swapBuffers();

            if (!engine->getVSync() && engine->getFramerate() < 300) {
                int frameDelay = 1000 / engine->getFramerate();
                std::uint64_t frameTime{ SDL_GetTicks() - frameStart };
                if (frameTime < frameDelay) SDL_Delay(frameDelay - frameTime);
            }
        }

        engine->uninitialize();
        return EXIT_SUCCESS;
    }
    catch (const std::exception& e) {
        std::cerr << e.what() << '\n';
    }
    catch (...) {
        std::cerr << "Unknown error"sv << '\n';
    }

    return EXIT_FAILURE;
}

#endif