//
// Created by Алексей Крукович on 17.04.23.
//
#include "engine.hxx"

#include <boost/program_options.hpp>

#include <SDL3/SDL.h>
#include <cassert>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <optional>
#include <stdexcept>
#include <thread>
#include <unordered_map>

#include "glad/glad.h"
#include "hot_reload_provider.hxx"
#include "program.hxx"

#define OM_GL_CHECK()                                                                            \
    {                                                                                            \
        const GLenum err = glGetError();                                                         \
        if (err != GL_NO_ERROR) {                                                                \
            switch (err) {                                                                       \
            case GL_INVALID_ENUM:                                                                \
                std::cerr << "GL_INVALID_ENUM" << std::endl;                                     \
                break;                                                                           \
            case GL_INVALID_VALUE:                                                               \
                std::cerr << "GL_INVALID_VALUE" << std::endl;                                    \
                break;                                                                           \
            case GL_INVALID_OPERATION:                                                           \
                std::cerr << "GL_INVALID_OPERATION" << std::endl;                                \
                break;                                                                           \
            case GL_INVALID_FRAMEBUFFER_OPERATION:                                               \
                std::cerr << "GL_INVALID_FRAMEBUFFER_OPERATION" << std::endl;                    \
                break;                                                                           \
            case GL_OUT_OF_MEMORY:                                                               \
                std::cerr << "GL_OUT_OF_MEMORY" << std::endl;                                    \
                break;                                                                           \
            default:                                                                             \
                std::cerr << "UNKNOWN ERROR" << std::endl;                                       \
                break;                                                                           \
            }                                                                                    \
            std::cerr << __FILE__ << ':' << __LINE__ << '(' << __FUNCTION__ << ')' << std::endl; \
            assert(false);                                                                       \
        }                                                                                        \
    }

using namespace std::literals;
namespace fs = std::filesystem;

static const std::unordered_map<Event, std::string_view> s_eventToStringView{
    { Event::up_pressed, "up_pressed" },
    { Event::up_released, "up_released" },
    { Event::down_pressed, "down_pressed" },
    { Event::down_released, "down_released" },
    { Event::left_pressed, "left_pressed" },
    { Event::left_released, "left_released" },
    { Event::right_pressed, "right_pressed" },
    { Event::right_released, "right_released" },
    { Event::space_pressed, "space_pressed" },
    { Event::space_released, "space_released" },
    { Event::return_pressed, "return_pressed" },
    { Event::return_released, "return_released" },
    { Event::lctrl_pressed, "lctrl_pressed" },
    { Event::lctrl_released, "lctrl_released" },
    { Event::turn_off, "turn_off" }
};

std::ostream& operator<<(std::ostream& out, Event event) {
    out << s_eventToStringView.at(event);
    return out;
}

std::ifstream& operator>>(std::ifstream& in, Vertex& vertex) {
    in >> vertex.x >> vertex.y >> vertex.z;
    return in;
}

std::ifstream& operator>>(std::ifstream& in, Triangle& triangle) {
    for (auto& vertex : triangle.vertices)
        in >> vertex;

    return in;
}

struct EventBind
{
    Event eventPressed{};
    Event eventReleased{};
};

