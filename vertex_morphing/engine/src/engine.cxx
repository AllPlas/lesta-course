//
// Created by Алексей Крукович on 17.04.23.
//
#include "engine.hxx"

#include <boost/program_options.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <SDL3/SDL.h>
#include <cassert>
#include <filesystem>
#include <fstream>
#include <glad/glad.h>
#include <iostream>
#include <optional>
#include <stdexcept>
#include <thread>
#include <unordered_map>

#include "hot_reload_provider.hxx"
#include "imgui_impl_opengl3.hxx"
#include "imgui_impl_sdl3.hxx"
#include "opengl_check.hxx"
#include "program.hxx"
#include "texture.hxx"

using namespace std::literals;
namespace fs = std::filesystem;

static const std::unordered_map<Event::Type, std::string_view> s_eventTypeToStringView{
    { Event::Type::key_down, "key_down" },      { Event::Type::key_up, "key_up" },
    { Event::Type::mouse_down, "button_down" }, { Event::Type::mouse_up, "button_up" },
    { Event::Type::mouse_wheel, "spin_wheel" }, { Event::Type::mouse_motion, "mouse_motion" },
    { Event::Type::turn_off, "turn_off" },      { Event::Type::not_event, "" }
};

static const std::unordered_map<Event::Keyboard::Key, std::string_view> s_eventKeysToStringView{
    { Event::Keyboard::Key::w, "w_" },
    { Event::Keyboard::Key::a, "a_" },
    { Event::Keyboard::Key::s, "s_" },
    { Event::Keyboard::Key::d, "d_" },
    { Event::Keyboard::Key::space, "space_" },
    { Event::Keyboard::Key::enter, "enter_" },
    { Event::Keyboard::Key::l_control, "left_control_" },
    { Event::Keyboard::Key::l_shift, "left_shift_" },
    { Event::Keyboard::Key::escape, "escape_" },
    { Event::Keyboard::Key::backspace, "backspace_" },
    { Event::Keyboard::Key::not_key, "" }
};

static const std::unordered_map<Event::Mouse::Button, std::string_view> s_eventButtonsToStringView{
    { Event::Mouse::Button::left, "left_" },
    { Event::Mouse::Button::right, "right_" },
    { Event::Mouse::Button::middle, "middle_" },
    { Event::Mouse::Button::not_button, "" }
};

