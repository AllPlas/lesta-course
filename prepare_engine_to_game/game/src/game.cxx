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
    Sprite* sprite{};
    Sprite* crate{};
    View m_view{};

    bool m_isDebugMenuOn{ false };
    bool m_debugTankInfo{ false };

    int m_framerate{ 150 };
    std::vector<Sprite::Position> m_cratesPositions{};

    float acceleration = 20.f; // Ускорение объекта
    float deceleration = 2 * acceleration;
    float maxSpeed = 100.0f;   // Максимальная скорость объекта
    float currentSpeed = 0.0f; // Текущая скорость объекта

    bool m_isMove{ false };
    bool is_binding_key = false;
    Event::Keyboard::Key* binding_key = nullptr;
    ImGuiKey m_key{};

    struct Config
    {
        Config() = delete;
        inline static Event::Keyboard::Key move_key{ Event::Keyboard::Key::w };
        inline static Event::Keyboard::Key rotate_left_key{ Event::Keyboard::Key::a };
        inline static Event::Keyboard::Key rotate_right_key{ Event::Keyboard::Key::d };
        inline static Event::Keyboard::Key interact_key{ Event::Keyboard::Key::space };
    };

public:
    ~PirateGame() noexcept override {
        delete sprite;
        delete crate;
    }

    void initialize() override {
        getEngineInstance()->initialize(R"(
{
    "window_name": "Pirate Game",
    "window_width": 800,
    "window_height": 600,
    "is_window_resizable": false
}
)");

        ImGui::SetCurrentContext(getEngineInstance()->getImGuiContext());
        sprite =
            new Sprite{ "/Users/aleksey/lesta-course/prepare_engine_to_game/data/assets/ship.png",
                        { 84, 94 } };
        crate =
            new Sprite{ "/Users/aleksey/lesta-course/prepare_engine_to_game/data/assets/water.png",
                        { 100, 100 } };

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

            if (event.keyboard.key == Config::move_key) { m_isMove = true; }

            if (event.keyboard.key == Config::rotate_left_key) {
                sprite->setRotate(sprite->getRotate() + 1);
                if (sprite->getRotate() > 360) sprite->setRotate(sprite->getRotate() - 360);
            }

            if (event.keyboard.key == Config::rotate_right_key) {
                sprite->setRotate(sprite->getRotate() - 1);
                if (sprite->getRotate() < 0) sprite->setRotate(sprite->getRotate() + 360);
            }

            if (event.keyboard.key == Event::Keyboard::Key::l_shift)
                sprite->setScale(sprite->getScale() == 0.5f ? 1.0f : 0.5f);
        }

        if (event.type == Event::Type::key_up) {
            if (event.keyboard.key == Config::move_key) { m_isMove = false; }
        }
    }

    void render() override {
        getEngineInstance()->render(*sprite, m_view);
        std::ranges::for_each(m_cratesPositions, [&](const auto& pos) {
            crate->setPosition(pos);
            getEngineInstance()->render(*crate, m_view);
        });
    }

    void update() override {
        static auto time{ std::chrono::steady_clock::now() };
        float timeElapsed =
            std::chrono::duration<float, std::ratio<1>>((std::chrono::steady_clock::now() - time))
                .count();
        time = std::chrono::steady_clock::now();

        if (m_isMove) {
            currentSpeed += acceleration * timeElapsed;
            if (currentSpeed > maxSpeed) { currentSpeed = maxSpeed; }
        }
        else {
            currentSpeed -= deceleration * timeElapsed;
            if (currentSpeed < 0) currentSpeed = 0;
        }
        float deltaX = 0.0f;
        float deltaY = currentSpeed * timeElapsed;

        // Предполагая, что angle - угол в градусах
        float angleInRadians = sprite->getRotate() * (std::numbers::pi / 180.0);

        // Вычисление новых координат
        float newX = sprite->getPosition().x + deltaX * std::cos(angleInRadians) -
                     deltaY * std::sin(angleInRadians);
        float newY = sprite->getPosition().y + deltaX * std::sin(angleInRadians) +
                     deltaY * std::cos(angleInRadians);

        sprite->setPosition({ newX, newY });
        m_view.setPosition(sprite->getPosition().x / 400, sprite->getPosition().y / 300);
        m_view.setScale(0.5);

        //  sprite.updateWindowSize();
        //    crate.updateWindowSize();
        // sprite.checkAspect({ 800, 600 });
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

            ImGui::Checkbox("Debug Ship info", &m_debugTankInfo);
            if (m_debugTankInfo) {
                ImGui::Text("pos x: %.1f\npos y: %.1f\nangle: %.1f\nspeed: %.1f",
                            sprite->getPosition().x,
                            sprite->getPosition().y,
                            sprite->getRotate(),
                            currentSpeed);
            }

            ImGui::Text("Move key: %s", ImGui::GetKeyName(m_key));
            auto it = ImGui::GetKeyIndex(ImGuiKey_D);
            ImGui::SameLine();
            if (ImGui::Button("change key")) { is_binding_key = true; }
            if (is_binding_key) {
                ImGui::Text("Waiting for keyboard input...");
                for (ImGuiKey key{ ImGuiKey_NamedKey_BEGIN }; key < ImGuiKey_NamedKey_END;
                     key = static_cast<ImGuiKey>(key + 1)) {
                    if (ImGui::IsKeyPressed(key)) {
                        m_key = key;

                        if (m_key == ImGuiKey_D) Config::move_key = Event::Keyboard::Key::d;

                        is_binding_key = false;
                        break;
                    }
                }
            }

            ImGui::End();
        }
    }

    void setupKeyBinds() {}
};

IGame* createGame(IEngine* engine) {
    if (engine == nullptr) throw std::runtime_error{ "Error : createGame : engine is nullptr"s };
    return new PirateGame{};
}

void destroyGame(IGame* game) {
    if (game == nullptr) throw std::runtime_error{ "Error : destroyGame : game is nullptr"s };
    delete game;
}