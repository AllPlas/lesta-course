#include <array>
#include <chrono>
#include <engine.hxx>
#include <memory>
#include <stdexcept>

#include "config.hxx"
#include "island.hxx"
#include "map.hxx"
#include "menu.hxx"
#include "player.hxx"
#include "ship.hxx"

#pragma clang diagnostic push
#pragma ide diagnostic ignored "NullDereference"
using namespace std::literals;

class PirateGame final : public IGame
{
private:
    inline static constexpr Size s_originalWindowSize{ 800, 600 };
    inline static constexpr int s_deltaForTouches{ 50 };

    int m_framerate{ 150 };

    std::unique_ptr<Player> player{};
    std::unique_ptr<Ship> ship{};
    std::unique_ptr<Map> map{};
    std::unique_ptr<Texture> coin{};
    std::unique_ptr<Audio> mainAudio{};

    Menu menu{};

    std::unordered_map<std::string, Sprite> m_islandSprites{};
    std::unordered_map<char, std::string> m_charToIslandString{};

    bool m_isOnShip{ true };
    bool m_viewOnTreasure{};

    View m_view{};

    bool m_isDebugMenuOn{ false };
    bool m_debugTankInfo{ false };

    Rectangle rectMap{};
    Rectangle rectGetOut{};
    Rectangle rectDig{};

public:
    PirateGame() = default;
    ~PirateGame() override = default;

    PirateGame(const PirateGame& pg) = delete;
    PirateGame& operator=(const PirateGame& pg) = delete;
    PirateGame(PirateGame&&) = delete;
    PirateGame& operator=(PirateGame&&) = delete;

