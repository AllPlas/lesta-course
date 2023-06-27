#include "treasure.hxx"

Treasure::Treasure(const fs::path& treasureTexPath, const fs::path& xMarkTexPath, Size size)
    : m_treasureSprite{ treasureTexPath, size }
    , m_xMarkSprite{ xMarkTexPath, size }
    , m_size{ size } {}

void Treasure::setPosition(Position position) {
    m_position = position;
    m_treasureSprite.setPosition(m_position);
    m_xMarkSprite.setPosition(m_position);
}

Position Treasure::getPosition() const noexcept { return m_position; }

Sprite& Treasure::getTreasureSprite() noexcept { return m_treasureSprite; }

Sprite& Treasure::getXMarkSprite() noexcept { return m_xMarkSprite; }

void Treasure::resizeUpdate() {
    m_treasureSprite.updateWindowSize();
    m_treasureSprite.checkAspect({ 800, 600 });
    m_treasureSprite.setPosition(m_position);

    m_xMarkSprite.updateWindowSize();
    m_xMarkSprite.checkAspect({ 800, 600 });
    m_xMarkSprite.setPosition(m_position);
}
