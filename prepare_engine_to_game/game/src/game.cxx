#include <array>
#include <chrono>
#include <engine.hxx>
#include <stdexcept>

#include "island.hxx"
#include "map.hxx"
#include "player.hxx"
#include "ship.hxx"

#pragma clang diagnostic push
#pragma ide diagnostic ignored "NullDereference"
using namespace std::literals;

class PirateGame final : public IGame
{
private:
    Ship* ship{};
    Player* player{};
    Map* map{};

    Texture* coin{};

    std::unordered_map<std::string, Sprite> m_islandSprites{};
    std::unordered_map<char, std::string> m_charToIslandString{};

    bool m_isOnShip{ true };
    bool m_viewOnTreasure{};

    View m_view{};
    float m_scale{ 1.0f };

    bool m_isDebugMenuOn{ false };
    bool m_debugTankInfo{ false };

    int m_framerate{ 150 };

    bool is_binding_key = false;
    Event::Keyboard::Key* binding_key = nullptr;
    ImGuiKey m_key{};

    struct Config
    {
        Config() = delete;
        inline static Event::Keyboard::Key ship_move_key{ Event::Keyboard::Key::w };
        inline static Event::Keyboard::Key ship_rotate_left_key{ Event::Keyboard::Key::a };
        inline static Event::Keyboard::Key ship_rotate_right_key{ Event::Keyboard::Key::d };
        inline static Event::Keyboard::Key interact_key{ Event::Keyboard::Key::e };
        inline static Event::Keyboard::Key player_move_up_key{ Event::Keyboard::Key::w };
        inline static Event::Keyboard::Key player_move_left_key{ Event::Keyboard::Key::a };
        inline static Event::Keyboard::Key player_move_right_key{ Event::Keyboard::Key::d };
        inline static Event::Keyboard::Key player_move_down_key{ Event::Keyboard::Key::s };
        inline static Event::Keyboard::Key view_treasure_key{ Event::Keyboard::Key::space };
        inline static Event::Keyboard::Key dig_treasure_key{ Event::Keyboard::Key::f };
    };

public:
    PirateGame() = default;

    PirateGame(const PirateGame& pg) = delete;
    PirateGame& operator=(const PirateGame& pg) = delete;
    PirateGame(PirateGame&&) = delete;
    PirateGame& operator=(PirateGame&&) = delete;

    ~PirateGame() noexcept override {
        delete ship;
        delete player;
        delete map;
        delete coin;
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
        ship = new Ship{ "data/assets/ship.png", { 66, 113 } };
        player = new Player{ "data/assets/pirate/front/front_standing.png", { 30, 30 } };
        map = new Map{ "data/assets/water.png",
                       "data/assets/air.png",
                       "data/assets/bottle.png",
                       "data/assets/treasure.png",
                       "data/assets/xmark.png",
                       { 50, 50 },
                       { 8000, 8000 } };
        coin = new Texture{};
        coin->load("data/assets/coin.png");
        map->generateBottle();
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
        m_islandSprites.try_emplace("sand_with_grass", "data/assets/sand_with_grass.png", size);
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
            if (m_isOnShip) {
                if (event.keyboard.key == Config::ship_move_key) {
                    ship->move();
                    break;
                }

                if (event.keyboard.key == Config::ship_rotate_left_key) {
                    ship->rotateLeft();
                    ship->setInteract(false);
                    break;
                }

                if (event.keyboard.key == Config::ship_rotate_right_key) {
                    ship->rotateRight();
                    ship->setInteract(false);
                    break;
                }
            }
            else {
                if (event.keyboard.key == Config::player_move_up_key) {
                    player->moveUp();
                    break;
                }

                if (event.keyboard.key == Config::player_move_left_key) {
                    player->moveLeft();
                    break;
                }

                if (event.keyboard.key == Config::player_move_right_key) {
                    player->moveRight();
                    break;
                }

                if (event.keyboard.key == Config::player_move_down_key) {
                    player->moveDown();
                    break;
                }

                if (event.keyboard.key == Config::dig_treasure_key) {
                    player->tryDig();
                    break;
                }
            }

            if (event.keyboard.key == Config::interact_key) {
                if (ship->isInteract() && m_isOnShip) {
                    m_isOnShip = false;
                    player->setPosition(
                        { ship->getPosition().x -
                              10.f * std::sin(ship->getSprite().getRotate().getInRadians()),
                          ship->getPosition().y +
                              10.f * std::cos(ship->getSprite().getRotate().getInRadians()) });
                    ship->stopMove();
                    ship->stopRotateRight();
                    ship->stopRotateLeft();
                }
                else if (player->isNearShip()) {
                    m_isOnShip = true;
                    player->stopMoveDown();
                    player->stopMoveRight();
                    player->stopMoveLeft();
                    player->stopMoveUp();
                }
                break;
            }

            if (event.keyboard.key == Config::view_treasure_key && !map->hasBottle()) {
                m_viewOnTreasure = !m_viewOnTreasure;
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
            if (m_isOnShip) {
                if (event.keyboard.key == Config::ship_move_key) {
                    ship->stopMove();
                    break;
                }

                if (event.keyboard.key == Config::ship_rotate_left_key) {
                    ship->stopRotateLeft();
                    break;
                }

                if (event.keyboard.key == Config::ship_rotate_right_key) {
                    ship->stopRotateRight();
                    break;
                }
            }
            else {
                if (event.keyboard.key == Config::player_move_up_key) {
                    player->stopMoveUp();
                    break;
                }

                if (event.keyboard.key == Config::player_move_left_key) {
                    player->stopMoveLeft();
                    break;
                }

                if (event.keyboard.key == Config::player_move_right_key) {
                    player->stopMoveRight();
                    break;
                }

                if (event.keyboard.key == Config::player_move_down_key) {
                    player->stopMoveDown();
                    break;
                }
            }

            break;

        case Event::Type::window_resized:
            map->resizeUpdate();
            ship->resizeUpdate();
            player->resizeUpdate();
            break;

        default:
            break;
        }
    }

