#include "map.hxx"

#include "engine.hxx"

Map::Map(const fs::path& waterTexturePath,
         const fs::path& airTexturePath,
         const fs::path& bottleTexturePath,
         const fs::path& treasureTexturePath,
         const fs::path& xMarkTexturePath,
         Size textureSize,
         Size mapSize)
    : m_waterSprite{ waterTexturePath, textureSize }
    , m_airSprite{ airTexturePath, textureSize }
    , m_bottle{ bottleTexturePath, textureSize }
    , m_treasure{ treasureTexturePath, xMarkTexturePath, textureSize }
    , m_textureSize{ textureSize }
    , m_mapSize{ mapSize } {
    float xOffset = -((800 / 2.0f) - (m_textureSize.width / 2.0f));
    float yOffset = -((600 / 2.0f) - (m_textureSize.height / 2.0f));
    for (std::ptrdiff_t h{}; h < m_mapSize.height / m_textureSize.height; ++h) {
        for (std::ptrdiff_t w{}; w < m_mapSize.width / m_textureSize.width; ++w) {
            float xPos = xOffset + (w * m_textureSize.width);
            float yPos = yOffset + (h * m_textureSize.height);
            m_waterPositions.push_back({ xPos, yPos });
        }
    }
}

void Map::addIsland(Position position, const std::vector<std::string>& pattern) {
    Rectangle rectangle{ .xy = position,
                         .wh = { m_textureSize.width * static_cast<float>(pattern.at(0).size()),
                                 m_textureSize.height * static_cast<float>(pattern.size()) } };
    m_islands.emplace_back(m_textureSize, rectangle, pattern);

    for (const auto& pos : m_islands.back().getPositions()) {
        auto found{ std::find(m_waterPositions.begin(), m_waterPositions.end(), pos.second) };
        if (found != m_waterPositions.end()) m_waterPositions.erase(found);
    }
}

const std::vector<Position>& Map::getWaterPositions() const noexcept { return m_waterPositions; }

void Map::resizeUpdate() {
    for (auto& island : m_islands)
        island.resizeUpdate();

    m_bottle.resizeUpdate();
    m_treasure.resizeUpdate();

    m_waterSprite.updateWindowSize();
    m_waterSprite.checkAspect({ 800, 600 });
    m_airSprite.updateWindowSize();
    m_airSprite.checkAspect({ 800, 600 });
}

void Map::render(const View& view) {
    for (auto& island : m_islands)
        island.render(view);

    if (m_hasBottle) getEngineInstance()->render(m_bottle.getSprite(), view);

    auto viewPos{ view.getPosition() };

    auto tileX{ static_cast<std::size_t>(viewPos.x) / m_textureSize.width + 8 };
    auto tileY{ static_cast<std::size_t>(viewPos.y) / m_textureSize.height + 6 };

    auto windowTilesX{ (getEngineInstance()->getWindowSize().width / 50) / 2 / view.getScale() };
    auto windowTilesY{ (getEngineInstance()->getWindowSize().height / 50) / 2 / view.getScale() };

    std::ptrdiff_t leftCornerX{ static_cast<std::ptrdiff_t>(tileX - windowTilesX - 1) };
    std::ptrdiff_t leftCornerY{ static_cast<std::ptrdiff_t>(tileY - windowTilesY - 1) };

    for (std::ptrdiff_t i{ leftCornerY < 0 ? 0 : leftCornerY };
         i <= ((leftCornerY + getEngineInstance()->getWindowSize().height / 50 / view.getScale() +
                2) > 159
                   ? 159
                   : leftCornerY +
                         getEngineInstance()->getWindowSize().height / 50 / view.getScale() + 2);
         ++i)
        for (std::ptrdiff_t j{ leftCornerX < 0 ? 0 : leftCornerX };
             j <=
             (leftCornerX + (getEngineInstance()->getWindowSize().width / 50) / view.getScale() +
                          2 >
                      159
                  ? 159
                  : leftCornerX +
                        (getEngineInstance()->getWindowSize().width / 50) / view.getScale() + 2);
             ++j) {
            getWaterSprite().setPosition({ getWaterPositions().at(i * 160 + j) });
            getEngineInstance()->render(getWaterSprite(), view);
        }
}

Sprite& Map::getWaterSprite() noexcept { return m_waterSprite; }
Island& Map::getIsland(std::size_t id) noexcept { return m_islands.at(id); }

void Map::interact(Ship& ship) {
    if (!ship.isInteract())
        for (auto& island : m_islands) {
            island.interact(ship);
            if (ship.isInteract()) {
                m_interactIsland = &island;
                break;
            }
        }

    if (m_hasBottle) {
        auto is{ intersect(m_bottle.getSprite(), ship.getSprite()).has_value() };
        if (is) {
            generateTreasure();
            m_hasBottle = false;
        }
    }

    Rectangle rect{ .xy{ -400.0f, -300.f }, .wh{ 8000.f, 8000.f } };
    if (auto intersectRect{ intersect(rect, ship.getSprite().getRectangle()) }) {
        if (!rect.contains(ship.getPosition())) {
            ship.forceStop();
            ship.setInteract(false);
        }
    }
    m_shipRectangle = ship.getSprite().getRectangle();
}

void Map::interact(Player& player) {
    m_interactIsland->interact(player);

    if (player.isDigging()) {
        auto is{ intersect(m_treasure.getTreasureSprite(), player.getSprite()) };
        if (is && !m_hasBottle) {
            if (!m_isTreasureUnearthed)
                m_isTreasureUnearthed = true;
            else {
                m_isTreasureUnearthed = false;
                generateBottle();
                player.addMoney(1);
            }
        }
        player.stopDig();
    }

    player.setNearShip(m_shipRectangle.contains(player.getPosition()));
}

bool Map::hasBottle() const noexcept { return m_hasBottle; }

void Map::generateBottle() {
    if (!m_hasBottle) {
        m_hasBottle = true;
        m_bottle.setPosition({ 125, 125 });
    }
}

void Map::generateTreasure() {
    auto pos{ m_islands.at(0).getPositions().at(0).second };
    m_treasure.setPosition(pos);
}

Treasure& Map::getTreasure() noexcept { return m_treasure; }

bool Map::isTreasureUnearthed() const noexcept { return m_isTreasureUnearthed; }
