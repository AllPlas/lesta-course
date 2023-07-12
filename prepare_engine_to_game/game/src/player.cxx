#include "player.hxx"

Player::Player(const fs::path& texturePath, Size size)
    : m_sprite{ size }, m_digAudio{ std::make_unique<Audio>("data/audio/dig.wav") } {
    m_textures["back"][0].load("data/assets/pirate/back/back_standing.png");
    m_textures["back"][1].load("data/assets/pirate/back/back_walking_a.png");
    m_textures["back"][2].load("data/assets/pirate/back/back_walking_b.png");

    m_textures["front"][0].load("data/assets/pirate/front/front_standing.png");
    m_textures["front"][1].load("data/assets/pirate/front/front_walking_a.png");
    m_textures["front"][2].load("data/assets/pirate/front/front_walking_b.png");

    m_textures["left"][0].load("data/assets/pirate/left/left_standing.png");
    m_textures["left"][1].load("data/assets/pirate/left/left_walking_a.png");
    m_textures["left"][2].load("data/assets/pirate/left/left_walking_b.png");

    m_textures["right"][0].load("data/assets/pirate/right/right_standing.png");
    m_textures["right"][1].load("data/assets/pirate/right/right_walking_a.png");
    m_textures["right"][2].load("data/assets/pirate/right/right_walking_b.png");
}

Sprite& Player::getSprite() noexcept { return m_sprite; }

void Player::moveUp() { m_isMoveUp = true; }

void Player::moveDown() { m_isMoveDown = true; }

void Player::moveLeft() { m_isMoveLeft = true; }

void Player::moveRight() { m_isMoveRight = true; }

void Player::stopMoveUp() {
    m_isMoveUp = false;
    m_animUpIndex = 0;
}

void Player::stopMoveLeft() {
    m_isMoveLeft = false;
    m_animLeftIndex = 0;
}

void Player::stopMoveRight() {
    m_isMoveRight = false;
    m_animRightIndex = 0;
}

void Player::stopMoveDown() {
    m_isMoveDown = false;
    m_animDownIndex = 0;
}

void Player::setPosition(Position position) {
    m_position = position;
    m_sprite.setPosition(m_position);
    m_sprite.setTexture(m_textures["front"][0]);
}

Position Player::getPosition() const noexcept { return m_position; }

void Player::resizeUpdate() {
    m_sprite.updateWindowSize();
    m_sprite.checkAspect({ 800, 600 });
    m_sprite.setPosition(m_position);
}

void Player::forceStop() {
    m_position = m_lastPosition;
    m_sprite.setPosition(m_position);
}

void Player::update(std::chrono::microseconds timeElapsed) {
    float timeElapsedInSec{ static_cast<float>(timeElapsed.count()) / 1000000.0f };

    m_lastPosition = m_position;

    if (m_isMoveLeft) {
        m_position.x -= m_speed * timeElapsedInSec;
        m_animLeftIndex += s_animBoost * timeElapsedInSec;
        m_sprite.setTexture(m_textures["left"][static_cast<std::size_t>(m_animLeftIndex) %
                                               m_textures["left"].size()]);
    }

    if (m_isMoveRight) {
        m_position.x += m_speed * timeElapsedInSec;
        m_animRightIndex += s_animBoost * timeElapsedInSec;
        m_sprite.setTexture(m_textures["right"][static_cast<std::size_t>(m_animRightIndex) %
                                                m_textures["right"].size()]);
    }

    if (m_isMoveUp) {
        m_position.y += m_speed * timeElapsedInSec;
        m_animUpIndex += s_animBoost * timeElapsedInSec;
        m_sprite.setTexture(m_textures["back"][static_cast<std::size_t>(m_animUpIndex) %
                                               m_textures["back"].size()]);
    }

    if (m_isMoveDown) {
        m_position.y -= m_speed * timeElapsedInSec;
        m_animDownIndex += s_animBoost * timeElapsedInSec;
        m_sprite.setTexture(m_textures["front"][static_cast<std::size_t>(m_animDownIndex) %
                                                m_textures["front"].size()]);
    }

    m_sprite.setPosition(m_position);
}

const Sprite& Player::getSprite() const noexcept { return m_sprite; }

void Player::tryDig() {
    m_isDigging = true;
    m_digAudio->play();
}

bool Player::isDigging() const noexcept { return m_isDigging; }
void Player::stopDig() { m_isDigging = false; }

void Player::addMoney(int money) { m_money += money; }
int Player::getMoney() const noexcept { return m_money; }
void Player::clearMoney() { m_money = 0; }

bool Player::isNearShip() const noexcept { return m_isNearShip; }
void Player::setNearShip(bool nearShip) { m_isNearShip = nearShip; }

bool Player::hasBottle() const noexcept { return m_hasBottle; }
void Player::setBottle(bool hasBottle) { m_hasBottle = hasBottle; }
