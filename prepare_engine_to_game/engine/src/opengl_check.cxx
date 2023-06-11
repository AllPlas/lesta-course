//
// Created by Алексей Крукович on 16.05.23.
//

#include "opengl_check.hxx"

#include <glad/glad.h>
#include <iostream>

using namespace std::literals;

void openGLCheck() {
    const GLenum err = glGetError();
    if (err != GL_NO_ERROR) {
        switch (err) {
        case GL_INVALID_ENUM:
            std::cerr << "GL_INVALID_ENUM" << std::endl;
            break;
        case GL_INVALID_VALUE:
            std::cerr << "GL_INVALID_VALUE" << std::endl;
            break;
        case GL_INVALID_OPERATION:
            std::cerr << "GL_INVALID_OPERATION" << std::endl;
            break;
        case GL_INVALID_FRAMEBUFFER_OPERATION:
            std::cerr << "GL_INVALID_FRAMEBUFFER_OPERATION" << std::endl;
            break;
        case GL_OUT_OF_MEMORY:
            std::cerr << "GL_OUT_OF_MEMORY" << std::endl;
            break;
        default:
            std::cerr << "UNKNOWN ERROR" << std::endl;
            break;
        }
        std::cerr << __FILE__ << ':' << __LINE__ << '(' << __FUNCTION__ << ')' << std::endl;
        throw std::runtime_error{""s};
    }
}
