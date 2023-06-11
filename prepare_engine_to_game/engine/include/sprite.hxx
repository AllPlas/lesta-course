//
// Created by Алексей Крукович on 11.06.23.
//

#ifndef VERTEX_MORPHING_SPRITE_HXX
#define VERTEX_MORPHING_SPRITE_HXX
#include <glm/glm.hpp>

#include <vector>

#include "buffer.hxx"
#include "texture.hxx"

class Sprite final
{
public:
    struct Position
    {
        float x{};
        float y{};
    };

    struct Size
    {
        float width{};
        float height{};
    };

private:
    Position m_position{};
    Size m_size{};

    Texture m_texture{};

    glm::mat3 m_moveMatrix{ 0.0f };
    glm::mat3 m_scaleMatrix{ 0.0f };
    glm::mat3 m_aspectMatrix{ 0.0f };
    glm::mat3 m_rotationMatrix{ 0.0f };

    std::vector<Vertex2> m_vertices{};
    std::vector<uint16_t> m_indices{};

public:
    Sprite(const fs::path& texturePath, Size size);

    [[nodiscard]] Position getPosition() const noexcept;

    void setPosition(Position position);

    [[nodiscard]] Size getSize() const noexcept;

    void setScale(glm::vec2 scale);

    void checkAspect(Size size);

    [[nodiscard]] const std::vector<Vertex2>& getVertices() const noexcept;
    [[nodiscard]] const std::vector<uint16_t>& getIndices() const noexcept;
    [[nodiscard]] const Texture& getTexture() const noexcept;
    [[nodiscard]] glm::mat3 getResultMatrix() const noexcept;
};

#endif // VERTEX_MORPHING_SPRITE_HXX
