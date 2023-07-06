#ifndef VERTEX_MORPHING_TEXTURE_HXX
#define VERTEX_MORPHING_TEXTURE_HXX

#include <filesystem>

using namespace std::literals;
namespace fs = std::filesystem;

#ifdef __ANDROID__
#    include <vector>

class Image
{
private:
    fs::path m_filePath{};
    unsigned char* m_image{};
    int m_height{};
    int m_width{};

public:
    Image() = default;
    Image(const Image& image);
    Image& operator=(const Image& image);

    ~Image();

    void load(const fs::path& path);
    [[nodiscard]] const unsigned char* getPixels() const noexcept;
    [[nodiscard]] int getWidth() const noexcept;
    [[nodiscard]] int getHeight() const noexcept;

private:
    static std::vector<char> readFile(const fs::path& path);
};
#endif

class Texture final
{
private:
    std::uint32_t m_texture{};
    std::size_t m_width{};
    std::size_t m_height{};
#ifdef __ANDROID__
    Image m_image{};
#endif

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
