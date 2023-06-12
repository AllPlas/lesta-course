#include "texture.hxx"

#include <boost/gil/extension/io/png.hpp>

#include <glad/glad.h>

#include "opengl_check.hxx"

namespace gil = boost::gil;

Texture::~Texture() { glDeleteTextures(1, &m_texture); }

void Texture::load(const fs::path& path) {
    gil::rgba8_image_t image{};
    gil::read_and_convert_image(path, image, gil::png_tag{});

    int x = 0; // Координата x пикселя
    int y = 0; // Координата y пикселя
    std::cout << path << '\n';
    gil::rgba8_view_t::xy_locator locator = view(image).xy_at(x, y);
    unsigned char red = gil::nth_channel_view(locator, 0)[0];
    unsigned char green = gil::nth_channel_view(locator, 1)[0];
    unsigned char blue = gil::nth_channel_view(locator, 2)[0];
    unsigned char alpha = gil::nth_channel_view(locator, 3)[0];

    std::cout << "Pixel color at (" << 0 << ", " << 0 << "): ";
    std::cout << "R: " << static_cast<int>(red) << ", ";
    std::cout << "G: " << static_cast<int>(green) << ", ";
    std::cout << "B: " << static_cast<int>(blue) << ", ";
    std::cout << "A: " << static_cast<int>(alpha) << std::endl;

    load(gil::interleaved_view_get_raw_data(gil::view(image)), image.width(), image.height());
}

void Texture::load(const void* pixels, std::size_t width, std::size_t height) {
    glDeleteTextures(1, &m_texture);
    openGLCheck();

    glGenTextures(1, &m_texture);
    openGLCheck();

    bind();

    m_width = width;
    m_height = height;

    glTexImage2D(GL_TEXTURE_2D,
                 0,
                 GL_RGBA,
                 static_cast<GLsizei>(width),
                 static_cast<GLsizei>(height),
                 0,
                 GL_RGBA,
                 GL_UNSIGNED_BYTE,
                 pixels);
    openGLCheck();

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    openGLCheck();

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    openGLCheck();

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    openGLCheck();

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    openGLCheck();
}

std::uint32_t Texture::operator*() const noexcept { return m_texture; }

std::size_t Texture::getWidth() const noexcept { return m_width; }

std::size_t Texture::getHeight() const noexcept { return m_height; }

void Texture::bind() const {
    glBindTexture(GL_TEXTURE_2D, m_texture);
    openGLCheck();
}
