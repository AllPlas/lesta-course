#include "player.hxx"

Player::Player(const fs::path& texturePath, Size size) : m_sprite{ size } {
    m_textures.try_emplace("back");
    m_textures.try_emplace("front");
    m_textures.try_emplace("left");
    m_textures.try_emplace("right");

    m_textures["back"].load("data/assets/pirate/back/back_standing.png");
    m_textures["front"].load("data/assets/pirate/front/front_standing.png");
    m_textures["left"].load("data/assets/pirate/left/left_standing.png");
    m_textures["right"].load("data/assets/pirate/right/right_standing.png");
}

Sprite& Player::getSprite() noexcept { return m_sprite; }

void Player::moveUp() {
    m_isMoveUp = true;
    m_sprite.setTexture(m_textures["back"]);
}

void Player::moveDown() {
    m_isMoveDown = true;
    m_sprite.setTexture(m_textures["front"]);
}

void Player::moveLeft() {
    m_isMoveLeft = true;
    m_sprite.setTexture(m_textures["left"]);
}

void Player::moveRight() {
    m_isMoveRight = true;
    m_sprite.setTexture(m_textures["right"]);
}

void Player::stopMoveUp() { m_isMoveUp = false; }

void Player::stopMoveLeft() { m_isMoveLeft = false; }

void Player::stopMoveRight() { m_isMoveRight = false; }

void Player::stopMoveDown() { m_isMoveDown = false; }

void Player::setPosition(Position position) {
    m_position = position;
    m_sprite.setPosition(m_position);
    m_sprite.setTexture(m_textures["front"]);
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
    if (m_isMoveUp) m_position.y += m_speed * timeElapsedInSec;
    if (m_isMoveDown) m_position.y -= m_speed * timeElapsedInSec;
    if (m_isMoveLeft) m_position.x -= m_speed * timeElapsedInSec;
    if (m_isMoveRight) m_position.x += m_speed * timeElapsedInSec;

    m_sprite.setPosition(m_position);
}

const Sprite& Player::getSprite() const noexcept { return m_sprite; }

void Player::tryDig() { m_isDigging = true; }

bool Player::isDigging() const noexcept { return m_isDigging; }
void Player::stopDig() { m_isDigging = false; }

void Player::addMoney(int money) { m_money += money; }
int Player::getMoney() const noexcept { return m_money; }

bool Player::isNearShip() const noexcept { return m_isNearShip; }
void Player::setNearShip(bool nearShip) { m_isNearShip = nearShip; }
