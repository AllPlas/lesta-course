#include "structures.hxx"

#include <algorithm>

float Angle::getInRadians() const noexcept { return m_angleInDegrees * std::numbers::pi / 180.0f; }
float Angle::getInDegrees() const noexcept { return m_angleInDegrees; }

bool Rectangle::contains(Position position) const noexcept {
    return (position.x >= xy.x && position.x <= xy.x + wh.width && position.y >= xy.y &&
            position.y <= xy.y + wh.height);
}

void Rectangle::rotate(const Angle& angle) noexcept {
    const float centerX = xy.x + wh.width / 2.0f;
    const float centerY = xy.y + wh.height / 2.0f;

    const float cosine = std::cos(angle.getInRadians());
    const float sine = std::sin(angle.getInRadians());

    // Поворот вершин
    const float rotatedTopLeftX = (xy.x - centerX) * cosine - (xy.y - centerY) * sine + centerX;
    const float rotatedTopLeftY = (xy.x - centerX) * sine + (xy.y - centerY) * cosine + centerY;

    const float rotatedTopRightX =
        (xy.x + wh.width - centerX) * cosine - (xy.y - centerY) * sine + centerX;
    const float rotatedTopRightY =
        (xy.x + wh.width - centerX) * sine + (xy.y - centerY) * cosine + centerY;

    const float rotatedBottomLeftX =
        (xy.x - centerX) * cosine - (xy.y + wh.height - centerY) * sine + centerX;
    const float rotatedBottomLeftY =
        (xy.x - centerX) * sine + (xy.y + wh.height - centerY) * cosine + centerY;

    const float rotatedBottomRightX =
        (xy.x + wh.width - centerX) * cosine - (xy.y + wh.height - centerY) * sine + centerX;
    const float rotatedBottomRightY =
        (xy.x + wh.width - centerX) * sine + (xy.y + wh.height - centerY) * cosine + centerY;

    // Нахождение новых координат xy и размеров wh
    float minX =
        std::min({ rotatedTopLeftX, rotatedTopRightX, rotatedBottomLeftX, rotatedBottomRightX });
    float minY =
        std::min({ rotatedTopLeftY, rotatedTopRightY, rotatedBottomLeftY, rotatedBottomRightY });
    float maxX =
        std::max({ rotatedTopLeftX, rotatedTopRightX, rotatedBottomLeftX, rotatedBottomRightX });
    float maxY =
        std::max({ rotatedTopLeftY, rotatedTopRightY, rotatedBottomLeftY, rotatedBottomRightY });

    xy.x = minX;
    xy.y = minY;
    wh.width = maxX - minX;
    wh.height = maxY - minY;
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