std::ostream& operator<<(std::ostream& out, const Event& eventNew) {
    out << s_eventKeysToStringView.at(eventNew.keyboard.key)
        << s_eventButtonsToStringView.at(eventNew.mouse.button)
        << s_eventTypeToStringView.at(eventNew.type);

    return out;
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
        { SDLK_w, Event::Keyboard::Key::w },
        { SDLK_a, Event::Keyboard::Key::a },
        { SDLK_s, Event::Keyboard::Key::s },
        { SDLK_d, Event::Keyboard::Key::d },
        { SDLK_SPACE, Event::Keyboard::Key::space },
        { SDLK_RETURN, Event::Keyboard::Key::enter },
        { SDLK_LCTRL, Event::Keyboard::Key::l_control },
        { SDLK_LSHIFT, Event::Keyboard::Key::l_shift },
        { SDLK_ESCAPE, Event::Keyboard::Key::escape },
        { SDLK_BACKSPACE, Event::Keyboard::Key::backspace }
    };

    if (auto found{ keymap.find(sdlEvent.key.keysym.sym) }; found != keymap.end()) {
        Event event{};
        event.keyboard.key = found->second;

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

class EngineImpl final : public IEngine
{
public:
    glm::mat3 matrix{};

private:
    SDL_Window* m_window{};
    SDL_GLContext m_glContext{};
    ShaderProgram m_program{};

    GLuint m_verticesArray{};

public:
    EngineImpl() = default;
    ~EngineImpl() override = default;

    std::string initialize(std::string_view config) override {
        initSDL();
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);
        m_window = createWindow();
        SDL_SetWindowPosition(m_window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
        SDL_SetWindowMinimumSize(m_window, 640, 480);
        SDL_ShowWindow(m_window);

        createGLContext();

        glGenVertexArrays(1, &m_verticesArray);
        openGLCheck();

        glBindVertexArray(m_verticesArray);
        openGLCheck();

        glEnable(GL_DEPTH_TEST);
        openGLCheck();

        glEnable(GL_BLEND);
        openGLCheck();

        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        openGLCheck();

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
        // ImGui::StyleColorsLight();

        // Setup Platform/Renderer backends
        ImGui_ImplSDL3_InitForOpenGL(m_window, m_glContext);
        const char* glsl_version = "#version 150";
        ImGui_ImplOpenGL3_Init(glsl_version);

        return "";
    }

    void uninitialize() override {
        glDeleteVertexArrays(1, &m_verticesArray);
        openGLCheck();

        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplSDL3_Shutdown();
        ImGui::DestroyContext();

        if (m_glContext) SDL_GL_DeleteContext(m_glContext);
        if (m_window) SDL_DestroyWindow(m_window);
        SDL_Quit();
    }

    bool readInput(Event& event) override {
        SDL_Event sdlEvent;
        if (SDL_PollEvent(&sdlEvent)) {
            ImGui_ImplSDL3_ProcessEvent(&sdlEvent);

            if (sdlEvent.type == SDL_EVENT_QUIT) {
                event.type = Event::Type::turn_off;
                return true;
            }

            if (sdlEvent.type == SDL_EVENT_KEY_DOWN || sdlEvent.type == SDL_EVENT_KEY_UP) {
                if (auto e{ checkKeyboardInput(sdlEvent) }) {
                    event = *e;
                    return true;
                }
            }

            if (sdlEvent.type == SDL_EVENT_MOUSE_BUTTON_DOWN ||
                sdlEvent.type == SDL_EVENT_MOUSE_BUTTON_UP ||
                sdlEvent.type == SDL_EVENT_MOUSE_MOTION || sdlEvent.type == SDL_EVENT_MOUSE_WHEEL) {
                if (auto e{ checkMouseInput(sdlEvent) }) {
                    event = *e;
                    return true;
                }
            }
        }

        return false;
    }

    void renderTriangle(const Triangle& triangle) override {
        auto matLoc{ glGetUniformLocation(*m_program, "matrix") };

        glUniformMatrix3fv(matLoc, 1, GL_FALSE, glm::value_ptr(matrix));

        GLuint vbo{};
        glGenBuffers(1, &vbo);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER,
                     triangle.vertices.size() * sizeof(Vertex),
                     triangle.vertices.data(),
                     GL_STATIC_DRAW);

        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glValidateProgram(*m_program);
        openGLCheck();

        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), nullptr);
        openGLCheck();

        glEnableVertexAttribArray(0);
        openGLCheck();

        glVertexAttribPointer(1,
                              2,
                              GL_FLOAT,
                              GL_FALSE,
                              sizeof(Vertex),
                              reinterpret_cast<const GLvoid*>(offsetof(Vertex, texX)));
        openGLCheck();

        glEnableVertexAttribArray(1);
        openGLCheck();

        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        openGLCheck();

        glDrawArrays(GL_TRIANGLES, 0, 3);
        openGLCheck();

        glDeleteBuffers(1, &vbo);
    }

    void renderTriangle(const Triangle& triangle, const Texture& texture) override {
        m_program.use();

        auto texture1{ glGetUniformLocation(*m_program, "texSampler") };
        openGLCheck();

        glUniform1i(texture1, 0);
        openGLCheck();

        glActiveTexture(GL_TEXTURE0);
        openGLCheck();

        texture.bind();
        renderTriangle(triangle);
    }

    void swapBuffers() override {
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL3_NewFrame();
        ImGui::NewFrame();

        bool show_demo_window{ true };

        if (show_demo_window) ImGui::ShowDemoWindow(&show_demo_window);
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        int w{}, h{};
        SDL_GetWindowSize(m_window, &w, &h);
        glViewport(0, 0, w, h);
        openGLCheck();

        SDL_GL_SwapWindow(m_window);

        glClearColor(0.0f, 0.0f, 0.f, 1.f);
        openGLCheck();

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        openGLCheck();
    }

    void recompileShaders(std::string_view vertexPath, std::string_view fragmentPath) override {
        m_program.recompileShaders(vertexPath, fragmentPath);
        m_program.use();

        auto texture{ glGetUniformLocation(*m_program, "texSampler") };
        openGLCheck();

        glUniform1i(texture, 0);
        openGLCheck();

        glActiveTexture(GL_TEXTURE0);
        openGLCheck();
    }

    void render(const VertexBuffer<Vertex2>& vertexBuffer,
                const IndexBuffer<std::uint16_t>& indexBuffer,
                const Texture& texture) override {
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
            GL_TRIANGLES, static_cast<GLsizei>(vertexBuffer.size()), GL_UNSIGNED_SHORT, nullptr);
        openGLCheck();
    }

private:
    static void initSDL() {
        if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS | SDL_INIT_AUDIO | SDL_INIT_GAMEPAD |
                     SDL_INIT_TIMER) != 0)
            throw std::runtime_error{ "Error : failed call SDL_Init: "s + SDL_GetError() };
    }

    static SDL_Window* createWindow() {
        if (auto window{
                SDL_CreateWindow("OpenGL test",
                                 800,
                                 600,
                                 SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIDDEN) };
            window != nullptr)
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

        if (platform == "macOS") {
            gl_major_ver = 4;
            gl_minor_ver = 1;
            gl_context_profile = SDL_GL_CONTEXT_PROFILE_CORE;
        }

        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, gl_context_profile);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, gl_major_ver);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, gl_minor_ver);

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
};

static bool s_alreadyExist{ false };

EnginePtr createEngine() {
    if (s_alreadyExist) throw std::runtime_error{ "Error : engine already exist"s };
    s_alreadyExist = true;
    return { new EngineImpl{}, destroyEngine };
}

