#ifndef ENGINE_PREPARE_TO_GAME_STRUCTURES_HXX
#define ENGINE_PREPARE_TO_GAME_STRUCTURES_HXX

#include <cmath>
#include <numbers>
#include <optional>

struct Position
{
    float x{};
    float y{};
};

struct Size
{
    float width{};
    float height{};
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
};

std::optional<LineSegment> intersect(LineSegment l1, LineSegment l2);
std::optional<Rectangle> intersect(const Rectangle& r1, const Rectangle& r2);

#endif // ENGINE_PREPARE_TO_GAME_STRUCTURES_HXX
