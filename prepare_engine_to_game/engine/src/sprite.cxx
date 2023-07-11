#include "sprite.hxx"

#include "engine.hxx"

Sprite::Sprite(Size size)
    : m_size{ size }
    , m_windowWidth{ getEngineInstance()->getWindowSize().width }
    , m_windowHeight{ getEngineInstance()->getWindowSize().height } {
    initialize();
}

Sprite::Sprite(const fs::path& texturePath)
    : m_hasTexture{ true }
    , m_texture{ new Texture{} }
    , m_windowWidth{ getEngineInstance()->getWindowSize().width }
    , m_windowHeight{ getEngineInstance()->getWindowSize().height } {
    m_texture->load(texturePath);
    m_size.width = m_texture->getWidth();
    m_size.height = m_texture->getHeight();
    initialize();
}

Sprite::Sprite(const fs::path& texturePath, Size size)
    : m_size{ size }
    , m_hasTexture{ true }
    , m_texture{ new Texture{} }
    , m_windowWidth{ getEngineInstance()->getWindowSize().width }
    , m_windowHeight{ getEngineInstance()->getWindowSize().height } {
    m_texture->load(texturePath);
    initialize();
}

void Sprite::checkAspect(Size size) {
    m_aspectMatrix[0][0] = size.width / m_windowWidth;
    m_aspectMatrix[1][1] = size.height / m_windowHeight;
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
    float x = resultVec.x * (getEngineInstance()->getWindowSize().width / 2.0f);
    float y = resultVec.y * (getEngineInstance()->getWindowSize().height / 2.0f);

    return { x, y };
}

void Sprite::setPosition(Position position) {
    m_moveMatrix[2][0] = position.x / (getEngineInstance()->getWindowSize().width / 2.0f);
    m_moveMatrix[2][1] = position.y / (getEngineInstance()->getWindowSize().height / 2.0f);
}

Size Sprite::getSize() const noexcept {
    auto scale{ m_scaleMatrix };
    scale[0][0] *= m_aspectMatrix[0][0];
    scale[1][1] *= m_aspectMatrix[1][1];
    auto transformed{ m_rotationMatrix * scale * glm::vec3(m_size.width, m_size.height, 1.0f) };
    return { std::abs(transformed.x), std::abs(transformed.y) };
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

    updateWindowSize();
    checkAspect(s_originalWindowSize);

    auto aspectWH{ s_originalWindowSize.width / s_originalWindowSize.height };
    m_aspectMatrix[1][1] *=
        1 + std::abs((aspectWH - 1) * pow(std::sin(m_rotationAngle.getInRadians()), 2));
    m_aspectMatrix[0][0] /=
        1 + std::abs((aspectWH - 1) * pow(std::sin(m_rotationAngle.getInRadians()), 2));
}

Angle Sprite::getRotate() const noexcept { return m_rotationAngle; }

const std::vector<Vertex2>& Sprite::getVertices() const noexcept { return m_vertices; }

const std::vector<uint16_t>& Sprite::getIndices() const noexcept { return m_indices; }

const Texture& Sprite::getTexture() const noexcept { return *m_texture; }

void Sprite::updateWindowSize() {
    m_windowWidth = getEngineInstance()->getWindowSize().width;
    m_windowHeight = getEngineInstance()->getWindowSize().height;
}

Rectangle Sprite::getRectangle() const noexcept {
    return Rectangle{ .xy = { getPosition().x - getSize().width / 2.0f,
                              getPosition().y - getSize().height / 2.0f },
                      .wh = { getSize() } };
}

Sprite::~Sprite() {
    if (m_hasTexture) delete m_texture;
}

void Sprite::initialize() {
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

    if (s_originalWindowSize.width == 0 || s_originalWindowSize.height == 0) {
        s_originalWindowSize.width = getEngineInstance()->getWindowSize().width;
        s_originalWindowSize.height = getEngineInstance()->getWindowSize().height;
    }

    m_vertices.push_back({ (-m_size.width / 2) / (s_originalWindowSize.width / 2.0f),
                           (m_size.height / 2) / (s_originalWindowSize.height / 2.0f),
                           0.0,
                           0.0,
                           0 });

    m_vertices.push_back({ (m_size.width / 2) / (s_originalWindowSize.width / 2.0f),
                           (m_size.height / 2) / (s_originalWindowSize.height / 2.0f),
                           1.0,
                           0.0,
                           0 });

    m_vertices.push_back({ (m_size.width / 2) / (s_originalWindowSize.width / 2.0f),
                           (-m_size.height / 2) / (s_originalWindowSize.height / 2.0f),
                           1.0,
                           1.0,
                           0 });

    m_vertices.push_back({ (-m_size.width / 2) / (s_originalWindowSize.width / 2.0f),
                           (-m_size.height / 2) / (s_originalWindowSize.height / 2.0f),
                           0.0,
                           1.0,
                           0 });

    m_indices = { 0, 1, 2, 0, 2, 3 };
}

void Sprite::setTexture(Texture& texture) {
    if (m_hasTexture) throw std::runtime_error{ "Error : setTexture : The sprite has own texture" };
    m_texture = &texture;
}

void Sprite::setOriginalSize(Size size) { s_originalWindowSize = size; }

std::optional<Rectangle> intersect(const Sprite& s1, const Sprite& s2) {
    return intersect(s1.getRectangle(), s2.getRectangle());
}
