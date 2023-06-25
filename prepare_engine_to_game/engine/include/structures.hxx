#ifndef ENGINE_PREPARE_TO_GAME_STRUCTURES_HXX
#define ENGINE_PREPARE_TO_GAME_STRUCTURES_HXX

#include <cmath>
#include <compare>
#include <numbers>
#include <optional>

struct Position
{
    float x{};
    float y{};

    bool operator<=>(const Position& position) const = default;
};

struct Size
{
    float width{};
    float height{};

    bool operator<=>(const Size& size) const = default;
};

struct Scale
{
    float x{ 1.0f };
    float y{ 1.0f };

    bool operator<=>(const Scale& scale) const = default;
};

class Angle
{
private:
    float m_angleInDegrees{};

public:
    Angle& operator=(float angleInDegrees) {
        m_angleInDegrees = angleInDegrees;
        return *this;
    }

    [[nodiscard]] float getInRadians() const noexcept;
    [[nodiscard]] float getInDegrees() const noexcept;

    bool operator<=>(const Angle& angle) const = default;
};

struct LineSegment
{
    // start <= end;
    float start{};
    float end{};
};

struct Rectangle
{
    Position xy{};
    Size wh{};

    [[nodiscard]] bool contains(Position position) const noexcept;
};

std::optional<LineSegment> intersect(LineSegment l1, LineSegment l2);
std::optional<Rectangle> intersect(const Rectangle& r1, const Rectangle& r2);

#endif // ENGINE_PREPARE_TO_GAME_STRUCTURES_HXX
