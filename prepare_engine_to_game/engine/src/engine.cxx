//
// Created by Алексей Крукович on 17.04.23.
//
#include "engine.hxx"

#include <boost/json.hpp>
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

using namespace std::literals;
namespace fs = std::filesystem;
namespace json = boost::json;

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

std::ostream& operator<<(std::ostream& out, const Event& event) {
    out << s_eventKeysToStringView.at(event.keyboard.key)
        << s_eventButtonsToStringView.at(event.mouse.button)
        << s_eventTypeToStringView.at(event.type);

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

    ShaderProgram m_program{};
    GLuint m_verticesArray{};

    int m_framerate{ 150 };

public:
    EngineImpl() = default;

    ~EngineImpl() override = default;

    std::string initialize(std::string_view config) override {
        initSDL();

        SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS,
                            SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG); // Always required on Mac
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);

        auto jsonValue(json::parse(config));

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

        m_window = createWindow(
            static_cast<int>(width), static_cast<int>(height), static_cast<SDL_WindowFlags>(flags));
        SDL_SetWindowPosition(m_window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
        SDL_SetWindowMinimumSize(m_window, static_cast<int>(minWidth), static_cast<int>(minHeight));
        SDL_ShowWindow(m_window);

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

        //  glUniformMatrix3fv(matLoc, 1, GL_FALSE, glm::value_ptr(matrix));
        std::vector<Vertex> vert{ triangle.vertices.begin(), triangle.vertices.end() };
        VertexBuffer<Vertex> vb{ std::move(vert) };
        std::vector<uint16_t> idx{ 0, 1, 2 };
        IndexBuffer ib{ std::move(idx) };
        vb.bind();
        ib.bind();

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

        glDrawElements(GL_TRIANGLES, ib.size(), GL_UNSIGNED_SHORT, nullptr);
        openGLCheck();

        glDisableVertexAttribArray(0);
        glDisableVertexAttribArray(1);
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
        bool show_demo_window{ true };

        if (show_demo_window) ImGui::ShowDemoWindow(&show_demo_window);
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        int width, height;
        SDL_GetWindowSizeInPixels(m_window, &width, &height);
        glViewport(0, 0, width, height);
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
        auto matLoc{ glGetUniformLocation(*m_program, "matrix") };
        glm::mat3 mat{};
        mat[0][0] = 1;
        mat[1][1] = 1;
        mat[2][2] = 1;
        glUniformMatrix3fv(matLoc, 1, GL_FALSE, glm::value_ptr(mat));

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
        //  openGLCheck();

        glDisableVertexAttribArray(0);
        glDisableVertexAttribArray(1);
        glDisableVertexAttribArray(2);
    }

    void render(const Sprite& sprite) override {
        auto matLoc{ glGetUniformLocation(*m_program, "matrix") };
        glUniformMatrix3fv(matLoc, 1, GL_FALSE, glm::value_ptr(sprite.getResultMatrix()));

        VertexBuffer vertexBuffer{ sprite.getVertices() };
        IndexBuffer indexBuffer{ sprite.getIndices() };

        sprite.getTexture().bind();
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

    ImGuiContext* getImGuiContext() const noexcept override { return ImGui::GetCurrentContext(); }

private:
    static void initSDL() {
        if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS | SDL_INIT_AUDIO | SDL_INIT_GAMEPAD |
                     SDL_INIT_TIMER) != 0)
            throw std::runtime_error{ "Error : failed call SDL_Init: "s + SDL_GetError() };
    }

    static SDL_Window* createWindow(int width, int height, SDL_WindowFlags flags) {
        if (auto window{ SDL_CreateWindow("OpenGL test", width, height, flags) }; window != nullptr)
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
static EnginePtr g_engine{};

void createEngine() {
    if (s_alreadyExist) throw std::runtime_error{ "Error : engine already exist"s };
    s_alreadyExist = true;
    g_engine = { new EngineImpl{}, destroyEngine };
}

void destroyEngine(IEngine* e) {
    if (!s_alreadyExist) throw std::runtime_error{ "Error : engine not exist"s };
    if (e == nullptr) throw std::runtime_error{ "Error : nullptr"s };
    delete e;
    s_alreadyExist = false;
}

const EnginePtr& getEngineInstance() {
    if (!s_alreadyExist) throw std::runtime_error{ "Error : engine not exist"s };
    return g_engine;
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
            createEngine();
            auto& engine{ getEngineInstance() };
            auto answer{ engine->initialize(R"({
"is_window_resizable": true
})") };
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
                game->initialize();
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

                if (!engine->getVSync()) {
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