#ifndef ENGINE_PREPARE_TO_GAME_SHIP_HXX
#define ENGINE_PREPARE_TO_GAME_SHIP_HXX

#include <chrono>
#include <filesystem>
#include <sprite.hxx>

#include "player.hxx"

namespace fs = std::filesystem;

class Ship final
{
public:
    struct Config
    {
        float moveAcceleration{ 20.0f };
        float moveDeceleration{ 2 * moveAcceleration };
        float moveMaxSpeed{ 100.0f };

        float rotateAcceleration{ 20.f };
        float rotateDeceleration{ 2 * rotateAcceleration };
        float rotateMaxSpeed{ 40.f };
    };

private:
    Position m_position{};
    Position m_lastPosition{};

    bool m_isMove{};
    bool m_isRotateLeft{};
    bool m_isRotateRight{};
    bool m_isInteract{};

    Config m_config{};
    float m_currentMoveSpeed{};
    float m_currentRotateSpeed{};

    Sprite m_sprite;

    Player& m_player;

public:
    Ship(const fs::path& textureFilepath, Size size, Player& player);

    void move();
    void rotateLeft();
    void rotateRight();

    void stopMove();
    void stopRotateLeft();
    void stopRotateRight();

    void forceStop();
    void setInteract(bool isInteract);

    Config& config() noexcept;
    Player& getPlayer() noexcept;

    void resizeUpdate();
    void update(std::chrono::microseconds timeElapsed);

    [[nodiscard]] const Sprite& getSprite() const noexcept;
    [[nodiscard]] float getMoveSpeed() const noexcept;
    [[nodiscard]] float getRotateSpeed() const noexcept;
    [[nodiscard]] bool isInteract() const noexcept;
    [[nodiscard]] Position getPosition() const noexcept;
};

#endif // ENGINE_PREPARE_TO_GAME_SHIP_HXX