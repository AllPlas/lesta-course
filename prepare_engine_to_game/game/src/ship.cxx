#include "ship.hxx"

#include <algorithm>
#include <engine.hxx>

Ship::Ship(const fs::path& textureFilepath, Size size) : m_sprite{ textureFilepath, size } {}

void Ship::move() { m_isMove = true; }

void Ship::rotateLeft() { m_isRotateLeft = true; }

void Ship::rotateRight() { m_isRotateRight = true; }

void Ship::stopMove() { m_isMove = false; }

void Ship::stopRotateLeft() { m_isRotateLeft = false; }

void Ship::stopRotateRight() { m_isRotateRight = false; }

void Ship::update(std::chrono::microseconds timeElapsed) {

    m_sprite.updateWindowSize();
    m_sprite.checkAspect({ 800, 600 });

    //    m_sprite.setScale({ std::sqrt(600.f / getEngineInstance()->getWindowSize().height * 800.f
    //    /
    //                                 getEngineInstance()->getWindowSize().width),
    //                      std::sqrt(600.f / getEngineInstance()->getWindowSize().height * 800.f /
    //                               getEngineInstance()->getWindowSize().width) });
    float timeElapsedInSec{ static_cast<float>(timeElapsed.count()) / 1000000.0f };

    if (m_isMove) {
        m_currentMoveSpeed += m_config.moveAcceleration * timeElapsedInSec;
        m_currentMoveSpeed = std::min(m_currentMoveSpeed, m_config.moveMaxSpeed);
    }
    else {
        m_currentMoveSpeed -= m_config.moveDeceleration * timeElapsedInSec;
        m_currentMoveSpeed = std::max(m_currentMoveSpeed, 0.0f);
    }

    if (m_isRotateLeft) {
        m_currentRotateSpeed += m_config.rotateAcceleration * timeElapsedInSec;
        m_currentRotateSpeed = std::min(m_currentRotateSpeed, m_config.rotateMaxSpeed);
    }

    if (m_isRotateRight) {
        m_currentRotateSpeed -= m_config.rotateAcceleration * timeElapsedInSec;
        m_currentRotateSpeed = std::max(m_currentRotateSpeed, -m_config.rotateMaxSpeed);
    }

    if (!m_isRotateLeft && !m_isRotateRight && m_currentRotateSpeed != 0) {
        if (m_currentRotateSpeed > 0) {
            m_currentRotateSpeed -= m_config.rotateDeceleration * timeElapsedInSec;
            m_currentRotateSpeed = std::max(m_currentRotateSpeed, 0.0f);
        }
        else {
            m_currentRotateSpeed += m_config.rotateDeceleration * timeElapsedInSec;
            m_currentRotateSpeed = std::min(m_currentRotateSpeed, 0.0f);
        }
    }

    float deltaX{ 0.0f };
    float deltaY{ m_currentMoveSpeed * timeElapsedInSec };
    float deltaAngle{ m_currentRotateSpeed * timeElapsedInSec };

    float newAngle{ m_sprite.getRotate().getInDegrees() + deltaAngle };
    if (newAngle > 360)
        newAngle -= 360;
    else if (newAngle < 0)
        newAngle += 360;

    m_sprite.setRotate(newAngle);

    float newX = m_sprite.getPosition().x + deltaX * std::cos(m_sprite.getRotate().getInRadians()) -
                 deltaY * std::sin(m_sprite.getRotate().getInRadians());

    float newY = m_sprite.getPosition().y + deltaX * std::sin(m_sprite.getRotate().getInRadians()) +
                 deltaY * std::cos(m_sprite.getRotate().getInRadians());

    m_sprite.setPosition({ newX, newY });
}

Ship::Config& Ship::config() noexcept { return m_config; }

const Sprite& Ship::getSprite() const noexcept { return m_sprite; }

float Ship::getMoveSpeed() const noexcept { return m_currentMoveSpeed; }

float Ship::getRotateSpeed() const noexcept { return m_currentRotateSpeed; }
