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

void Island::update() {
    for (auto& [_, sprite] : *s_islandTiles) {
        sprite.updateWindowSize();
        sprite.checkAspect({ 800, 600 });
    }
}

void Island::render(const View& view) {
    for (const auto& pos : m_positions) {
        std::reference_wrapper<Sprite> sprite{ s_islandTiles->at(
            s_charToIslandString->at(pos.first)) };
        sprite.get().setPosition(pos.second);
        getEngineInstance()->render(sprite, view);
    }
}

void Island::setIslandTiles(std::unordered_map<std::string, Sprite>& islandTiles) {
    s_islandTiles = &islandTiles;
}

void Island::setIslandPattern(std::unordered_map<char, std::string>& pattern) {
    s_charToIslandString = &pattern;
}
