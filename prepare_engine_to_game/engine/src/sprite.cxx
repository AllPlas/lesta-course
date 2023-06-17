#include "sprite.hxx"

#include "engine.hxx"

Sprite::Sprite(const fs::path& texturePath, Sprite::Size size)
    : m_size{ size }
    , m_windowWidth{ getEngineInstance()->getWindowSize().width }
    , m_windowHeight{ getEngineInstance()->getWindowSize().height } {
    m_texture.load(texturePath);

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

    m_vertices.push_back({ (-size.width / 2) / (m_windowWidth / 2.0f),
                           (size.height / 2) / (m_windowHeight / 2.0f),
                           0.0,
                           0.0,
                           0 });

    m_vertices.push_back({ (size.width / 2) / (m_windowWidth / 2.0f),
                           (size.height / 2) / (m_windowHeight / 2.0f),
                           1.0,
                           0.0,
                           0 });

    m_vertices.push_back({ (size.width / 2) / (m_windowWidth / 2.0f),
                           (-size.height / 2) / (m_windowHeight / 2.0f),
                           1.0,
                           1.0,
                           0 });

    m_vertices.push_back({ (-size.width / 2) / (m_windowWidth / 2.0f),
                           (-size.height / 2) / (m_windowHeight / 2.0f),
                           0.0,
                           1.0,
                           0 });

    m_indices = { 0, 1, 2, 0, 2, 3 };
}

void Sprite::checkAspect(Sprite::Size size) {
    m_aspectMatrix[0][0] = static_cast<float>(size.width) / m_windowWidth;
    m_aspectMatrix[1][1] = static_cast<float>(size.height) / m_windowHeight;
}

glm::mat3 Sprite::getResultMatrix() const noexcept {
    auto resultMatrix{ m_moveMatrix * m_scaleMatrix * m_aspectMatrix * m_rotationMatrix };
    return resultMatrix;
}

Sprite::Position Sprite::getPosition() const noexcept {
    auto resultVec{ m_moveMatrix * glm::vec3(m_position.x, m_position.y, 1.0) };
    float x = resultVec.x * (m_windowWidth / 2.0f);
    float y = resultVec.y * (m_windowHeight / 2.0f);
    return { x, y };
}

void Sprite::setPosition(Sprite::Position position) {
    m_moveMatrix[2][0] = position.x / (m_windowWidth / 2.0f);
    m_moveMatrix[2][1] = position.y / (m_windowHeight / 2.0f);
}

Sprite::Size Sprite::getSize() const noexcept {
    return { m_size.width * m_scaleMatrix[0][0] * m_aspectMatrix[0][0],
             m_size.height * m_scaleMatrix[0][0] * m_aspectMatrix[0][0] };
}

void Sprite::setScale(float scale) {
    m_scale = scale;
    m_scaleMatrix[0][0] = scale;
    m_scaleMatrix[1][1] = scale;
}

float Sprite::getScale() const noexcept { return m_scale; }

void Sprite::setRotate(float angle) {
    m_rotationAngle = angle * std::numbers::pi / 180;
    m_rotationMatrix[0][0] = std::cos(m_rotationAngle);
    m_rotationMatrix[0][1] = std::sin(m_rotationAngle);
    m_rotationMatrix[1][0] = -std::sin(m_rotationAngle);
    m_rotationMatrix[1][1] = std::cos(m_rotationAngle);
}

float Sprite::getRotate() const noexcept { return m_rotationAngle * 180 / std::numbers::pi; }

const std::vector<Vertex2>& Sprite::getVertices() const noexcept { return m_vertices; }

const std::vector<uint16_t>& Sprite::getIndices() const noexcept { return m_indices; }

const Texture& Sprite::getTexture() const noexcept { return m_texture; }

void Sprite::updateWindowSize() {
    m_windowWidth = getEngineInstance()->getWindowSize().width;
    m_windowHeight = getEngineInstance()->getWindowSize().height;
}
