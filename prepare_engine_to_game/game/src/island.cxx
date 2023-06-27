#include "island.hxx"

#include <algorithm>
#include <engine.hxx>
#include <string>

Island::Island(Size size, Rectangle rectangle, const std::vector<std::string>& pattern)
    : m_size{ size }, m_rectangle{ rectangle }, m_pattern{ pattern } {
    int tilesW{ static_cast<int>(m_rectangle.wh.width / size.width) };
    int tilesH{ static_cast<int>(m_rectangle.wh.height / size.height) };

    std::reverse(m_pattern.begin(), m_pattern.end());

    if (tilesH != m_pattern.size())
        throw std::runtime_error{ "Error : Island() : tilesH != pattern.size()"s };

    for (const auto& rows : m_pattern)
        if (tilesW != rows.size())
            throw std::runtime_error{ "Error : Island() : tilesW != rows.size()"s };

    for (std::size_t h{}; h < m_pattern.size(); ++h)
        for (std::size_t w{}; w < m_pattern.size(); ++w) {
            if (m_pattern[h][w] != '#')
                m_positions.push_back({ m_pattern[h][w],
                                        { m_rectangle.xy.x + m_size.width * w,
                                          m_rectangle.xy.y + m_size.height * h } });
        }
}

void Island::resizeUpdate() {
    for (auto& [_, sprite] : *s_islandTiles) {
        sprite.updateWindowSize();
        sprite.checkAspect({ 800, 600 });
    }
}

void Island::render(const View& view) {
    if (isIslandOnView(view.getPosition())) {
        for (const auto& pos : m_positions) {
            auto& sprite{ s_islandTiles->at(s_charToIslandString->at(pos.first)) };
            sprite.setPosition(pos.second);
            getEngineInstance()->render(sprite, view);
        }
    }
}

void Island::setIslandTiles(std::unordered_map<std::string, Sprite>& islandTiles) {
    s_islandTiles = &islandTiles;
}

void Island::setIslandPattern(std::unordered_map<char, std::string>& pattern) {
    s_charToIslandString = &pattern;
}

const std::vector<std::pair<char, Position>>& Island::getPositions() const noexcept {
    return m_positions;
}

void Island::interact(Ship& ship) {
    if (isIslandOnView(ship.getSprite().getPosition())) {
        if (intersect(*this, ship)) ship.forceStop();
    }
}

void Island::interact(Player& player) {
    if (isIslandOnView(player.getPosition())) {
        if (!intersect(*this, player)) player.forceStop();
    }
}

bool Island::isIslandOnView(Position position) const noexcept {
    auto viewPos{ position };
    auto leftX{ viewPos.x - (getEngineInstance()->getWindowSize().width / 2.0f) };
    auto leftY{ viewPos.y - (getEngineInstance()->getWindowSize().height / 2.0f) };

    Rectangle viewRect{ .xy{ leftX, leftY },
                        .wh = { static_cast<float>(getEngineInstance()->getWindowSize().width),
                                static_cast<float>(getEngineInstance()->getWindowSize().height) } };

    return intersect(viewRect, m_rectangle).has_value();
}

bool intersect(const Island& island, const Ship& ship) {
    for (const auto& pos : island.getPositions()) {
        Rectangle rectangle{ .xy = { pos.second.x - island.m_size.width / 2.0f,
                                     pos.second.y - island.m_size.height / 2.0f },
                             .wh = island.m_size };
        if (auto result{ rectangle.contains(ship.getPosition()) }) return true;
    }

    return false;
}

bool intersect(const Island& island, const Player& player) {
    for (const auto& pos : island.getPositions()) {
        Rectangle rectangle{ .xy = { pos.second.x - island.m_size.width / 2.0f,
                                     pos.second.y - island.m_size.height / 2.0f },
                             .wh = island.m_size };
        if (auto result{ rectangle.contains(player.getPosition()) }) return true;
    }

    return false;
}
