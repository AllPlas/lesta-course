#ifndef ENGINE_PREPARE_TO_GAME_BOTTLE_HXX
#define ENGINE_PREPARE_TO_GAME_BOTTLE_HXX

#include <engine.hxx>
#include <sprite.hxx>
#include <structures.hxx>

#include "ship.hxx"

class Bottle
{
private:
    Sprite m_sprite;

    Position m_position{};
    Size m_size{};

public:
    Bottle(const fs::path& texturePath, Size size);

    Sprite& getSprite() noexcept;
    void interact(Ship& ship);

    void resizeUpdate();
    void setPosition(Position position);
    [[nodiscard]] Position getPosition() const noexcept;

    friend bool intersect(const Bottle& bottle, const Ship& ship);
};

bool intersect(const Bottle& bottle, const Ship& ship);

#endif // ENGINE_PREPARE_TO_GAME_BOTTLE_HXX
