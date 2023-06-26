#include <algorithm>
#include <array>
#include <chrono>
#include <engine.hxx>
#include <stdexcept>

#include "island.hxx"
#include "map.hxx"
#include "ship.hxx"

#pragma clang diagnostic push
#pragma ide diagnostic ignored "NullDereference"
using namespace std::literals;

class PirateGame : public IGame
{
private:
    Ship* ship{};
    Map* map{};

    std::unordered_map<std::string, Sprite> m_islandSprites{};
    std::unordered_map<char, std::string> m_charToIslandString{};

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
        delete map;
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
        ship = new Ship{ "data/assets/ship.png", { 100, 100 } };
        map = new Map{ "data/assets/water.png", "data/assets/air.png", { 50, 50 }, { 8000, 8000 } };
        map->addIsland({ 400, 400 },
                       { { "########SSSS###" },
                         { "######SSBGGGS##" },
                         { "##SSBBBGGGRGB##" },
                         { "#SRBBRRGRRPGGS#" },
                         { "##SBBGGGGRPPGS#" },
                         { "###GPGBSSSGGBS#" },
                         { "##SBB#####BRGS#" },
                         { "##BBS#####BGGS#" },
                         { "##BGG#####BGPS#" },
                         { "#SBGGBB##SGRGS#" },
                         { "#SGRGRB##SBGRS#" },
                         { "##GGPGB##SBGS##" },
                         { "##GGGGRS##SSS##" },
                         { "##BBGBB########" },
                         { "###SBS#########" } });

        map->addIsland({ 1500, 0 },
                       { { "###############" },
                         { "#####S#########" },
                         { "#####SS########" },
                         { "#####RB##SS####" },
                         { "#####SB##SB####" },
                         { "#####SB##SBS###" },
                         { "######S##SRS###" },
                         { "#SS###S###SS#S#" },
                         { "##SS#########S#" },
                         { "##SS########SS#" },
                         { "###SSS####SSSR#" },
                         { "####SRSSSSBSS##" },
                         { "####SBBRRBRS###" },
                         { "######SRSS#####" },
                         { "###############" } });

        map->addIsland({ 0, 2000 },
                       { { "###############" },
                         { "###############" },
                         { "###########SS##" },
                         { "#SS#######SS###" },
                         { "##B#####SBBB###" },
                         { "##RS##SRBPB####" },
                         { "##SGBSBBGG#####" },
                         { "###GPGGPGB#####" },
                         { "###SRGGGRR#####" },
                         { "####GGPRGSS####" },
                         { "####SS#RGPGS###" },
                         { "########SGGS###" },
                         { "#########RB####" },
                         { "#########SS####" },
                         { "###############" } });

        map->addIsland({ 1600, 1500 },
                       { { "###############" },
                         { "#####SSSSSS####" },
                         { "###SSBGBBRSS###" },
                         { "###GGRPGGBBBS##" },
                         { "#SGGGGGGGGPGSS#" },
                         { "#SRGPGGGGGGGGS#" },
                         { "##RGGBBRBGGPGS#" },
                         { "##SGGGSSSSBBS##" },
                         { "##SPPGS###SSS##" },
                         { "##SBGGPS#######" },
                         { "###SGGGS#######" },
                         { "###SSGGG#######" },
                         { "###SSBBBS######" },
                         { "#####SBRBS#####" },
                         { "#######SB######" } });

        map->addIsland({ 2200, 700 },
                       { { "#####SSSSS#####" },
                         { "##SSBBGGGBSS###" },
                         { "#SBBGPGGPGRBBS#" },
                         { "#BBGGGGPGGGGGB#" },
                         { "SRBGGGGGGGGPGGS" },
                         { "SGGGPBGGRBGGGGB" },
                         { "SGPGGBSSSSBGPGB" },
                         { "SGGGS####SSBGGS" },
                         { "#SGGS#####SBGGS" },
                         { "#SSGS######BPG#" },
                         { "##SBS#####SBSS#" },
                         { "##SBBS####SGS##" },
                         { "###SRS####SSS##" },
                         { "####S######S###" },
                         { "###############" } });

