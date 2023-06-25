#include <algorithm>
#include <array>
#include <chrono>
#include <engine.hxx>
#include <ranges>
#include <stdexcept>

#include "island.hxx"
#include "ship.hxx"

#pragma clang diagnostic push
#pragma ide diagnostic ignored "NullDereference"
using namespace std::literals;

class PirateGame : public IGame
{
private:
    Ship* ship{};
    Island* island{};

    Sprite* secShipForTest{};
    Sprite* water{};
    View m_view{};
    float m_scale{ 1.0f };

    bool m_isDebugMenuOn{ false };
    bool m_debugTankInfo{ false };

    int m_framerate{ 150 };
    std::vector<Position> m_waterPositions{};

    bool m_is_enable_intersect_check{ false };
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
        delete ship;
        delete secShipForTest;
        delete water;
        delete island;
    }

    void initialize() override {
        getEngineInstance()->initialize(R"(
{
    "window_name": "Pirate Game",
    "window_width": 800,
    "window_height": 600,
    "is_window_resizable": true
}
)");

        ImGui::SetCurrentContext(getEngineInstance()->getImGuiContext());
        ship = new Ship{ "/Users/aleksey/lesta-course/prepare_engine_to_game/data/assets/ship.png",
                         { 66, 113 } };

        secShipForTest =
            new Sprite{ "/Users/aleksey/lesta-course/prepare_engine_to_game/data/assets/ship.png",
                        { 84, 94 } };
        water =
            new Sprite{ "/Users/aleksey/lesta-course/prepare_engine_to_game/data/assets/water.png",
                        { 100, 100 } };

        island =
            new Island{ "/Users/aleksey/lesta-course/prepare_engine_to_game/data/assets/sand.png",
                        { 50, 50 },
                        { .xy = { 0, 0 }, .wh = { 500, 500 } },
                        { { "0000000000" },
                          { "0000000000" },
                          { "0000000000" },
                          { "0000##0000" },
                          { "0000##0000" },
                          { "0000##0000" },
                          { "0000##0000" },
                          { "0000##0000" },
                          { "0000##0000" },
                          { "00######00" } } };
    }

    void onEvent(const Event& event) override {
        switch (event.type) {
        case Event::Type::key_down:
            if (event.keyboard.key == Config::move_key) {
                ship->move();
                break;
            }

            if (event.keyboard.key == Config::rotate_left_key) {
                ship->rotateLeft();
                break;
            }

            if (event.keyboard.key == Config::rotate_right_key) {
                ship->rotateRight();
                break;
            }

            if (event.keyboard.key == Event::Keyboard::Key::l_control) {
                m_isDebugMenuOn = !m_isDebugMenuOn;
                break;
            }

            break;
        case Event::Type::key_up:
            if (event.keyboard.key == Config::move_key) {
                ship->stopMove();
                break;
            }

            if (event.keyboard.key == Config::rotate_left_key) {
                ship->stopRotateLeft();
                break;
            }

            if (event.keyboard.key == Config::rotate_right_key) {
                ship->stopRotateRight();
                break;
            }

            break;

        default:
            break;
        }
    }

    void render() override {
        getEngineInstance()->render(ship->getSprite(), m_view);
        // getEngineInstance()->render(*secShipForTest, m_view);

        std::ranges::for_each(island->getPositions(), [&](const auto& pos) {
            if (std::abs(pos.x - ship->getSprite().getPosition().x) <= 500 / m_scale &&
                std::abs(pos.y - ship->getSprite().getPosition().y) <= 400 / m_scale) {
                island->getSprite().setPosition(pos);
                getEngineInstance()->render(island->getSprite(), m_view);
            }
        });

        std::ranges::for_each(m_waterPositions, [&](const auto& pos) {
            if (std::abs(pos.x - ship->getSprite().getPosition().x) <= 500 / m_scale &&
                std::abs(pos.y - ship->getSprite().getPosition().y) <= 400 / m_scale) {
                water->setPosition(pos);
                getEngineInstance()->render(*water, m_view);
            }
        });
    }

    void update() override {
        static auto time{ std::chrono::steady_clock::now() };
        std::chrono::milliseconds timeElapsed{
            std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() -
                                                                  time)
        };
        ship->update(timeElapsed);
        water->updateWindowSize();
        water->checkAspect({ 800, 600 });

        float spriteSize = 100.0f;
        float xOffset = -((800 / 2.0f) - (spriteSize / 2.0f));
        float yOffset = -((600 / 2.0f) - (spriteSize / 2.0f));
        m_waterPositions.clear();
        for (std::ptrdiff_t i = -2000 / 100; i < 2000 / 100; ++i) {
            for (std::ptrdiff_t j = -2000 / 100; j < 2000 / 100; ++j) {
                float xPos = xOffset + (i * spriteSize);
                float yPos = yOffset + (j * spriteSize);
                m_waterPositions.push_back({ xPos, yPos });
            }
        }

        //      water->setScale({ 800.f / getEngineInstance()->getWindowSize().width,
        //                       600.f / getEngineInstance()->getWindowSize().height });
        //  water->setScale({ 1.0f, 600.f / getEngineInstance()->getWindowSize().height });

        time = std::chrono::steady_clock::now();

        auto lastPost{ ship->getSprite().getPosition() };
        m_view.setPosition(ship->getSprite().getPosition());
        m_view.setScale(m_scale);

        auto is{ intersect(ship->getSprite(), *secShipForTest) };
        //    if (is && m_is_enable_intersect_check) { sprite->setPosition(lastPost); }

        // sprite.updateWindowSize();
        //    water.updateWindowSize();
        // sprite.checkAspect({ 800, 600 });
        //    water.checkAspect({ 800, 600 });
        // sprite.setPosition({ 0.2, 0.2 });

        if (m_isDebugMenuOn) {
            ImGui::Begin("Debug Menu");
            ImGui::Text("FPS = %.1f ", ImGui::GetIO().Framerate);

            ImGui::SliderInt("FPS", &m_framerate, 60, 300);
            if (ImGui::Button("Apply FPS")) { getEngineInstance()->setFramerate(m_framerate); }

            if (ImGui::Button((getEngineInstance()->getVSync() ? "Disable"s : "Enable"s + " VSync")
                                  .c_str())) {
                getEngineInstance()->setVSync(!getEngineInstance()->getVSync());
            }

            ImGui::Checkbox("Debug Ship info", &m_debugTankInfo);
            if (m_debugTankInfo) {
                ImGui::Checkbox("Intersect", &m_is_enable_intersect_check);

                ImGui::Text("pos x: %.1f\npos y: %.1f\nangle: %.1f\nspeed: %.1f\nintersect: "
                            "%d\nwidth %.1f\nheight: %.1f\nwindowW %d\nwindowH %d",
                            ship->getSprite().getPosition().x,
                            ship->getSprite().getPosition().y,
                            ship->getSprite().getRotate().getInDegrees(),
                            ship->getMoveSpeed(),
                            is.has_value(),
                            ship->getSprite().getSize().width,
                            ship->getSprite().getSize().height,
                            getEngineInstance()->getWindowSize().width,
                            getEngineInstance()->getWindowSize().height);
            }

            ImGui::SliderFloat("camera height", &m_scale, 0.1f, 10.f);

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
#pragma clang diagnostic pop