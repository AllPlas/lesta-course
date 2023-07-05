#ifndef ENGINE_PREPARE_TO_GAME_STRUCTURES_HXX
#define ENGINE_PREPARE_TO_GAME_STRUCTURES_HXX

#include <cmath>
#include <optional>

#ifndef __ANDROID__
#    include <compare>
#    include <numbers>
#else
namespace std::numbers {
constexpr double pi = 3.14159265358979323846;
}
#endif

struct Position
{
    float x{};
    float y{};

#ifndef __ANDROID__
    auto operator<=> (const Position& position) const = default;
#else
    bool operator==(const Position& position) const = default;
#endif
};

struct Size
{
    float width{};
    float height{};

#ifndef __ANDROID__
    auto operator<=> (const Size& size) const = default;
#else
    bool operator==(const Size& size) const = default;
#endif
};

struct Scale
{
    float x{ 1.0f };
    float y{ 1.0f };

#ifndef __ANDROID__
    auto operator<=> (const Scale& scale) const = default;
#else
    bool operator==(const Scale& scale) const = default;
#endif
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

#ifndef __ANDROID__
    auto operator<=> (const Angle& angle) const = default;
#else
    bool operator==(const Angle& angle) const = default;
#endif
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
    void rotate(const Angle& angle) noexcept;

#ifndef __ANDROID__
    auto operator<=> (const Rectangle&) const = default;
#else
    bool operator==(const Rectangle&) const = default;
#endif
};

std::optional<LineSegment> intersect(LineSegment l1, LineSegment l2);
std::optional<Rectangle> intersect(const Rectangle& r1, const Rectangle& r2);

#endif // ENGINE_PREPARE_TO_GAME_STRUCTURES_HXX