        map->addIsland({ 3000, 1000 },
                       { { "###############" },
                         { "#########S#####" },
                         { "#######SBBS####" },
                         { "########SRGS###" },
                         { "#####SS##BGGS##" },
                         { "###SRSBS#SGBR##" },
                         { "##SSBBBS##SSS##" },
                         { "###SSS#####S###" },
                         { "###############" },
                         { "#######SSSS####" },
                         { "##SSBBSGPBRSS##" },
                         { "##SBRBGGBSSS###" },
                         { "###SSSSBS######" },
                         { "#######S#######" },
                         { "###############" } });

        map->addIsland({ 600, 3000 },
                       { { "###############" },
                         { "##SS###########" },
                         { "#SBS###########" },
                         { "#SGB###########" },
                         { "##PGS##########" },
                         { "#SBGS##########" },
                         { "#SBPS##########" },
                         { "#SGGRS#########" },
                         { "##SGGGG########" },
                         { "###BBGPGSSSSSS#" },
                         { "###SSBRGGPRBS##" },
                         { "####SSSSSSSS###" },
                         { "###############" },
                         { "###############" },
                         { "###############" } });

        map->addIsland({ 2000, 3500 },
                       { { "###############" },
                         { "############S##" },
                         { "##########SRSS#" },
                         { "#####SSRRRBBRS#" },
                         { "#####SRRBBBRS##" },
                         { "####SSBBBRS####" },
                         { "#####SRRBR#####" },
                         { "######SBBS#####" },
                         { "#####SSRSS#####" },
                         { "##SSSRBRS######" },
                         { "#SBRRRBSS######" },
                         { "#SSBRBBBS######" },
                         { "####SSRRB######" },
                         { "#####SSRRS#####" },
                         { "###############" } });

        map->addIsland({ 4000, 0 },
                       { { "###############" },
                         { "####SSRRS######" },
                         { "###SSBBRBBS####" },
                         { "####SRRRRBBB###" },
                         { "#####SS#SSSBBS#" },
                         { "##########BRBSS" },
                         { "##SSSS####SRRSS" },
                         { "##SBBRS##SSBBSS" },
                         { "#SSRBBS#SSBBRS#" },
                         { "##SBBRS#SBRBRR#" },
                         { "##SBBRS##BBBBS#" },
                         { "###BRBS###SSSS#" },
                         { "####SSS########" },
                         { "###############" },
                         { "###############" } });

        Size size{ 50, 50 };
        m_islandSprites.try_emplace("sand", "data/assets/sand.png", size);
        m_islandSprites.try_emplace("sand_with_grass", "data/assets/sand_and_grass.png", size);
        m_islandSprites.try_emplace("grass", "data/assets/grass.png", size);
        m_islandSprites.try_emplace("rock", "data/assets/rocks.png", size);
        m_islandSprites.try_emplace("palm", "data/assets/palm.png", size);

        m_charToIslandString['S'] = "sand";
        m_charToIslandString['B'] = "sand_with_grass";
        m_charToIslandString['G'] = "grass";
        m_charToIslandString['R'] = "rock";
        m_charToIslandString['P'] = "palm";

        Island::setIslandTiles(m_islandSprites);
        Island::setIslandPattern(m_charToIslandString);
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

            if (event.keyboard.key == Event::Keyboard::Key::l_shift) {
                ship->config().moveMaxSpeed *= 2;
                ship->config().moveAcceleration *= 2;
                ship->config().moveDeceleration *= 2;
                break;
            }

            if (event.keyboard.key == Event::Keyboard::Key::z) {
                ship->config().moveMaxSpeed /= 2;
                ship->config().moveAcceleration /= 2;
                ship->config().moveDeceleration /= 2;
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
        map->render(m_view);
    }

    void update() override {
        static auto time{ std::chrono::steady_clock::now() };
        auto now{ std::chrono::steady_clock::now() };
        std::chrono::microseconds timeElapsed{
            std::chrono::duration_cast<std::chrono::microseconds>(now - time)
        };
        time = now;
        ship->update(timeElapsed);
        map->update();
        m_view.setPosition(ship->getSprite().getPosition());
        m_view.setScale(m_scale);

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

                ImGui::Text("pos x: %.1f\npos y: %.1f\nangle: %.1f\nspeed: %.1f\n"
                            "width %.1f\nheight: %.1f\nwindowW %d\nwindowH %d",
                            ship->getSprite().getPosition().x,
                            ship->getSprite().getPosition().y,
                            ship->getSprite().getRotate().getInDegrees(),
                            ship->getMoveSpeed(),
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