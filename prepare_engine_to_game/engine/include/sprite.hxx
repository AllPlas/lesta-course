#ifndef VERTEX_MORPHING_SPRITE_HXX
#define VERTEX_MORPHING_SPRITE_HXX
#include <glm/glm.hpp>

#include <optional>
#include <vector>

#include "buffer.hxx"
#include "structures.hxx"
#include "texture.hxx"

class Sprite final
{
private:
    Position m_position{};
    Size m_size{};
    Scale m_scale{};
    Angle m_rotationAngle{};

    Texture m_texture{};

    glm::mat3 m_moveMatrix{ 0.0f };
    glm::mat3 m_scaleMatrix{ 0.0f };
    glm::mat3 m_aspectMatrix{ 0.0f };
    glm::mat3 m_rotationMatrix{ 0.0f };

    int m_windowWidth{};
    int m_windowHeight{};

    std::vector<Vertex2> m_vertices{};
    std::vector<uint16_t> m_indices{};

public:
    Sprite(const fs::path& texturePath, Size size);

    [[nodiscard]] Position getPosition() const noexcept;
    void setPosition(Position position);

    [[nodiscard]] Size getSize() const noexcept;

    void setScale(Scale scale);
    [[nodiscard]] Scale getScale() const noexcept;

    void setRotate(float angle);
    [[nodiscard]] Angle getRotate() const noexcept;

    void checkAspect(Size size);
    void updateWindowSize();

    [[nodiscard]] const std::vector<Vertex2>& getVertices() const noexcept;
    [[nodiscard]] const std::vector<uint16_t>& getIndices() const noexcept;
    [[nodiscard]] const Texture& getTexture() const noexcept;
    [[nodiscard]] glm::mat3 getResultMatrix() const noexcept;
    [[nodiscard]] Rectangle getRectangle() const noexcept;
};

std::optional<Rectangle> intersect(const Sprite& s1, const Sprite& s2);

#endif // VERTEX_MORPHING_SPRITE_HXX