    void initialize() override {
        getEngineInstance()->initialize(R"(
{
    "window_name": "Pirate Game",
    "window_width": 800,
    "window_height": 600,
    "is_window_resizable": true
}
)");
        Sprite::setOriginalSize(s_originalWindowSize);

        ImGui::SetCurrentContext(getEngineInstance()->getImGuiContext());
        player =
            std::make_unique<Player>("data/assets/pirate/front/front_standing.png", Size{ 30, 30 });
        ship = std::make_unique<Ship>("data/assets/ship.png", Size{ 66, 113 }, *player.get());
        map = std::make_unique<Map>("data/assets/water.png",
                                    "data/assets/air.png",
                                    "data/assets/bottle.png",
                                    "data/assets/treasure.png",
                                    "data/assets/xmark.png",
                                    Size{ 50, 50 },
                                    Size{ 8000, 8000 });
        coin = std::make_unique<Texture>();
        mainAudio = std::make_unique<Audio>("data/audio/background.wav");
        coin->load("data/assets/coin.png");
        map->generateBottles();
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
        mainAudio->play(true);

        map->resizeUpdate();
        ship->resizeUpdate();
        player->resizeUpdate();
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

            if (event.keyboard.key == Config::view_treasure_key && player->hasBottle()) {
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

            if (event.keyboard.key == Event::Keyboard::Key::escape) {
                if (menu.getBindKey())
                    menu.setBindKey(false);
                else if (menu.getSettingMenu())
                    menu.setSettingMenu(false);
                else
                    menu.setActive(!menu.getActive());
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

        case Event::Type::touch_down:
            if (rectMap.contains(event.touch.pos)) {
                if (player->hasBottle()) m_viewOnTreasure = !m_viewOnTreasure;
                break;
            }

            if (rectGetOut.contains(event.touch.pos)) {
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

            if (rectDig.contains(event.touch.pos) && !m_isOnShip) {
                player->tryDig();
                break;
            }

            break;

        case Event::Type::touch_motion:
            if (event.touch.id == 0) {
                if (m_isOnShip) {
                    switch (static_cast<int>(event.touch.dy)) {
                    case s_deltaForTouches ... INT_MAX:
                        ship->move();
                        break;

                    default:
                        ship->stopMove();
                        break;
                    }

                    switch (static_cast<int>(event.touch.dx)) {
                    case INT_MIN ... - s_deltaForTouches:
                        ship->stopRotateRight();
                        ship->rotateLeft();
                        break;

                    case s_deltaForTouches ... INT_MAX:
                        ship->stopRotateLeft();
                        ship->rotateRight();
                        break;

                    default:
                        ship->stopRotateLeft();
                        ship->stopRotateRight();
                        break;
                    }
                }
                else {
                    switch (static_cast<int>(event.touch.dy)) {
                    case s_deltaForTouches ... INT_MAX:
                        player->stopMoveDown();
                        player->moveUp();
                        break;

                    case INT_MIN ... - s_deltaForTouches:
                        player->stopMoveUp();
                        player->moveDown();
                        break;

                    default:
                        player->stopMoveUp();
                        player->stopMoveDown();
                        break;
                    }

                    switch (static_cast<int>(event.touch.dx)) {
                    case s_deltaForTouches ... INT_MAX:
                        player->stopMoveLeft();
                        player->moveRight();
                        break;

                    case INT_MIN ... - s_deltaForTouches:
                        player->stopMoveRight();
                        player->moveLeft();
                        break;

                    default:
                        player->stopMoveLeft();
                        player->stopMoveRight();
                        break;
                    }
                }
            }
            break;

        case Event::Type::touch_up:
            if (event.touch.id == 0) {
                ship->stopMove();
                ship->stopRotateLeft();
                ship->stopRotateRight();
                player->stopMoveUp();
                player->stopMoveLeft();
                player->stopMoveRight();
                player->stopMoveDown();
            }
            break;

        default:
            break;
        }
    }

    void render() override {
        if (menu.getActive()) {
            menu.render();
            return;
        }

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

#ifdef __ANDROID__
        ImGui::SetNextWindowPos(
            ImVec2(ImGui::GetIO().DisplaySize.x - 110, ImGui::GetIO().DisplaySize.y - 385));
        ImGui::Begin("Menu",
                     nullptr,
                     ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                         ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse |
                         ImGuiWindowFlags_NoBackground);
        ImGui::SetWindowFontScale(2.0f);
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.4f, 0.6f, 1.0f));
        ImGui::Button("Map", { 100, 100 });
        {
            static auto firstXY{ ImGui::GetItemRectMin() };
            static auto secondXY{ ImGui::GetItemRectMax() };

            rectMap.xy.x = firstXY.x;
            rectMap.xy.y = getEngineInstance()->getWindowSize().height - secondXY.y;
            rectMap.wh.width = secondXY.x - firstXY.x;
            rectMap.wh.height = secondXY.y - firstXY.y;
        }

        ImGui::Button(m_isOnShip ? "Ashore" : "Onboard", { 100, 100 });
        {
            static auto firstXY{ ImGui::GetItemRectMin() };
            static auto secondXY{ ImGui::GetItemRectMax() };

            rectGetOut.xy.x = firstXY.x;
            rectGetOut.xy.y = getEngineInstance()->getWindowSize().height - secondXY.y;
            rectGetOut.wh.width = secondXY.x - firstXY.x;
            rectGetOut.wh.height = secondXY.y - firstXY.y;
        }

        ImGui::Button("Dig", { 100, 100 });
        {
            static auto firstXY{ ImGui::GetItemRectMin() };
            static auto secondXY{ ImGui::GetItemRectMax() };

            rectDig.xy.x = firstXY.x;
            rectDig.xy.y = getEngineInstance()->getWindowSize().height - secondXY.y;
            rectDig.wh.width = secondXY.x - firstXY.x;
            rectDig.wh.height = secondXY.y - firstXY.y;
        }
        ImGui::PopStyleColor();
        ImGui::End();
#endif
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

        m_view.setScale(Config::camera_height);

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

            ImGui::SliderFloat("camera height", &Config::camera_height, 0.1f, 10.f);
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