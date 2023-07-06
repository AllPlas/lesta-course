#include "texture.hxx"

#include <glad/glad.h>

#include "opengl_check.hxx"

#ifndef __ANDROID__
#    include <boost/gil/extension/io/png.hpp>
namespace gil = boost::gil;
#endif

Texture::~Texture() {
    if (m_copied) glDeleteTextures(1, &m_texture);
}

#ifndef __ANDROID__

void Texture::load(const fs::path& path) {
    gil::rgba8_image_t image{};
    gil::read_and_convert_image(path, image, gil::png_tag{});

    load(gil::interleaved_view_get_raw_data(gil::view(image)), image.width(), image.height());
}
#else
void Texture::load(const fs::path& path) {
    m_image.load(path);

    load(m_image.getPixels(), m_image.getWidth(), m_image.getHeight());
}
#endif

void Texture::load(const void* pixels, std::size_t width, std::size_t height) {
    if (m_copied) {
        glDeleteTextures(1, &m_texture);
        openGLCheck();
    }

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

Texture::Texture(Texture& texture)
    : m_texture{ texture.m_texture }, m_width{ texture.m_width }, m_height{ texture.m_height } {
    texture.m_copied = true;
}

Texture& Texture::operator=(Texture& texture) {
    m_texture = texture.m_texture;
    m_width = texture.m_width;
    m_height = texture.m_height;
    texture.m_copied = true;
    return *this;
}

#ifdef __ANDROID__
#    include <SDL3/SDL.h>

#    define STB_IMAGE_IMPLEMENTATION
#    include "stb_image.h"

Image::Image(const Image& image) : m_filePath{ image.m_filePath } {
    if (!m_filePath.empty()) load(image.m_filePath);
}

Image& Image::operator=(const Image& image) {
    if (m_image != nullptr) stbi_image_free(m_image);
    m_filePath = image.m_filePath;
    if (!m_filePath.empty()) load(m_filePath);

    return *this;
}

Image::~Image() {
    if (m_image != nullptr) stbi_image_free(m_image);
}

void Image::load(const fs::path& path) {
    int channels{};
    auto imageFile{ readFile(path) };
    if (m_image != nullptr) stbi_image_free(m_image);
    m_image = stbi_load_from_memory(reinterpret_cast<const unsigned char*>(imageFile.data()),
                                    imageFile.size(),
                                    &m_width,
                                    &m_height,
                                    &channels,
                                    0);
}

std::vector<char> Image::readFile(const fs::path& path) {
    SDL_RWops* io{ SDL_RWFromFile(path.c_str(), "rb") };
    if (io == nullptr)
        throw std::runtime_error{ "Error : readFile : bad load file "s + path.string() };

    Sint64 fileSize{ io->size(io) };
    if (fileSize == -1)
        throw std::runtime_error{ "Error : readFile : can't determine size of file "s +
                                  path.string() };

    std::vector<char> buf(fileSize);

    auto numReadObjects{ io->read(io, buf.data(), fileSize) };
    if (numReadObjects != fileSize)
        throw std::runtime_error{ "Error : readFile : can't read all content from file "s +
                                  path.string() };

    if (io->close(io) != 0)
        throw std::runtime_error{ "Error : readFile : failed to close file "s + path.string() };

    return buf;
}

const unsigned char* Image::getPixels() const noexcept { return m_image; }

int Image::getWidth() const noexcept { return m_width; }

int Image::getHeight() const noexcept { return m_height; }
#endif
