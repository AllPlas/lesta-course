#include <array>
#include <engine.hxx>
#include <stdexcept>

using namespace std::literals;

class PirateGame : public IGame
{
private:
    Sprite sprite{ "/Users/aleksey/lesta-course/prepare_engine_to_game/data/tank_dark.png",
                   { 84, 92 } };

    bool m_isDebugMenuOn{ false };
    int m_framerate{ 150 };

public:
    void initialize() override {
        ImGui::SetCurrentContext(getEngineInstance()->getCurrentContext());
    }

    void onEvent(const Event& event) override {
        if (event.type == Event::Type::key_down) {
            if (event.keyboard.key == Event::Keyboard::Key::l_control)
                m_isDebugMenuOn = !m_isDebugMenuOn;
        }
    }

    void render() const override { getEngineInstance()->render(sprite); }

    void update() override {
        sprite.checkAspect({ 800, 600 });
        sprite.setPosition({ 0.2, 0.2 });

        if (m_isDebugMenuOn) {
            ImGui::Begin("Debug Menu");
            ImGui::Text("FPS = %.1f ", ImGui::GetIO().Framerate);

            ImGui::SliderInt("FPS", &m_framerate, 60, 500);
            if (ImGui::Button("Apply FPS")) { getEngineInstance()->setFramerate(m_framerate); }

            if (ImGui::Button((getEngineInstance()->getVSync() ? "Disable"s : "Enable"s + " VSync")
                                  .c_str())) {
                getEngineInstance()->setVSync(!getEngineInstance()->getVSync());
            }

            ImGui::End();
        }
    }
};

IGame* createGame(IEngine* engine) {
    if (engine == nullptr) throw std::runtime_error{ "Error : createGame : engine is nullptr"s };
    return new PirateGame{};
}

void destroyGame(IGame* game) {
    if (game == nullptr) throw std::runtime_error{ "Error : destroyGame : game is nullptr"s };
    delete game;
}