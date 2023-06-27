#include "bottle.hxx"

Bottle::Bottle(const fs::path& texturePath, Size size)
    : m_sprite{ texturePath, size }, m_size{ size } {}

Sprite& Bottle::getSprite() noexcept { return m_sprite; }

void Bottle::interact(Ship& ship) {
    if (intersect(*this, ship)) {}
}

bool intersect(const Bottle& bottle, const Ship& ship) {
    return intersect(bottle.m_sprite, ship.getSprite()).has_value();
}

void Bottle::setPosition(Position position) {
    m_position = position;
    m_sprite.setPosition(m_position);
}

Position Bottle::getPosition() const noexcept { return m_position; }

void Bottle::resizeUpdate() {
    m_sprite.updateWindowSize();
    m_sprite.checkAspect({ 800, 600 });
    m_sprite.setPosition(m_position);
}
