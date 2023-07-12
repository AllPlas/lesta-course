#ifndef ENGINE_PREPARE_TO_GAME_MAP_HXX
#define ENGINE_PREPARE_TO_GAME_MAP_HXX

#include <array>
#include <filesystem>
#include <memory>
#include <sprite.hxx>
#include <vector>
#include <view.hxx>

#include "bottle.hxx"
#include "island.hxx"
#include "player.hxx"
#include "ship.hxx"
#include "treasure.hxx"

namespace fs = std::filesystem;

class Map
{
private:
    Sprite m_waterSprite;
    Sprite m_airSprite;

    Bottle m_bottle;
    Treasure m_treasure;

    Size m_textureSize{};
    Size m_mapSize{};

    Rectangle m_shipRectangle{};

    std::vector<Island> m_islands{};
    std::vector<Position> m_waterPositions{};
    std::vector<Position> m_airPositions{};
    std::vector<Position> m_bottlePositions{};

    std::unique_ptr<VertexBuffer<Vertex2>> m_gridPtr{};
    std::unique_ptr<IndexBuffer<std::uint32_t>> m_idxGridPtr{};

    std::array<std::unique_ptr<VertexBuffer<Vertex2>>, 5> m_islandVertexBuffers{};
    std::array<std::unique_ptr<IndexBuffer<std::uint32_t>>, 5> m_islandIndexBuffers{};

    std::unique_ptr<VertexBuffer<Vertex2>> m_bottleVertexBuffer{};
    std::unique_ptr<IndexBuffer<std::uint32_t>> m_bottleIndexBuffer{};

    inline static constexpr int s_maxCountOfBottles{ 50 };
    int m_countOfBottles{};
    bool m_isTreasureUnearthed{};

    Island* m_interactIsland{};

public:
    Map(const fs::path& waterTexturePath,
        const fs::path& airTexturePath,
        const fs::path& bottleTexturePath,
        const fs::path& treasureTexturePath,
        const fs::path& xMarkTexturePath,
        Size textureSize,
        Size mapSize);

    void addIsland(Position position, const std::vector<std::string>& pattern);
    [[nodiscard]] Island& getIsland(std::size_t id) noexcept;
    void resizeUpdate();
    void render(const View& view);

    void interact(Ship& ship);
    void interact(Player& player);

    void generateBottles();
    void generateTreasure();

    [[nodiscard]] const std::vector<Position>& getWaterPositions() const noexcept;
    [[nodiscard]] Sprite& getWaterSprite() noexcept;
    [[nodiscard]] bool isTreasureUnearthed() const noexcept;
    Treasure& getTreasure() noexcept;

private:
    void updateBottlePositions();
};

#endif // ENGINE_PREPARE_TO_GAME_MAP_HXX
