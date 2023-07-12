#include "ship.hxx"

#include <algorithm>

Ship::Ship(const fs::path& textureFilepath, Size size, Player& player)
    : m_sprite{ textureFilepath, size }, m_player{ player } {}

void Ship::move() { m_isMove = true; }

void Ship::rotateLeft() { m_isRotateLeft = true; }

void Ship::rotateRight() { m_isRotateRight = true; }

void Ship::stopMove() { m_isMove = false; }

void Ship::stopRotateLeft() { m_isRotateLeft = false; }

void Ship::stopRotateRight() { m_isRotateRight = false; }

void Ship::update(std::chrono::microseconds timeElapsed) {
    float timeElapsedInSec{ static_cast<float>(timeElapsed.count()) / 1000000.0f };

    if (m_isMove) {
        m_currentMoveSpeed += m_config.moveAcceleration * timeElapsedInSec;
        m_currentMoveSpeed = std::min(m_currentMoveSpeed, m_config.moveMaxSpeed);
        m_isInteract = false;
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

    float newX = m_position.x + deltaX * std::cos(m_sprite.getRotate().getInRadians()) -
                 deltaY * std::sin(m_sprite.getRotate().getInRadians());

    float newY = m_position.y + deltaX * std::sin(m_sprite.getRotate().getInRadians()) +
                 deltaY * std::cos(m_sprite.getRotate().getInRadians());

    m_lastPosition = m_position;

    m_position.x = newX;
    m_position.y = newY;

    m_sprite.setPosition(m_position);
}

Ship::Config& Ship::config() noexcept { return m_config; }

const Sprite& Ship::getSprite() const noexcept { return m_sprite; }

float Ship::getMoveSpeed() const noexcept { return m_currentMoveSpeed; }

float Ship::getRotateSpeed() const noexcept { return m_currentRotateSpeed; }

void Ship::resizeUpdate() {
    m_sprite.updateWindowSize();
    m_sprite.checkAspect({ 800, 600 });
    m_sprite.setPosition(m_position);
}

void Ship::forceStop() {
    m_currentMoveSpeed = 0;
    m_isMove = false;
    m_isInteract = true;
    m_position = m_lastPosition;
    m_sprite.setPosition(m_position);
}

bool Ship::isInteract() const noexcept { return m_isInteract; }
Position Ship::getPosition() const noexcept { return m_position; }

void Ship::setInteract(bool isInteract) { m_isInteract = isInteract; }

Player& Ship::getPlayer() noexcept { return m_player; }