    void render() override {
        if (m_viewOnTreasure) {
            getEngineInstance()->render(map->getTreasure().getXMarkSprite(), m_view);
        }
        else {
            if (!m_isOnShip) getEngineInstance()->render(player->getSprite(), m_view);
            getEngineInstance()->render(ship->getSprite(), m_view);
            if (map->isTreasureUnearthed())
                getEngineInstance()->render(map->getTreasure().getTreasureSprite(), m_view);
        }

        map->render(m_view);
    }

    void update() override {
        static auto time{ std::chrono::steady_clock::now() };
        auto now{ std::chrono::steady_clock::now() };
        std::chrono::microseconds timeElapsed{
            std::chrono::duration_cast<std::chrono::microseconds>(now - time)
        };
        time = now;

        if (!m_viewOnTreasure) {
            if (m_isOnShip) {
                map->interact(*ship);
                ship->update(timeElapsed);
            }
            else {
                map->interact(*player);
                player->update(timeElapsed);
            }

            if (m_isOnShip)
                m_view.setPosition(ship->getPosition());
            else
                m_view.setPosition(player->getPosition());
        }
        else
            m_view.setPosition(map->getTreasure().getPosition());

        m_view.setScale(m_scale);

        Position viewPos{ m_view.getPosition() };
        if (m_view.getPosition().x <
            getEngineInstance()->getWindowSize().width / 2.0f / m_view.getScale() - 400.f)
            viewPos.x =
                getEngineInstance()->getWindowSize().width / 2.0f / m_view.getScale() - 400.f;
        if (m_view.getPosition().y <
            getEngineInstance()->getWindowSize().height / 2.0f / m_view.getScale() - 300.f)
            viewPos.y =
                getEngineInstance()->getWindowSize().height / 2.0f / m_view.getScale() - 300.f;
        if (m_view.getPosition().x >
            8000 - getEngineInstance()->getWindowSize().width / 2.0f / m_view.getScale() - 400.f)
            viewPos.x = 8000 -
                        getEngineInstance()->getWindowSize().width / 2.0f / m_view.getScale() -
                        400.f;
        if (m_view.getPosition().y >
            8000 - getEngineInstance()->getWindowSize().height / 2.0f / m_view.getScale() - 300.f)
            viewPos.y = 8000 -
                        getEngineInstance()->getWindowSize().height / 2.0f / m_view.getScale() -
                        300.f;

        m_view.setPosition(viewPos);

        ImGui::SetNextWindowPos({ getEngineInstance()->getWindowSize().width - 150.0f, 0.0f });
        ImGui::Begin("_",
                     nullptr,
                     ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoMove |
                         ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize);

        auto tex = reinterpret_cast<ImTextureID>(coin->operator*());
        ImVec2 cursorPos{ ImGui::GetCursorPos() };
        ImGui::SetCursorPosY(cursorPos.y + 4);
        ImVec2 texSize(28, 28);

        ImGui::PushItemWidth(150);

        ImGui::Image(tex, texSize);
        ImGui::SameLine();

        ImGui::SetCursorPosY(cursorPos.y);
        ImGui::SetWindowFontScale(2.5f);
        ImGui::Text("%d", player->getMoney());
        ImGui::PopItemWidth();
        ImGui::End();

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

                        if (m_key == ImGuiKey_D) Config::ship_move_key = Event::Keyboard::Key::d;

                        is_binding_key = false;
                        break;
                    }
                }
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
#pragma clang diagnostic pop