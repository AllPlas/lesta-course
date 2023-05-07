//
// Created by Алексей Крукович on 17.04.23.
//
#include "engine.hxx"

#include <SDL3/SDL.h>
#include <cassert>
#include <filesystem>
#include <fstream>
#include <future>
#include <iostream>
#include <optional>
#include <stdexcept>
#include <thread>
#include <unordered_map>

#include "glad/glad.h"

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
private:
    SDL_Window* m_window{};
    SDL_GLContext m_glContext{};
    GLuint m_programId{};

    fs::path shaderFolder;

public:
    EngineImpl() = default;
    ~EngineImpl() override { uninitialize(); }

    std::string initialize(std::string_view config) override {
        shaderFolder = config;
        initSDL();
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);
        m_window = createWindow();
        createGLContext();
        createProgram();
        return "";
    }

    void uninitialize() override {
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
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), triangle.vertices.data());
        glEnableVertexAttribArray(0);
        glValidateProgram(m_programId);

        glDrawArrays(GL_TRIANGLES, 0, 3);
    }

    void swapBuffers() override {
        SDL_GL_SwapWindow(m_window);
        glClearColor(0.3f, 0.3f, 1.f, 1.f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
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

    void createProgram() {
        m_programId = glCreateProgram();
        createShader(GL_VERTEX_SHADER, "vertex_shader.glsl");
        createShader(GL_FRAGMENT_SHADER, "fragment_shader.glsl");

        glLinkProgram(m_programId);
        glBindAttribLocation(m_programId, 0, "a_position");

        glUseProgram(m_programId);
        glEnable(GL_DEPTH_TEST);
    }

    void createShader(GLuint type, std::string_view filename) {
        GLuint shader{ glCreateShader(type) };
        std::string shaderSource{ readFile(shaderFolder / filename) };
        const char* source{ shaderSource.c_str() };

        glShaderSource(shader, 1, &source, nullptr);
        glCompileShader(shader);

        GLint compileStatus{};
        glGetShaderiv(shader, GL_COMPILE_STATUS, &compileStatus);
        if (compileStatus == 0) {
            glDeleteShader(shader);
            throw std::runtime_error{ "Error : createShader : bad compile shader"s };
        }

        glAttachShader(m_programId, shader);
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

int main(int argc, char* argv[]) {
    try {
        auto engine{ createEngine() };
        auto answer = engine->initialize(argv[1]);
        if (!answer.empty()) { return EXIT_FAILURE; }
        std::cout << "start app"sv << std::endl;

        std::string_view libraryName{ SDL_GetPlatform() == "Windows"s ? "game.dll" : "./game.so" };
        std::string_view tempLibraryName{ "./temp.dll" };
        void* gameLibraryHandle{};
        auto game{ reloadGame(nullptr, libraryName, tempLibraryName, *engine, gameLibraryHandle) };
        auto timeDuringLoading{ fs::last_write_time(libraryName) };
        game->initialize();

        bool isEnd{};
        auto updateResult{ std::async(std::launch::async, &IGame::update, game.get()) };
        while (!isEnd) {
            auto currentWriteTime{ fs::last_write_time(libraryName) };
            if (currentWriteTime != timeDuringLoading) {
                std::cout << "waiting...\n"sv;
                updateResult.get();

                while (true) {
                    std::this_thread::sleep_for(100ms);
                    auto nextWriteTime{ fs::last_write_time(libraryName) };

                    if (nextWriteTime != currentWriteTime)
                        currentWriteTime = nextWriteTime;
                    else
                        break;
                }

                std::cout << "reloading game\n"sv;
                game = reloadGame(
                    std::move(game), libraryName, tempLibraryName, *engine, gameLibraryHandle);

                if (game == nullptr) {
                    std::cerr << "next attempt to reload game\n"sv;
                    continue;
                }

                timeDuringLoading = currentWriteTime;
            }

            Event event{};
            while (engine->readInput(event)) {
                // std::cout << event << '\n';

                if (event == Event::turn_off) {
                    std::cout << "exiting..."sv << std::endl;
                    isEnd = true;
                    break;
                }

                game->onEvent(event);
            }

            if (event == Event::turn_off) break;
            if (!updateResult.valid())
                updateResult = std::async(std::launch::async, &IGame::update, game.get());
            else if (updateResult.wait_for(0s) == std::future_status::ready) {
                // game->update();
                //      game->render();
                //                updateResult = std::async(std::launch::async, &IGame::update,
                //                game.get());
            }

            Triangle t{ { -0.5, -0.5, 0.5, 0.5, -0.5, 0.5, 0.0, 0.8, -0.5 } };

            engine->renderTriangle(t);
            engine->swapBuffers();
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