#include "imgui_impl_sdl3.hxx"

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <SDL3/SDL.h>
#include <glad/glad.h>

#include "buffer.hxx"
#include "engine.hxx"
#include "program.hxx"
#include "texture.hxx"
#include "opengl_check.hxx"

static float g_time{};
static bool g_mousePressed[3]{};
static float g_mouseWheel{};
static Program g_program{};

void imgui_to_engine_render(ImDrawData* drawData, IEngine* engine) {
    // Avoid render when minimized, scale coordinates for retina displays
    ImGuiIO& io{ ImGui::GetIO() };

    int fbWidth{ static_cast<int>((io.DisplaySize.x * io.DisplayFramebufferScale.x)) };
    int fbHeight{ static_cast<int>(io.DisplaySize.y * io.DisplayFramebufferScale.y) };
    if (fbWidth == 0 || fbHeight == 0) return;

    drawData->ScaleClipRects(io.DisplayFramebufferScale);

    Texture* texture = reinterpret_cast<Texture*>(io.Fonts->TexID);
    assert(texture != nullptr);

    glm::mat3 move{};
    move[0][0] = 1;
    move[1][1] = 1;
    move[2][0] = -1;
    move[2][1] = 1;
    move[2][2] = 1;

    glm::mat3 aspect{};
    aspect[0][0] = 2.0f / io.DisplaySize.x;
    aspect[1][1] = -2.0f / io.DisplaySize.y;
    aspect[2][2] = 1;

    glm::mat3 resultMat{ move * aspect };

    g_program.use();

    auto matLoc{ glGetUniformLocation(*g_program, "ProjMtx") };
    openGLCheck();

    glUniformMatrix3fv(matLoc, 1, GL_FALSE, glm::value_ptr(resultMat));
    openGLCheck();

    auto texLoc{ glGetUniformLocation(*g_program, "Texture") };
    openGLCheck();

   // texture->bind();
    glUniform1i(texLoc, 0);
    openGLCheck();

    glActiveTexture(GL_TEXTURE0);
    openGLCheck();

    for (int n = 0; n < drawData->CmdListsCount; ++n) {
        const ImDrawList* cmdList = drawData->CmdLists[n];
        const ImDrawIdx* idxBufferOffset = nullptr;

        static_assert(sizeof(Vertex2) == sizeof(ImDrawVert), "Vertex2 != ImDrawVert");

        const Vertex2* vertexData{ reinterpret_cast<const Vertex2*>(cmdList->VtxBuffer.Data) };
        std::size_t verticesSize{ static_cast<std::size_t>(cmdList->VtxBuffer.size()) };
        std::vector<Vertex2> vector{ vertexData, vertexData + verticesSize };
        VertexBuffer<Vertex2> vertexBuffer{ std::move(vector) };

        const std::uint16_t* idxData{ reinterpret_cast<const std::uint16_t*>(
            cmdList->IdxBuffer.Data) };
        std::size_t indicesSize{ static_cast<std::size_t>(cmdList->IdxBuffer.size()) };
        std::vector<std::uint16_t> vector1{ idxData, idxData + indicesSize };
        IndexBuffer<std::uint16_t> indexBuffer{ std::move(vector1) };

        for (int cmd_i{}; cmd_i < cmdList->CmdBuffer.size(); ++cmd_i) {
            const ImDrawCmd* pcmd = &cmdList->CmdBuffer[cmd_i];

            assert(pcmd->UserCallback == nullptr); // we not use it

            Texture* texture1 = reinterpret_cast<Texture*>(pcmd->TextureId);

            engine->render(vertexBuffer, indexBuffer, texture1, 0, pcmd->ElemCount);

            idxBufferOffset += pcmd->ElemCount;
        }
    }
}

static const char* ImGui_ImplSdlGL3_GetClipboardText(void*) { return SDL_GetClipboardText(); }

static void ImGui_ImplSdlGL3_SetClipboardText(void*, const char* text) {
    SDL_SetClipboardText(text);
}

bool ImGui_ImplSDL3_InitForOpenGL(SDL_Window* window, void* sdl_gl_context) {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();

    ImGuiIO& io{ ImGui::GetIO() };
    io.BackendPlatformName = "lesta engine";
    io.KeyMap[ImGuiKey_Tab] = SDL_SCANCODE_TAB;
    io.KeyMap[ImGuiKey_LeftArrow] = SDL_SCANCODE_LEFT;
    io.KeyMap[ImGuiKey_RightArrow] = SDL_SCANCODE_RIGHT;
    io.KeyMap[ImGuiKey_UpArrow] = SDL_SCANCODE_UP;
    io.KeyMap[ImGuiKey_DownArrow] = SDL_SCANCODE_DOWN;
    io.KeyMap[ImGuiKey_PageUp] = SDL_SCANCODE_PAGEUP;
    io.KeyMap[ImGuiKey_PageDown] = SDL_SCANCODE_PAGEDOWN;
    io.KeyMap[ImGuiKey_Home] = SDL_SCANCODE_HOME;
    io.KeyMap[ImGuiKey_End] = SDL_SCANCODE_END;
    io.KeyMap[ImGuiKey_Insert] = SDL_SCANCODE_INSERT;
    io.KeyMap[ImGuiKey_Delete] = SDL_SCANCODE_DELETE;
    io.KeyMap[ImGuiKey_Backspace] = SDL_SCANCODE_BACKSPACE;
    io.KeyMap[ImGuiKey_Space] = SDL_SCANCODE_SPACE;
    io.KeyMap[ImGuiKey_Enter] = SDL_SCANCODE_RETURN;
    io.KeyMap[ImGuiKey_Escape] = SDL_SCANCODE_ESCAPE;
    io.KeyMap[ImGuiKey_A] = SDL_SCANCODE_A;
    io.KeyMap[ImGuiKey_C] = SDL_SCANCODE_C;
    io.KeyMap[ImGuiKey_V] = SDL_SCANCODE_V;
    io.KeyMap[ImGuiKey_X] = SDL_SCANCODE_X;
    io.KeyMap[ImGuiKey_Y] = SDL_SCANCODE_Y;
    io.KeyMap[ImGuiKey_Z] = SDL_SCANCODE_Z;

    io.SetClipboardTextFn = ImGui_ImplSdlGL3_SetClipboardText;
    io.GetClipboardTextFn = ImGui_ImplSdlGL3_GetClipboardText;

    g_time = SDL_GetTicks() / 1000.f;

    return true;
}

