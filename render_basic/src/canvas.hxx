//
// Created by Алексей Крукович on 25.04.23.
//

#ifndef RENDER_BASIC_CANVAS_HXX
#define RENDER_BASIC_CANVAS_HXX

#include <algorithm>
#include <compare>
#include <cstdint>
#include <fstream>
#include <random>
#include <ranges>
#include <stdexcept>
#include <string_view>
#include <vector>

namespace graphics {

using namespace std::literals;

struct Color
{
    std::uint8_t red{};
    std::uint8_t green{};
    std::uint8_t blue{};

    auto operator<=>(const Color& color) const = default;
};

inline constexpr Color red{ 255, 0, 0 };
inline constexpr Color green{ 0, 255, 0 };
inline constexpr Color blue{ 0, 0, 255 };

using Pixels = std::vector<Color>;

static std::ofstream& operator<<(std::ofstream& out, const Pixels& pixels) {
    out.write(reinterpret_cast<const char*>(pixels.data()),
              pixels.size() * sizeof(Pixels::value_type));
    return out;
}

static std::ifstream& operator>>(std::ifstream& in, Pixels& pixels) {
    in.read(reinterpret_cast<char*>(pixels.data()), pixels.size() * sizeof(Pixels::value_type));
    return in;
}

struct Position
{
    std::size_t x{};
    std::size_t y{};

    auto operator<=>(const Position& position) const = default;

    static Position generateRandom(std::size_t width, std::size_t height) {
        static std::mt19937_64 engine{ std::random_device{}() };
        std::uniform_int_distribution<std::size_t> randX{ 0, width - 1 };
        std::uniform_int_distribution<std::size_t> randY{ 0, height - 1 };

        return { randX(engine), randY(engine) };
    }
};

class Canvas
{
private:
    std::size_t m_width{};
    std::size_t m_height{};
    Pixels m_pixels{};

public:
    Canvas(std::size_t width, std::size_t height)
        : m_width{ width }, m_height{ height }, m_pixels(m_width * m_height) {}

    void saveImage(std::string_view filename) {
        std::ofstream out{ filename.data(), std::ios::binary };
        out.exceptions(std::ios::failbit);
        out << "P6\n"sv << m_width << ' ' << m_height << '\n' << 255 << '\n';
        out << m_pixels;
    }

    void loadImage(std::string_view filename) {
        std::ifstream in{ filename.data(), std::ios::binary };
        in.exceptions(std::ios::failbit);
        std::string header{};
        std::string colorFormat{};
        char ws{};
        in >> header >> m_width >> m_height >> colorFormat >> std::noskipws >> ws;
        if (!std::iswspace(ws))
            throw std::runtime_error{ "Error : loadImage : bad load image from file"s };

        m_pixels.resize(m_width * m_height);
        in >> m_pixels;
    }

    void setPixel(Position position, Color color) {
        m_pixels.at(position.y * m_width + position.x) = color;
    }

    [[nodiscard]] Color getPixel(Position position) const noexcept {
        return m_pixels.at(position.y * m_width + position.x);
    }

    void clear(Color color = {}) { std::ranges::fill(m_pixels, color); }

    [[nodiscard]] const Pixels& getPixels() const noexcept { return m_pixels; }
    [[nodiscard]] std::size_t getWidth() const noexcept { return m_width; }
    [[nodiscard]] std::size_t getHeight() const noexcept { return m_height; }

    [[nodiscard]] auto begin() { return m_pixels.begin(); }
    [[nodiscard]] auto end() { return m_pixels.end(); }

    auto operator<=>(const Canvas& canvas) const = default;
};

using Positions = std::vector<Position>;

class IRender
{
public:
    virtual ~IRender() = default;

    virtual Positions pixelsPositions(Position start, Position end) const = 0;
};

} // namespace graphics

#endif // RENDER_BASIC_CANVAS_HXX