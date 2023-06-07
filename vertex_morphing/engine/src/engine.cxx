//
// Created by Алексей Крукович on 17.04.23.
//
#include "engine.hxx"

#include <boost/program_options.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
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
#include "imgui_impl_sdl3.hxx"
#include "opengl_check.hxx"
#include "program.hxx"
#include "texture.hxx"

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
    glm::mat3 matrix{};

private:
    SDL_Window* m_window{};
    SDL_GLContext m_glContext{};
    Program m_program{};

    GLuint m_verticesArray{};

    GLuint m_verticesBuffer{};
    GLuint m_indicesBuffer{};

    std::vector<Vertex> m_vertices{};
    std::vector<GLuint> m_indices{};

    Texture m_texture{};

public:
    EngineImpl() = default;
    ~EngineImpl() override { uninitialize(); }

    std::string initialize(std::string_view config) override {
        initSDL();
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);
        m_window = createWindow();
        createGLContext();

        glGenBuffers(1, &m_verticesBuffer);
        openGLCheck();

        glGenBuffers(1, &m_indicesBuffer);
        openGLCheck();

        glBindBuffer(GL_ARRAY_BUFFER, m_verticesBuffer);
        openGLCheck();

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indicesBuffer);
        openGLCheck();

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

        glViewport(0, 0, 800, 600);
        openGLCheck();

        SDL_GL_SetSwapInterval(1);

        ImGui_ImplSDL3_InitForOpenGL(m_window, m_glContext);

        return "";
    }

    void uninitialize() override {
        glDeleteBuffers(1, &m_verticesBuffer);
        openGLCheck();

        glDeleteBuffers(1, &m_indicesBuffer);
        openGLCheck();

        glDeleteVertexArrays(1, &m_verticesArray);
        openGLCheck();

        if (m_glContext) SDL_GL_DeleteContext(m_glContext);
        if (m_window) SDL_DestroyWindow(m_window);
        SDL_Quit();
    }

    bool readInput(Event& event) override {
        SDL_Event sdlEvent;
        if (SDL_PollEvent(&sdlEvent)) {
            ImGui_ImplSDL3_ProcessEvent(&sdlEvent);

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
        texture.bind();
        renderTriangle(triangle);
    }

    void renderFromBuffer() override {
        glValidateProgram(*m_program);
        openGLCheck();

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), nullptr);
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

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indicesBuffer);
        openGLCheck();

        glDrawElements(GL_TRIANGLES, m_indices.size(), GL_UNSIGNED_INT, nullptr);
        openGLCheck();
    }

    void swapBuffers() override {
        ImGui_ImplSDL3_NewFrame(m_window);

        bool show_demo_window{ true };

        ImGui::ShowDemoWindow(&show_demo_window);
        ImGui::Text("Hello");
        ImGui::Render();
        ImDrawData* drawData = ImGui::GetDrawData();
        imgui_to_engine_render(drawData, this);

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

    void reloadIndicesBuffer(std::string_view indicesPath) override {
        m_indices.clear();
        glBindVertexArray(0);
        openGLCheck();

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
        openGLCheck();

        glBindVertexArray(m_verticesArray);
        openGLCheck();
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
        openGLCheck();
    }

    void render(const VertexBuffer<Vertex2>& vertexBuffer,
                const IndexBuffer<std::uint16_t>& indexBuffer,
                Texture* texture,
                std::uint16_t startIndex,
                std::size_t numVertices) override {

        glEnable(GL_BLEND);
        openGLCheck();

        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        openGLCheck();

        texture->bind();
        glActiveTexture(GL_TEXTURE0);
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
            GL_TRIANGLES, static_cast<GLsizei>(numVertices), GL_UNSIGNED_SHORT, nullptr);
        openGLCheck();
    }

private:
    static void initSDL() {
        if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) != 0)
            throw std::runtime_error{ "Error : failed call SDL_Init: "s + SDL_GetError() };
    }

    static SDL_Window* createWindow() {
        if (auto window{ SDL_CreateWindow("OpenGL test", 800, 600, SDL_WINDOW_OPENGL) };
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
IEngine* g_enginePtr{};

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

IEngine* getEngineInstance() {
    if (s_alreadyExist) return g_enginePtr;
    throw std::runtime_error{ "Error : engine not exist"s };
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
            g_enginePtr = engine.get();
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

            HotReloadProvider::getInstance().addToCheck("indices", [&]() {
                std::cout << "reload indices buffer\n"sv;
                engine->reloadIndicesBuffer(HotReloadProvider::getInstance().getPath("indices"));
            });

            HotReloadProvider::getInstance().addToCheck("vertices", [&]() {
                std::cout << "reload vertices buffer\n"sv;
                engine->reloadVerticesBuffer(HotReloadProvider::getInstance().getPath("vertices"));
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
                    if (event == Event::turn_off) {
                        std::cout << "exiting"sv << std::endl;
                        isEnd = true;
                        break;
                    }

                    //                  game->onEvent(event);

                    if (event == Event::up_pressed) { speedY = 0.05; }
                    else if (event == Event::down_pressed) { speedY = -0.05; }
                    else if (event == Event::up_released || event == Event::down_released) {
                        speedY = 0.0;
                    }

                    if (event == Event::left_pressed) { speedX = -0.05; }
                    else if (event == Event::right_pressed) { speedX = 0.05; }
                    else if (event == Event::left_released || event == Event::right_released) {
                        speedX = 0.0;
                    }

                    if (event == Event::space_pressed) { angle += 0.05; }
                    if (event == Event::lctrl_pressed) { angle -= 0.05; }
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
           //     engine->renderTriangle(tr1, tank);
             //   engine->renderTriangle(tr2, tank);
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