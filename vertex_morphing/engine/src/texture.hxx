//
// Created by Алексей Крукович on 18.05.23.
//

#ifndef VERTEX_MORPHING_TEXTURE_HXX
#define VERTEX_MORPHING_TEXTURE_HXX

#include <filesystem>
#include <glad/glad.h>

using namespace std::literals;
namespace fs = std::filesystem;

class Texture final
{
private:
    GLuint m_texture;
    std::size_t m_width{};
    std::size_t m_height{};

public:
    Texture();
    ~Texture();

    void load(const fs::path& path);
    void load(const void* pixels, std::size_t width, std::size_t height);
    void bind() const;

    int getWidth() const noexcept;
    int getHeight() const noexcept;

    GLuint operator*() const noexcept;
};

#endif // VERTEX_MORPHING_TEXTURE_HXX
