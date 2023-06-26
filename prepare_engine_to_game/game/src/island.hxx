#ifndef ENGINE_PREPARE_TO_GAME_ISLAND_HXX
#define ENGINE_PREPARE_TO_GAME_ISLAND_HXX

#include <filesystem>
#include <sprite.hxx>
#include <unordered_map>
#include <vector>
#include <view.hxx>

namespace fs = std::filesystem;

class Island final
{
private:
    Size m_size{};
    Rectangle m_rectangle{};
    std::vector<std::string> m_pattern{};
    std::vector<std::pair<char, Position>> m_positions{};

    inline static std::unordered_map<std::string, Sprite>* s_islandTiles{};
    inline static std::unordered_map<char, std::string>* s_charToIslandString{};

public:
    Island(Size size, Rectangle rectangle, const std::vector<std::string>& pattern);

    static void setIslandTiles(std::unordered_map<std::string, Sprite>& islandTiles);
    static void setIslandPattern(std::unordered_map<char, std::string>& pattern);

    void resizeUpdate();
    void render(const View& view);

    [[nodiscard]] const std::vector<std::pair<char, Position>>& getPositions() const noexcept;
};

#endif // ENGINE_PREPARE_TO_GAME_ISLAND_HXX
