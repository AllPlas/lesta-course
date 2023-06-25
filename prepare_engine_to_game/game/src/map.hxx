#ifndef ENGINE_PREPARE_TO_GAME_MAP_HXX
#define ENGINE_PREPARE_TO_GAME_MAP_HXX

#include <filesystem>
#include <sprite.hxx>
#include <vector>

#include "island.hxx"

namespace fs = std::filesystem;

class Map
{
private:
    Sprite m_waterSprite;
    Sprite m_airSprite;
    Size m_textureSize{};
    Size m_mapSize{};

    std::vector<Island> m_islands{};
    std::vector<Position> m_waterPositions{};
    std::vector<Position> m_airPositions{};

public:
    Map(const fs::path& waterTexturePath,
        const fs::path& airTexturePath,
        Size textureSize,
        Size mapSize);

    void addIsland(Position position, const std::vector<std::string>& pattern);
    [[nodiscard]] Island& getIsland(std::size_t id) noexcept;
    void update();

    [[nodiscard]] const std::vector<Position>& getWaterPositions() const noexcept;
    [[nodiscard]] const std::vector<Position>& getIslandPositions(std::size_t id) const noexcept;
    [[nodiscard]] Sprite& getWaterSprite() noexcept;
};

#endif // ENGINE_PREPARE_TO_GAME_MAP_HXX
