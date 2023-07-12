#ifndef ENGINE_PREPARE_TO_GAME_PLAYER_HXX
#define ENGINE_PREPARE_TO_GAME_PLAYER_HXX

#include <array>
#include <audio.hxx>
#include <memory>
#include <sprite.hxx>
#include <structures.hxx>
#include <unordered_map>

class Player
{
private:
    Position m_position{};
    Position m_lastPosition{};

    std::unordered_map<std::string, std::array<Texture, 3>> m_textures{};
    inline static constexpr float s_animBoost{ 8.0f };
    float m_animUpIndex{};
    float m_animLeftIndex{};
    float m_animRightIndex{};
    float m_animDownIndex{};

    bool m_isMoveUp{};
    bool m_isMoveLeft{};
    bool m_isMoveRight{};
    bool m_isMoveDown{};

    bool m_isDigging{};
    bool m_isNearShip{};

    bool m_hasBottle{};

    inline static constexpr float m_speed{ 35.0f };

    std::unique_ptr<Audio> m_digAudio{};

    int m_money{};

    Sprite m_sprite;

public:
    Player(const fs::path& texturePath, Size size);

    void moveUp();
    void moveDown();
    void moveLeft();
    void moveRight();

    void stopMoveUp();
    void stopMoveLeft();
    void stopMoveRight();
    void stopMoveDown();

    Sprite& getSprite() noexcept;
    [[nodiscard]] const Sprite& getSprite() const noexcept;
    void resizeUpdate();
    void update(std::chrono::microseconds timeElapsed);

    void forceStop();

    void setPosition(Position position);
    [[nodiscard]] Position getPosition() const noexcept;

    void tryDig();
    void stopDig();
    [[nodiscard]] bool isDigging() const noexcept;

    void addMoney(int money);
    [[nodiscard]] int getMoney() const noexcept;
    void clearMoney();

    [[nodiscard]] bool isNearShip() const noexcept;
    void setNearShip(bool nearShip);

    [[nodiscard]] bool hasBottle() const noexcept;
    void setBottle(bool hasBottle);
};

#endif // ENGINE_PREPARE_TO_GAME_PLAYER_HXX
