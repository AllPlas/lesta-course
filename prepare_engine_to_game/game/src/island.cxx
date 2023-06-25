#include "island.hxx"

#include <algorithm>

Island::Island(const fs::path& textureFilepath,
               Size size,
               Rectangle rectangle,
               const std::vector<std::string>& pattern)
    : m_sprite{ textureFilepath, size }, m_rectangle{ rectangle }, m_pattern{ pattern } {
    int tilesW{ static_cast<int>(m_rectangle.wh.width / size.width) };
    int tilesH{ static_cast<int>(m_rectangle.wh.height / size.height) };

    std::reverse(m_pattern.begin(), m_pattern.end());

    if (tilesH != m_pattern.size())
        throw std::runtime_error{ "Error : Island() : tilesH != pattern.size()"s };
    if (tilesW != m_pattern[0].size())
        throw std::runtime_error{ "Error : Island() : tilesW != pattern[0].size()"s };

    for (std::size_t h{}; h < m_pattern.size(); ++h)
        for (std::size_t w{}; w < m_pattern.size(); ++w) {
            if (m_pattern[h][w] != ' ')
                m_positions.push_back({ m_rectangle.xy.x + m_sprite.getSize().width * w,
                                        m_rectangle.xy.y + m_sprite.getSize().height * h });
        }
}

const std::vector<Position>& Island::getPositions() const noexcept { return m_positions; }
Sprite& Island::getSprite() noexcept { return m_sprite; }

void Island::update() {
    m_sprite.updateWindowSize();
    m_sprite.checkAspect({ 800, 600 });
}
