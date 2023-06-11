//
// Created by Алексей Крукович on 11.06.23.
//

#include "sprite.hxx"

#include "engine.hxx"

Sprite::Sprite(const fs::path& texturePath, Sprite::Size size) {
    m_texture.load(texturePath);
    m_size = size;

    m_moveMatrix[0][0] = 1.0f;
    m_moveMatrix[1][1] = 1.0f;
    m_moveMatrix[2][2] = 1.0f;

    m_scaleMatrix[0][0] = 1.0f;
    m_scaleMatrix[1][1] = 1.0f;
    m_scaleMatrix[2][2] = 1.0f;

    m_aspectMatrix[0][0] = 1;
    m_aspectMatrix[1][1] = 1;
    m_aspectMatrix[2][2] = 1.0f;

    m_rotationMatrix[0][0] = 1.0f;
    m_rotationMatrix[1][1] = 1.0f;
    m_rotationMatrix[2][2] = 1.0f;

    m_vertices.push_back({ (-size.width / 2) / (getEngineInstance()->getWindowSize().first / 2.0f),
                           (size.height / 2) / (getEngineInstance()->getWindowSize().second / 2.0f),
                           0.0,
                           0.0,
                           0 });

    m_vertices.push_back({ (size.width / 2) / (getEngineInstance()->getWindowSize().first / 2.0f),
                           (size.height / 2) / (getEngineInstance()->getWindowSize().second / 2.0f),
                           1.0,
                           0.0,
                           0 });

    m_vertices.push_back(
        { (size.width / 2) / (getEngineInstance()->getWindowSize().first / 2.0f),
          (-size.height / 2) / (getEngineInstance()->getWindowSize().second / 2.0f),
          1.0,
          1.0,
          0 });

    m_vertices.push_back(
        { (-size.width / 2) / (getEngineInstance()->getWindowSize().first / 2.0f),
          (-size.height / 2) / (getEngineInstance()->getWindowSize().second / 2.0f),
          0.0,
          1.0,
          0 });

    m_indices = { 0, 1, 2, 0, 2, 3 };
}

void Sprite::checkAspect(Sprite::Size size) {
    m_aspectMatrix[0][0] =
        static_cast<float>(size.width) / getEngineInstance()->getWindowSize().first;
    m_aspectMatrix[1][1] =
        static_cast<float>(size.height) / getEngineInstance()->getWindowSize().second;
}

glm::mat3 Sprite::getResultMatrix() const noexcept {
    auto resultMatrix{ m_aspectMatrix * m_moveMatrix * m_rotationMatrix };
    return resultMatrix;
}

Sprite::Position Sprite::getPosition() const noexcept {
    auto resultVec{ m_scaleMatrix * m_moveMatrix * m_aspectMatrix * m_rotationMatrix *
                    glm::vec3(m_position.x, m_position.y, 1.0) };

    return { resultVec.x, resultVec.y };
}

void Sprite::setPosition(Sprite::Position position) {
    m_moveMatrix[2][0] = position.x;
    m_moveMatrix[2][1] = position.y;
}

Sprite::Size Sprite::getSize() const noexcept {
    return { m_size.width * m_scaleMatrix[0][0] * m_aspectMatrix[0][0],
             m_size.height * m_scaleMatrix[0][0] * m_aspectMatrix[0][0] };
}

void Sprite::setScale(glm::vec2 scale) {
    m_scaleMatrix[0][0] = scale.x;
    m_scaleMatrix[1][1] = scale.y;
}

const std::vector<Vertex2>& Sprite::getVertices() const noexcept { return m_vertices; }

const std::vector<uint16_t>& Sprite::getIndices() const noexcept { return m_indices; }

const Texture& Sprite::getTexture() const noexcept { return m_texture; }
