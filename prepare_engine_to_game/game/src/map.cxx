#include "map.hxx"

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
            float xPos = xOffset + (h * m_textureSize.width);
            float yPos = yOffset + (w * m_textureSize.height);
            m_waterPositions.push_back({ xPos, yPos });
        }
    }
}

void Map::addIsland(Position position, const std::vector<std::string>& pattern) {
    Rectangle rectangle{ .xy = position,
                         .wh = { m_textureSize.width * static_cast<float>(pattern.at(0).size()),
                                 m_textureSize.height * static_cast<float>(pattern.size()) } };
    m_islands.emplace_back("data/assets/sand.png", m_textureSize, rectangle, pattern);

    for (const auto& pos : m_islands.back().getPositions()) {
        auto found{ std::find(m_waterPositions.begin(), m_waterPositions.end(), pos) };
        if (found != m_waterPositions.end()) m_waterPositions.erase(found);
    }
}

const std::vector<Position>& Map::getWaterPositions() const noexcept { return m_waterPositions; }

const std::vector<Position>& Map::getIslandPositions(std::size_t id) const noexcept {
    return m_islands.at(id).getPositions();
}

void Map::update() {
    for (auto& island : m_islands)
        island.update();

    m_waterSprite.updateWindowSize();
    m_waterSprite.checkAspect({ 800, 600 });
    m_airSprite.updateWindowSize();
    m_airSprite.checkAspect({ 800, 600 });
}

Sprite& Map::getWaterSprite() noexcept { return m_waterSprite; }
Island& Map::getIsland(std::size_t id) noexcept { return m_islands.at(id); }
