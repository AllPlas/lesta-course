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

    [[nodiscard]] Position getPosition() const noexcept {
        auto resultVec{ m_scaleMatrix * m_moveMatrix * m_aspectMatrix * m_rotationMatrix *
                        glm::vec3(m_position.x, m_position.y, 1.0) };

        return { resultVec.x, resultVec.y };
    }

    void setPosition(Position position) {
        m_moveMatrix[2][0] = position.x;
        m_moveMatrix[2][1] = position.y;
    }

    [[nodiscard]] Size getSize() const noexcept {
        return { m_size.width * m_scaleMatrix[0][0] * m_aspectMatrix[0][0],
                 m_size.height * m_scaleMatrix[0][0] * m_aspectMatrix[0][0] };
    }

    void setScale(glm::vec2 scale) {
        m_scaleMatrix[0][0] = scale.x;
        m_scaleMatrix[1][1] = scale.y;
    }

    void checkAspect(Size size);

    [[nodiscard]] const std::vector<Vertex2>& getVertices() const noexcept { return m_vertices; }
    [[nodiscard]] const std::vector<uint16_t>& getIndices() const noexcept { return m_indices; }
    [[nodiscard]] const Texture& getTexture() const noexcept { return m_texture; }
    [[nodiscard]] glm::mat3 getResultMatrix() const noexcept {
        auto resultMatrix{ m_aspectMatrix * m_moveMatrix * m_rotationMatrix };

        return resultMatrix;
    }
};

#endif // VERTEX_MORPHING_SPRITE_HXX
