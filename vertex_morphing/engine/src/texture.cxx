//
// Created by Алексей Крукович on 18.05.23.
//

#include "texture.hxx"

#include <boost/gil/extension/io/png.hpp>

#include "opengl_check.hxx"

namespace gil = boost::gil;

Texture::Texture() = default;

Texture::~Texture() { glDeleteTextures(1, &m_texture); }

void Texture::load(const fs::path& path) {
    glDeleteTextures(1, &m_texture);
    openGLCheck();

    glGenTextures(1, &m_texture);
    openGLCheck();

    glBindTexture(GL_TEXTURE_2D, m_texture);
    openGLCheck();

    gil::rgba8_image_t image{};
    gil::read_and_convert_image(path, image, gil::png_tag{});

    m_width = image.width();
    m_height = image.height();

    glTexImage2D(GL_TEXTURE_2D,
                 0,
                 GL_RGBA,
                 image.width(),
                 image.height(),
                 0,
                 GL_RGBA,
                 GL_UNSIGNED_BYTE,
                 gil::interleaved_view_get_raw_data(gil::view(image)));
    openGLCheck();

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    openGLCheck();

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    openGLCheck();
}

GLuint Texture::operator*() const noexcept { return m_texture; }

int Texture::getWidth() const noexcept { return m_width; }

int Texture::getHeight() const noexcept { return m_height; }

void Texture::bind() const {
    glBindTexture(GL_TEXTURE_2D, m_texture);
    openGLCheck();
};
