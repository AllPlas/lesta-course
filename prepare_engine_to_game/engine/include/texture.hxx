#ifndef VERTEX_MORPHING_TEXTURE_HXX
#define VERTEX_MORPHING_TEXTURE_HXX

#include <filesystem>

using namespace std::literals;
namespace fs = std::filesystem;

class Texture final
{
private:
    std::uint32_t m_texture{};
    std::size_t m_width{};
    std::size_t m_height{};

    bool m_copied{};

public:
    Texture() = default;
    Texture(Texture& texture);
    Texture& operator=(Texture& texture);

    Texture(Texture&& texture) = delete;
    Texture& operator=(Texture&& texture) = delete;

    ~Texture();

    void load(const fs::path& path);
    void load(const void* pixels, std::size_t width, std::size_t height);
    void bind() const;

    [[nodiscard]] std::size_t getWidth() const noexcept;
    [[nodiscard]] std::size_t getHeight() const noexcept;

    std::uint32_t operator*() const noexcept;
};

#endif // VERTEX_MORPHING_TEXTURE_HXX
