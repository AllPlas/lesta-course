#include "view.hxx"

glm::mat3 View::getViewMatrix() const {
    glm::mat3 view{ 1.0f };
    view[2][0] = -m_position.x * m_scale;
    view[2][1] = -m_position.y * m_scale;
    view[0][0] = m_scale;
    view[1][1] = m_scale;
    return view;
}

void View::setPosition(Position position) { m_position = position; }

void View::setScale(float scale) { m_scale = scale; }