void ImGui_ImplSdlGL3_CreateFontsTexture() {
    // Build texture atlas
    ImGuiIO& io = ImGui::GetIO();
    unsigned char* pixels = nullptr;
    int width = 0;
    int height = 0;
    io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);

    // Store our identifier
    Texture* texture{ new Texture{} };
    texture->load(pixels, width, height);
    io.Fonts->TexID = texture;
}

bool ImGui_ImplSdlGL3_CreateDeviceObjects() {

    g_program.recompileShaders(
        "/Users/aleksey/lesta-course/vertex_morphing/data/shaders/imgui.vert",
        "/Users/aleksey/lesta-course/vertex_morphing/data/shaders/imgui.frag");

    ImGui_ImplSdlGL3_CreateFontsTexture();

    return true;
}

void ImGui_ImplSDL3_Shutdown() { ImGui::DestroyContext(); }

void ImGui_ImplSDL3_NewFrame(SDL_Window* window) {
    ImGuiIO& io{ ImGui::GetIO() };

    if (io.Fonts->TexID == nullptr) { ImGui_ImplSdlGL3_CreateDeviceObjects(); }

    // setup display size (every frame to accommodate for window resizing)
    int w, h;
    int display_w, display_h;
    SDL_GetWindowSize(window, &w, &h);
    SDL_GetWindowSizeInPixels(window, &display_w, &display_h);
    io.DisplaySize = ImVec2(static_cast<float>(w), static_cast<float>(h));
    io.DisplayFramebufferScale = ImVec2(w > 0 ? static_cast<float>(display_w) / w : 0.0f,
                                        h > 0 ? static_cast<float>(display_h) / h : 0.0f);

    // Setup time step
    uint64_t time{ SDL_GetTicks() };
    float current_time{ time / 1000.f };
    io.DeltaTime = current_time - g_time;

    if (io.DeltaTime <= 0) io.DeltaTime = 0.00001f;
    g_time = current_time;

    // Setup inputs

    float mx, my;
    uint32_t mouseMask{ SDL_GetMouseState(&mx, &my) };

    if (SDL_GetWindowFlags(window) & SDL_WINDOW_MOUSE_FOCUS)
        io.MousePos = ImVec2(mx, my);
    else
        io.MousePos = ImVec2(-FLT_MAX, -FLT_MAX);

    io.MouseDown[0] = g_mousePressed[0] || (mouseMask & SDL_BUTTON(SDL_BUTTON_LEFT)) != 0;
    io.MouseDown[1] = g_mousePressed[1] || (mouseMask & SDL_BUTTON(SDL_BUTTON_RIGHT)) != 0;
    io.MouseDown[2] = g_mousePressed[2] || (mouseMask & SDL_BUTTON(SDL_BUTTON_MIDDLE)) != 0;

    g_mousePressed[0] = g_mousePressed[1] = g_mousePressed[2] = false;

    io.MouseWheel = g_mouseWheel;
    g_mouseWheel = 0.0f;

    if (io.MouseDrawCursor)
        SDL_HideCursor();
    else
        SDL_ShowCursor();

    ImGui::NewFrame();
}

bool ImGui_ImplSDL3_ProcessEvent(const SDL_Event* event) {
    ImGuiIO& io{ ImGui::GetIO() };

    switch (event->type) {
    case SDL_EVENT_MOUSE_WHEEL:
        if (event->wheel.y > 0)
            g_mouseWheel = 1;
        else if (event->wheel.y < 0)
            g_mouseWheel = -1;

        return true;

    case SDL_EVENT_MOUSE_BUTTON_DOWN:
        if (event->button.button == SDL_BUTTON_LEFT) g_mousePressed[0] = true;
        if (event->button.button == SDL_BUTTON_RIGHT) g_mousePressed[1] = true;
        if (event->button.button == SDL_BUTTON_MIDDLE) g_mousePressed[2] = true;

        return true;

    case SDL_EVENT_TEXT_INPUT:
        io.AddInputCharactersUTF8(event->text.text);
        return true;

    case SDL_EVENT_KEY_DOWN:
    case SDL_EVENT_KEY_UP:
        int key{ event->key.keysym.sym & ~SDLK_SCANCODE_MASK };
        io.KeysDown[key] = (event->type == SDL_EVENT_KEY_DOWN);
        uint32_t mod_keys_mask = SDL_GetModState();
        io.KeyShift = ((mod_keys_mask & SDL_KMOD_SHIFT) != 0);
        io.KeyCtrl = ((mod_keys_mask & SDL_KMOD_CTRL) != 0);
        io.KeyAlt = ((mod_keys_mask & SDL_KMOD_ALT) != 0);
        io.KeySuper = ((mod_keys_mask & SDL_KMOD_GUI) != 0);

        return true;
    }

    return false;
}