void destroyEngine(IEngine* e) {
    if (!s_alreadyExist) throw std::runtime_error{ "Error : engine not exist"s };
    if (e == nullptr) throw std::runtime_error{ "Error : nullptr"s };
    delete e;
    s_alreadyExist = false;
}

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
            auto engine{ createEngine() };
            auto answer{ engine->initialize("") };
            if (!answer.empty()) { return EXIT_FAILURE; }
            std::cout << "start app"sv << std::endl;

            HotReloadProvider::setPath(args->configFilePath);

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
            });

            HotReloadProvider::getInstance().addToCheck("vertex_shader", [&]() {
                std::cout << "recompile shaders\n"sv;
                engine->recompileShaders(
                    HotReloadProvider::getInstance().getPath("vertex_shader"),
                    HotReloadProvider::getInstance().getPath("fragment_shader"));
            });

            HotReloadProvider::getInstance().addToCheck("fragment_shader", [&]() {
                std::cout << "recompile shaders\n"sv;
                engine->recompileShaders(
                    HotReloadProvider::getInstance().getPath("vertex_shader"),
                    HotReloadProvider::getInstance().getPath("fragment_shader"));
            });

            HotReloadProvider::getInstance().check();
            game->initialize();

            bool isEnd{};
            auto now = std::chrono::steady_clock::now();
            auto timeAfterLoading{ now };

            float posX{};
            float posY{};

            float speedX{};
            float speedY{};

            float angle{};

            Texture tank{};
            tank.load(HotReloadProvider::getInstance().getPath("tank_texture"));

            while (!isEnd) {
                HotReloadProvider::getInstance().check();
                Event event{};
                while (engine->readInput(event)) {
                    std::cout << event << '\n';
                    if (event.type == Event::Type::turn_off) {
                        std::cout << "exiting"sv << std::endl;
                        isEnd = true;
                        break;
                    }

                    //                  game->onEvent(event);
                    //
                    //                    if (event == Event::up_pressed) { speedY = 0.05; }
                    //                    else if (event == Event::down_pressed) { speedY = -0.05; }
                    //                    else if (event == Event::up_released || event ==
                    //                    Event::down_released) {
                    //                        speedY = 0.0;
                    //                    }
                    //
                    //                    if (event == Event::left_pressed) { speedX = -0.05; }
                    //                    else if (event == Event::right_pressed) { speedX = 0.05; }
                    //                    else if (event == Event::left_released || event ==
                    //                    Event::right_released) {
                    //                        speedX = 0.0;
                    //                    }
                    //
                    //                    if (event == Event::space_pressed) { angle += 0.05; }
                    //                    if (event == Event::lctrl_pressed) { angle -= 0.05; }
                }

                float alpha = std::sin(std::chrono::duration<float, std::ratio<1>>(
                                           std::chrono::steady_clock::now() - timeAfterLoading)
                                           .count()) *
                                  0.5f +
                              0.5f;

                auto elapsed = std::chrono::duration<float, std::ratio<1>>(
                                   std::chrono::steady_clock::now() - now)
                                   .count();
                now = std::chrono::steady_clock::now();

                posY += speedY * elapsed;
                posX += speedX * elapsed;

                Triangle tr1{ -0.4, 0.4, 1.0, 0.0,  0.0, 0.4, 0.4, 1.0,
                              1.0,  0.0, 0.4, -0.4, 1.0, 1.0, 1.0 };
                Triangle tr2{ -0.4, 0.4, 1.0,  0.0,  0.0, 0.4, -0.4, 1.0,
                              1.0,  1.0, -0.4, -0.4, 1.0, 0.0, 1.0 };

                Triangle tr3{ 0.0, -0.5, 0.0, -0.5, 0.0, 0.0, 0.5, 0.0, 0.0 };

                //    Triangle render{ blendTriangle(tr1, tr3, alpha) };
                //    Triangle render2{ blendTriangle(tr2, tr3, alpha) };

                glm::mat3 move{ 0.0f };
                move[0][0] = 1;
                move[1][1] = 1;
                move[2][0] = posX * std::cos(angle) - posY * std::sin(angle);
                move[2][1] = posX * std::sin(angle) + posY * std::cos(angle);
                move[2][2] = 1;

                glm::mat3 aspect{ 0.0f };
                aspect[0][0] = 1;
                aspect[0][1] = 0.f;
                aspect[1][0] = 0.f;
                aspect[1][1] = 640.f / 480.f;
                aspect[2][2] = 1;

                glm::mat3 rotation{ 0.0f };
                rotation[0][0] = std::cos(angle);
                rotation[0][1] = std::sin(angle);
                rotation[1][0] = -std::sin(angle);
                rotation[1][1] = std::cos(angle);
                rotation[2][2] = 1;

                glm::mat3 result{ move * aspect * rotation };

                auto d{ dynamic_cast<EngineImpl*>(engine.get()) };
                d->matrix = result;

                // tr1 = getTransformedTriangle(tr1, move * aspect * rotation);
                // tr2 = getTransformedTriangle(tr2, move * aspect * rotation);
                engine->renderTriangle(tr1, tank);
                engine->renderTriangle(tr2, tank);
                // engine->renderFromBuffer();
                engine->swapBuffers();
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