#ifndef ENGINE_PREPARE_TO_GAME_TREASURE_HXX
#define ENGINE_PREPARE_TO_GAME_TREASURE_HXX

#include <sprite.hxx>
#include <structures.hxx>

class Treasure
{
private:
    Sprite m_treasureSprite;
    Sprite m_xMarkSprite;

    Position m_position{};
    Size m_size{};

public:
    Treasure(const fs::path& treasureTexPath, const fs::path& xMarkTexPath, Size size);

    void resizeUpdate();
    void setPosition(Position position);
    [[nodiscard]] Position getPosition() const noexcept;

    Sprite& getTreasureSprite() noexcept;
    Sprite& getXMarkSprite() noexcept;
};

#endif // ENGINE_PREPARE_TO_GAME_TREASURE_HXX
