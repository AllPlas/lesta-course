#ifndef ENGINE_PREPARE_TO_GAME_SHIP_HXX
#define ENGINE_PREPARE_TO_GAME_SHIP_HXX

#include <chrono>
#include <filesystem>
#include <sprite.hxx>

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
    bool m_isMove{};
    bool m_isRotateLeft{};
    bool m_isRotateRight{};

    Config m_config{};
    float m_currentMoveSpeed{};
    float m_currentRotateSpeed{};

    Sprite m_sprite;

public:
    Ship(const fs::path& textureFilepath, Size size);

    void move();
    void rotateLeft();
    void rotateRight();

    void stopMove();
    void stopRotateLeft();
    void stopRotateRight();

    Config& config() noexcept;

    void resizeUpdate();
    void update(std::chrono::microseconds timeElapsed);

    [[nodiscard]] const Sprite& getSprite() const noexcept;
    [[nodiscard]] float getMoveSpeed() const noexcept;
    [[nodiscard]] float getRotateSpeed() const noexcept;
};

#endif // ENGINE_PREPARE_TO_GAME_SHIP_HXX