static std::optional<Event> checkKeyboardInput(SDL_Event& event) {
    static const std::unordered_map<SDL_Keycode, EventBind> keymap{
        { SDLK_w, EventBind{ Event::up_pressed, Event::up_released } },
        { SDLK_s, EventBind{ Event::down_pressed, Event::down_released } },
        { SDLK_a, EventBind{ Event::left_pressed, Event::left_released } },
        { SDLK_d, EventBind{ Event::right_pressed, Event::right_released } },
        { SDLK_SPACE, EventBind{ Event::space_pressed, Event::space_released } },
        { SDLK_RETURN, EventBind{ Event::return_pressed, Event::return_released } },
        { SDLK_LCTRL, EventBind{ Event::lctrl_pressed, Event::lctrl_released } }
    };

    if (auto found{ keymap.find(event.key.keysym.sym) }; found != keymap.end()) {
        if (event.type == SDL_EVENT_KEY_DOWN)
            return found->second.eventPressed;
        else if (event.type == SDL_EVENT_KEY_UP)
            return found->second.eventReleased;
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
    struct Uniforms
    {
        float scale{};
        float time{};
        std::pair<float, float> center{};
        float spiral_speed{};
        float spiral_density{};
    };

    inline static Uniforms uniforms{ 1.0, 1.0, {}, 1.0, 1.0 };

private:
    SDL_Window* m_window{};
    SDL_GLContext m_glContext{};
    Program m_program{};

    GLuint m_verticesBuffer{};
    GLuint m_indicesBuffer{};

    std::vector<Vertex> m_vertices{};
    std::vector<GLuint> m_indices{};

public:
    EngineImpl() = default;
    ~EngineImpl() override { uninitialize(); }

    std::string initialize(std::string_view config) override {
        initSDL();
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);
        m_window = createWindow();
        createGLContext();

        glGenBuffers(1, &m_verticesBuffer);
        OM_GL_CHECK();

        glGenBuffers(1, &m_indicesBuffer);
        OM_GL_CHECK();

        glBindBuffer(GL_ARRAY_BUFFER, m_verticesBuffer);
        OM_GL_CHECK();

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indicesBuffer);
        OM_GL_CHECK();

        return "";
    }

    void uninitialize() override {
        glDeleteBuffers(1, &m_verticesBuffer);
        OM_GL_CHECK();

        glDeleteBuffers(1, &m_indicesBuffer);
        OM_GL_CHECK();

        if (m_glContext) SDL_GL_DeleteContext(m_glContext);
        if (m_window) SDL_DestroyWindow(m_window);
        SDL_Quit();
    }

    bool readInput(Event& event) override {
        SDL_Event sdlEvent;
        if (SDL_PollEvent(&sdlEvent)) {
            if (sdlEvent.type == SDL_EVENT_QUIT) {
                event = Event::turn_off;
                return true;
            }

            if (sdlEvent.type == SDL_EVENT_KEY_DOWN || sdlEvent.type == SDL_EVENT_KEY_UP) {
                if (auto e{ checkKeyboardInput(sdlEvent) }) {
                    event = *e;
                    return true;
                }
            }
        }

        return false;
    }

    void renderTriangle(const Triangle& triangle) override {
        setupUniforms();

        GLuint vao{};
        glGenVertexArrays(1, &vao);
        OM_GL_CHECK();

        glBindVertexArray(vao);
        OM_GL_CHECK();

        GLuint vbo{};
        glGenBuffers(1, &vbo);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER,
                     triangle.vertices.size() * sizeof(Vertex),
                     triangle.vertices.data(),
                     GL_STATIC_DRAW);

        glValidateProgram(*m_program);
        OM_GL_CHECK();

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), nullptr);
        OM_GL_CHECK();

        glEnableVertexAttribArray(0);
        OM_GL_CHECK();

        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glDrawArrays(GL_TRIANGLES, 0, 3);
        OM_GL_CHECK();

        glDisableVertexAttribArray(0);
        OM_GL_CHECK();

        glDeleteVertexArrays(1, &vao);
        OM_GL_CHECK();

        glDeleteBuffers(1, &vbo);
    }

    void renderFromBuffer() override {
        setupUniforms();

        GLuint vao{};
        glGenVertexArrays(1, &vao);
        OM_GL_CHECK();

        glBindVertexArray(vao);
        OM_GL_CHECK();

        glValidateProgram(*m_program);
        OM_GL_CHECK();

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), nullptr);
        OM_GL_CHECK();

        glEnableVertexAttribArray(0);
        OM_GL_CHECK();

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indicesBuffer);
        OM_GL_CHECK();

        glDrawElements(GL_TRIANGLES, m_indices.size(), GL_UNSIGNED_INT, nullptr);
        OM_GL_CHECK();

        glDisableVertexAttribArray(0);
        OM_GL_CHECK();

        glDeleteVertexArrays(1, &vao);
        OM_GL_CHECK();
    }

    void swapBuffers() override {
        SDL_GL_SwapWindow(m_window);

        glClearColor(0.0f, 0.0f, 0.f, 1.f);
        OM_GL_CHECK();

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        OM_GL_CHECK();
    }

    void recompileShaders(std::string_view vertexPath, std::string_view fragmentPath) override {
        m_program.recompileShaders(vertexPath, fragmentPath);
        m_program.use();
    }

    void reloadIndicesBuffer(std::string_view indicesPath) override {
        m_indices.clear();

        std::ifstream in{ indicesPath.data() };
        if (!in.is_open())
            throw std::runtime_error{ "Error : reloadIndicesBuffer : bad open file"s };

        while (in) {
            GLuint index{};
            in >> index;
            if (!in) break;
            m_indices.push_back(index);
        }

        glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                     m_indices.size() * sizeof(GLuint),
                     m_indices.data(),
                     GL_STATIC_DRAW);
        OM_GL_CHECK();
    }

    void reloadVerticesBuffer(std::string_view verticesPath) override {
        m_vertices.clear();

        std::ifstream in{ verticesPath.data() };
        if (!in.is_open())
            throw std::runtime_error{ "Error : reloadVerticesBuffer : bad open file"s };

        while (in) {
            Vertex vertex{};
            in >> vertex;
            if (!in) break;
            m_vertices.push_back(vertex);
        }

        glBufferData(
            GL_ARRAY_BUFFER, m_vertices.size() * sizeof(Vertex), m_vertices.data(), GL_STATIC_DRAW);
        OM_GL_CHECK();
    }

