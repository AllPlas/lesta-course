#ifndef ENGINE_PREPARE_TO_GAME_VIEW_HXX
#define ENGINE_PREPARE_TO_GAME_VIEW_HXX

#include <glm/glm.hpp>

#include "structures.hxx"

class View final
{
private:
    Position m_position{};
    float m_scale{ 1.0f };

public:
    [[nodiscard]] glm::mat3 getViewMatrix() const;

    void setPosition(Position position);
    [[nodiscard]] Position getPosition() const noexcept;

    void setScale(float scale);
    [[nodiscard]] float getScale() const noexcept;
};

#endif // ENGINE_PREPARE_TO_GAME_VIEW_HXX
