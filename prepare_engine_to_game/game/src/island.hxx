#ifndef ENGINE_PREPARE_TO_GAME_ISLAND_HXX
#define ENGINE_PREPARE_TO_GAME_ISLAND_HXX

#include <filesystem>
#include <functional>
#include <sprite.hxx>
#include <vector>
#include <view.hxx>

namespace fs = std::filesystem;

class Island final
{
private:
    Sprite m_sprite;
    Rectangle m_rectangle{};
    std::vector<std::string> m_pattern{};
    std::vector<Position> m_positions{};

public:
    Island(const fs::path& textureFilepath,
           Size size,
           Rectangle rectangle,
           const std::vector<std::string>& pattern);

    [[nodiscard]] const std::vector<Position>& getPositions() const noexcept;
    Sprite& getSprite() noexcept;
};

#endif // ENGINE_PREPARE_TO_GAME_ISLAND_HXX
