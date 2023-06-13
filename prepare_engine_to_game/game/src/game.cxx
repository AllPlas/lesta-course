#include <algorithm>
#include <array>
#include <chrono>
#include <engine.hxx>
#include <ranges>
#include <stdexcept>

using namespace std::literals;

class PirateGame : public IGame
{
private:
    Sprite sprite{ "/Users/aleksey/Downloads/kenney_pirate-pack/PNG/Default size/Ships/ship (14).png",
                   { 84, 92 } };

    Sprite crate{ "/Users/aleksey/Downloads/Water Top.png", { 100, 100 } };

    bool m_isDebugMenuOn{ false };
    bool m_debugTankInfo{ false };

    int m_framerate{ 150 };
    std::vector<Sprite::Position> m_cratesPositions{};

    float acceleration = 1.f;  // Ускорение объекта
    float maxSpeed = 100.0f;   // Максимальная скорость объекта
    float currentSpeed = 0.0f; // Текущая скорость объекта

public:
    void initialize() override {
        ImGui::SetCurrentContext(getEngineInstance()->getImGuiContext());

        float screenCenterX = 800.0f / 2.0f;
        float screenCenterY = 600.0f / 2.0f;
        float spriteSize = 100.0f;
        float xOffset = -((800.0f / 2.0f) - (spriteSize / 2.0f));
        float yOffset = -((600.0f / 2.0f) - (spriteSize / 2.0f));

        for (std::size_t i = 0; i < 800 / 100; ++i) {
            for (std::size_t j = 0; j < 600 / 100; ++j) {
                float xPos = xOffset + (i * spriteSize);
                float yPos = yOffset + (j * spriteSize);
                m_cratesPositions.push_back({ xPos, yPos });
            }
        }
    }

    void onEvent(const Event& event) override {
        if (event.type == Event::Type::key_down) {
            if (event.keyboard.key == Event::Keyboard::Key::l_control)
                m_isDebugMenuOn = !m_isDebugMenuOn;

            if (event.keyboard.key == Event::Keyboard::Key::w) {
                currentSpeed += acceleration;
                if (currentSpeed > maxSpeed) { currentSpeed = maxSpeed; }
            }

            if (event.keyboard.key == Event::Keyboard::Key::l_shift) sprite.setScale({ 0.5, 0.5 });
        }

        if (event.type == Event::Type::key_up) {
            if (event.keyboard.key == Event::Keyboard::Key::w) { currentSpeed = 0; }
        }
    }

    void render() override {
        getEngineInstance()->render(sprite);
        std::ranges::for_each(m_cratesPositions, [&](const auto& pos) {
            crate.setPosition(pos);
            getEngineInstance()->render(crate);
        });
    }

    void update() override {
        static auto time{ std::chrono::steady_clock::now() };
        float timeElapsed =
            std::chrono::duration<float, std::ratio<1>>((std::chrono::steady_clock::now() - time))
                .count();
        time = std::chrono::steady_clock::now();

        float deltaY = currentSpeed * timeElapsed;
        sprite.setPosition({ sprite.getPosition().x, sprite.getPosition().y + deltaY });

        sprite.updateWindowSize();
        //    crate.updateWindowSize();
        sprite.checkAspect({ 800, 600 });
        //    crate.checkAspect({ 800, 600 });
        // sprite.setPosition({ 0.2, 0.2 });

        if (m_isDebugMenuOn) {
            ImGui::Begin("Debug Menu");
            ImGui::Text("FPS = %.1f ", ImGui::GetIO().Framerate);

            ImGui::SliderInt("FPS", &m_framerate, 60, 500);
            if (ImGui::Button("Apply FPS")) { getEngineInstance()->setFramerate(m_framerate); }

            if (ImGui::Button((getEngineInstance()->getVSync() ? "Disable"s : "Enable"s + " VSync")
                                  .c_str())) {
                getEngineInstance()->setVSync(!getEngineInstance()->getVSync());
            }

            ImGui::Checkbox("Debug Tank info", &m_debugTankInfo);
            if (m_debugTankInfo) {
                ImGui::Text(
                    "pos x: %.1f\npos y: %.1f", sprite.getPosition().x, sprite.getPosition().y);
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