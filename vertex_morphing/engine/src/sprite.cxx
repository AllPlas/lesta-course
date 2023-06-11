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
