#ifndef ENGINE_PREPARE_TO_GAME_VIEW_HXX
#define ENGINE_PREPARE_TO_GAME_VIEW_HXX

#include <glm/glm.hpp>

class View
{
private:
    float m_x{};
    float m_y{};

    float m_scale{ 1.0f };

public:
    [[nodiscard]] glm::mat3 getViewMatrix() const;
    void setPosition(float x, float y);
    void setScale(float scale);
};

#endif // ENGINE_PREPARE_TO_GAME_VIEW_HXX
