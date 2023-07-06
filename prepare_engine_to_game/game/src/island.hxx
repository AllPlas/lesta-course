#ifndef ENGINE_PREPARE_TO_GAME_ISLAND_HXX
#define ENGINE_PREPARE_TO_GAME_ISLAND_HXX

#include <filesystem>
#include <sprite.hxx>
#include <unordered_map>
#include <vector>
#include <view.hxx>

#include "player.hxx"
#include "ship.hxx"

namespace fs = std::filesystem;

class Island final
{
private:
    Size m_size{};
    Rectangle m_rectangle{};
    std::vector<std::string> m_pattern{};
    std::vector<std::pair<char, Position>> m_positions{};

    inline static std::unordered_map<std::string, Sprite>* s_islandTiles{};
    inline static const std::unordered_map<char, std::string>* s_charToIslandString{};

public:
    Island(Size size, Rectangle rectangle, const std::vector<std::string>& pattern);

    static void setIslandTiles(std::unordered_map<std::string, Sprite>& islandTiles);
    static void setIslandPattern(std::unordered_map<char, std::string>& pattern);

    void resizeUpdate();
    void render(const View& view);

    void interact(Ship& ship);
    void interact(Player& player);

    [[nodiscard]] const std::vector<std::pair<char, Position>>& getPositions() const noexcept;
    [[nodiscard]] bool isIslandOnView(Position position) const noexcept;

    static auto& getIslandTiles() { return s_islandTiles; }
    static auto& getChatToIsland() { return s_charToIslandString; }

    friend bool intersect(const Island& island, const Ship& ship);
    friend bool intersect(const Island& island, const Player& player);
};

bool intersect(const Island& island, const Ship& ship);
bool intersect(const Island& island, const Player& player);

#endif // ENGINE_PREPARE_TO_GAME_ISLAND_HXX
