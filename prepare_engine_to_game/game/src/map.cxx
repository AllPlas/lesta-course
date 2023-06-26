#include "map.hxx"

#include "engine.hxx"

Map::Map(const fs::path& waterTexturePath,
         const fs::path& airTexturePath,
         Size textureSize,
         Size mapSize)
    : m_waterSprite{ waterTexturePath, textureSize }
    , m_airSprite{ airTexturePath, textureSize }
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

    //    for (const auto& pos : m_islands.back().getPositions()) {
    //        auto found{ std::find(m_waterPositions.begin(), m_waterPositions.end(), pos) };
    //        if (found != m_waterPositions.end()) m_waterPositions.erase(found);
    //    }
}

const std::vector<Position>& Map::getWaterPositions() const noexcept { return m_waterPositions; }

void Map::update() {
    for (auto& island : m_islands)
        island.update();

    m_waterSprite.updateWindowSize();
    m_waterSprite.checkAspect({ 800, 600 });
    m_airSprite.updateWindowSize();
    m_airSprite.checkAspect({ 800, 600 });
}

void Map::render(const View& view) {
    for (auto& island : m_islands)
        island.render(view);

    auto shipPos{ view.getPosition() };

    auto tileX{ static_cast<std::size_t>(shipPos.x) / m_textureSize.width + 8 };
    auto tileY{ static_cast<std::size_t>(shipPos.y) / m_textureSize.height + 6 };

    auto windowTilesX{ (getEngineInstance()->getWindowSize().width / 50) / 2 };
    auto windowTilesY{ (getEngineInstance()->getWindowSize().height / 50) / 2 };

    std::ptrdiff_t leftCornerX{ static_cast<std::ptrdiff_t>(tileX) - windowTilesX - 1 };
    std::ptrdiff_t leftCornerY{ static_cast<std::ptrdiff_t>(tileY) - windowTilesY - 1 };

    for (std::ptrdiff_t i{ leftCornerY < 0 ? 0 : leftCornerY };
         i <= ((leftCornerY + getEngineInstance()->getWindowSize().height / 50 + 2) > 159
                   ? 159
                   : leftCornerY + getEngineInstance()->getWindowSize().height / 50 + 2);
         ++i)
        for (std::ptrdiff_t j{ leftCornerX < 0 ? 0 : leftCornerX };
             j <= (leftCornerX + (getEngineInstance()->getWindowSize().width / 50) + 2 > 159
                       ? 159
                       : leftCornerX + (getEngineInstance()->getWindowSize().width / 50) + 2);
             ++j) {
            getWaterSprite().setPosition({ getWaterPositions().at(i * 160 + j) });
            getEngineInstance()->render(getWaterSprite(), view);
        }
}

Sprite& Map::getWaterSprite() noexcept { return m_waterSprite; }
Island& Map::getIsland(std::size_t id) noexcept { return m_islands.at(id); }
