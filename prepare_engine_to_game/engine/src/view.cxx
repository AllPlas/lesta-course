#include "view.hxx"

#include <glm/gtc/matrix_transform.hpp>

glm::mat3 View::getViewMatrix() const {
    glm::mat3 view{ 1.0f };
    //  view = glm::translate(view, glm::vec3(-m_x, -m_y, 0.0f));
    view[2][0] = -m_x * m_scale;
    view[2][1] = -m_y * m_scale;
    view[0][0] = m_scale;
    view[1][1] = m_scale;
    // view = glm::scale(view, glm::vec3(m_scale, m_scale, 1.0f));
    return view;
}

void View::setPosition(float x, float y) {
    m_x = x;
    m_y = y;
}

void View::setScale(float scale) { m_scale = scale; }