private:
    static void initSDL() {
        if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) != 0)
            throw std::runtime_error{ "Error : failed call SDL_Init: "s + SDL_GetError() };
    }

    static SDL_Window* createWindow() {
        if (auto window{ SDL_CreateWindow("OpenGL test", 640, 480, SDL_WINDOW_OPENGL) };
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

    void setupUniforms() const {
        auto scaleLocation{ glGetUniformLocation(*m_program, "scale") };
        OM_GL_CHECK();

        glUniform1f(scaleLocation, uniforms.scale);
        OM_GL_CHECK();

        auto timeLocation{ glGetUniformLocation(*m_program, "time") };
        OM_GL_CHECK();

        glUniform1f(timeLocation, uniforms.time);
        OM_GL_CHECK();

        auto centerLocation{ glGetUniformLocation(*m_program, "center") };
        glUniform2f(centerLocation, uniforms.center.first, uniforms.center.second);

        auto spiralSpeedLocation{ glGetUniformLocation(*m_program, "spiral_speed") };
        glUniform1f(spiralSpeedLocation, uniforms.spiral_speed);

        auto spiralDensity{ glGetUniformLocation(*m_program, "spiral_density") };
        glUniform1f(spiralDensity, uniforms.spiral_density);
    }
};

static bool s_alreadyExist{ false };

std::unique_ptr<IEngine, std::function<void(IEngine*)>> createEngine() {
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
    return r;
}

Triangle blendTriangle(const Triangle& tl, const Triangle& tr, const float a) {
    Triangle triangle;
    triangle.vertices[0] = blendVertex(tl.vertices[0], tr.vertices[0], a);
    triangle.vertices[1] = blendVertex(tl.vertices[1], tr.vertices[1], a);
    triangle.vertices[2] = blendVertex(tl.vertices[2], tr.vertices[2], a);

    return triangle;
}

int main(int argc, const char* argv[]) {
    try {
        if (auto args{ parseCommandLine(argc, argv) }) {
            auto engine{ createEngine() };
            auto answer{ engine->initialize("") };
            if (!answer.empty()) { return EXIT_FAILURE; }
            std::cout << "start app"sv << std::endl;

            HotReloadProvider hotReloadProvider{ args->configFilePath };

            std::string_view tempLibraryName{ "./temp.dll" };
            void* gameLibraryHandle{};
            std::unique_ptr<IGame, std::function<void(IGame * game)>> game;

            hotReloadProvider.addToCheck("game", [&]() {
                std::cout << "changing game\n"sv;
                game = reloadGame(std::move(game),
                                  hotReloadProvider.getPath("game"),
                                  tempLibraryName,
                                  *engine,
                                  gameLibraryHandle);
            });

            hotReloadProvider.addToCheck("vertex_shader", [&]() {
                std::cout << "recompile shaders\n"sv;
                engine->recompileShaders(hotReloadProvider.getPath("vertex_shader"),
                                         hotReloadProvider.getPath("fragment_shader"));
            });

            hotReloadProvider.addToCheck("fragment_shader", [&]() {
                std::cout << "recompile shaders\n"sv;
                engine->recompileShaders(hotReloadProvider.getPath("vertex_shader"),
                                         hotReloadProvider.getPath("fragment_shader"));
            });

            hotReloadProvider.addToCheck("indices", [&]() {
                std::cout << "reload indices buffer\n"sv;
                engine->reloadIndicesBuffer(hotReloadProvider.getPath("indices"));
            });

            hotReloadProvider.addToCheck("vertices", [&]() {
                std::cout << "reload vertices buffer\n"sv;
                engine->reloadVerticesBuffer(hotReloadProvider.getPath("vertices"));
            });

            hotReloadProvider.check();
            game->initialize();

            bool isEnd{};
            auto& time = EngineImpl::uniforms.time;
            auto now = std::chrono::steady_clock::now();
            auto timeAfterLoading{ now };
            while (!isEnd) {
                hotReloadProvider.check();
                Event event{};
                while (engine->readInput(event)) {

                    if (event == Event::turn_off) {
                        std::cout << "exiting"sv << std::endl;
                        isEnd = true;
                        break;
                    }

                    game->onEvent(event);
                }

                if (std::chrono::steady_clock::now() - now > 0.05s) {
                    ++time;
                    now = std::chrono::steady_clock::now();
                }

                float alpha = std::sin(std::chrono::duration<float, std::ratio<1>>(
                                           std::chrono::steady_clock::now() - timeAfterLoading)
                                           .count()) *
                                  0.5f +
                              0.5f;

                Triangle tr1{ 0.0, 0.5, 0.0, 0.5, 0.0, 0.0, -0.5, 0.0, 0.0 };
                Triangle tr2{ -0.5, 0.0, 0.0, 0.5, 0.0, 0.0, 1.0, 0.0, 0.0 };
                Triangle tr3{ 0.0, -0.5, 0.0, -0.5, 0.0, 0.0, 0.5, 0.0, 0.0 };

                Triangle render{ blendTriangle(tr1, tr2, alpha) };
                Triangle render2{ blendTriangle(tr3, tr2, alpha) };

                engine->renderTriangle(render);
                engine->renderTriangle(render2);
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