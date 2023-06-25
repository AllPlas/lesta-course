#include "sprite.hxx"

#include "engine.hxx"

Sprite::Sprite(const fs::path& texturePath, Size size)
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

    m_aspectMatrix[0][0] = 1.0f;
    m_aspectMatrix[1][1] = 1.0f;
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

void Sprite::checkAspect(Size size) {
    float originalRatio{ size.width / size.height };
    float currentRatio{ static_cast<float>(m_windowWidth) / m_windowHeight };
    m_aspectMatrix[1][1] = currentRatio / originalRatio;
    m_aspectMatrix[0][0] = 1.0f;

    // m_aspectMatrix[1][1] = (static_cast<float>(m_windowWidth) / m_windowHeight);
}

glm::mat3 Sprite::getResultMatrix() const noexcept {
    auto mat{ m_scaleMatrix };
    mat[1][1] *= m_aspectMatrix[1][1];
    mat[0][0] *= m_aspectMatrix[0][0];
    auto resultMatrix{ m_moveMatrix * mat * m_rotationMatrix };
    return resultMatrix;
}

Position Sprite::getPosition() const noexcept {
    auto resultVec{ m_moveMatrix * glm::vec3(m_position.x, m_position.y, 1.0) };
    float x = resultVec.x * (m_windowWidth / 2.0f);
    float y = resultVec.y * (m_windowHeight / 2.0f);

    return { x, y };
}

void Sprite::setPosition(Position position) {
    m_moveMatrix[2][0] = position.x / (m_windowWidth / 2.0f);
    m_moveMatrix[2][1] = position.y / (m_windowHeight / 2.0f);
}

Size Sprite::getSize() const noexcept {
    return { m_size.width * m_scaleMatrix[0][0] * m_aspectMatrix[0][0],
             m_size.height * m_scaleMatrix[1][1] * m_aspectMatrix[1][1] };
}

void Sprite::setScale(Scale scale) {
    m_scale = scale;
    m_scaleMatrix[0][0] = scale.x;
    m_scaleMatrix[1][1] = scale.y;
}

Scale Sprite::getScale() const noexcept { return m_scale; }

void Sprite::setRotate(float angle) {
    m_rotationAngle = angle;
    m_rotationMatrix[0][0] = std::cos(m_rotationAngle.getInRadians());
    m_rotationMatrix[0][1] = std::sin(m_rotationAngle.getInRadians());
    m_rotationMatrix[1][0] = -std::sin(m_rotationAngle.getInRadians());
    m_rotationMatrix[1][1] = std::cos(m_rotationAngle.getInRadians());

}

Angle Sprite::getRotate() const noexcept { return m_rotationAngle; }

const std::vector<Vertex2>& Sprite::getVertices() const noexcept { return m_vertices; }

const std::vector<uint16_t>& Sprite::getIndices() const noexcept { return m_indices; }

const Texture& Sprite::getTexture() const noexcept { return m_texture; }

void Sprite::updateWindowSize() {
    m_windowWidth = getEngineInstance()->getWindowSize().width;
    m_windowHeight = getEngineInstance()->getWindowSize().height;
}

Rectangle Sprite::getRectangle() const noexcept {
    return Rectangle{ .xy = { getPosition().x - getSize().width / 2.0f,
                              getPosition().y - getSize().height / 2.0f },
                      .wh = { getSize() } };
}

std::optional<Rectangle> intersect(const Sprite& s1, const Sprite& s2) {
    return intersect(s1.getRectangle(), s2.getRectangle());
}
