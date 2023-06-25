#include "structures.hxx"

float Angle::getInRadians() const noexcept { return m_angleInDegrees * std::numbers::pi / 180.0f; }
float Angle::getInDegrees() const noexcept { return m_angleInDegrees; }

bool Rectangle::contains(Position position) const noexcept {
    return (position.x >= xy.x && position.x <= xy.x + wh.width && position.y >= xy.y &&
            position.y <= xy.y + wh.height);
}

std::optional<LineSegment> intersect(LineSegment l1, LineSegment l2) {
    float left{ std::max(l1.start, l2.start) };
    float right{ std::min(l1.end, l2.end) };

    if (right < left) return std::nullopt;

    return LineSegment{ .start = left, .end = right };
}

LineSegment projectX(const Rectangle& rectangle) {
    return LineSegment{ .start = rectangle.xy.x, .end = rectangle.xy.x + rectangle.wh.width };
}

LineSegment projectY(const Rectangle& rectangle) {
    return LineSegment{ .start = rectangle.xy.y, .end = rectangle.xy.y + rectangle.wh.height };
}

std::optional<Rectangle> intersect(const Rectangle& r1, const Rectangle& r2) {
    auto px{ intersect(projectX(r1), projectX(r2)) };
    auto py{ intersect(projectY(r1), projectY(r2)) };

    if (!px || !py) return std::nullopt;

    return Rectangle{ .xy = { px->start, py->start },
                      .wh = { px->end - px->start, py->end - py->start } };
}
