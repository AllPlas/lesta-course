#include "view.hxx"

#include "engine.hxx"

glm::mat3 View::getViewMatrix() const {
    glm::mat3 view{ 1.0f };
    view[2][0] = -m_position.x * m_scale;
    view[2][1] = -m_position.y * m_scale;
    view[0][0] = m_scale;
    view[1][1] = m_scale;
    return view;
}

void View::setPosition(Position position) {
    m_position = { position.x / (getEngineInstance()->getWindowSize().width / 2.0f),
                   position.y / (getEngineInstance()->getWindowSize().height / 2.0f) };
}

Position View::getPosition() const noexcept {
    return { m_position.x * (getEngineInstance()->getWindowSize().width / 2.0f),
             m_position.y * (getEngineInstance()->getWindowSize().height / 2.0f) };
}

void View::setScale(float scale) { m_scale = scale; }

float View::getScale() const noexcept { return m_scale; }
