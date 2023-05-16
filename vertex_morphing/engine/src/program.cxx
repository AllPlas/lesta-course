//
// Created by Алексей Крукович on 16.05.23.
//

#include "program.hxx"

#include <fstream>
#include <vector>

#include "opengl_check.hxx"

using namespace std::literals;

static std::string readFile(const std::string_view path) {
    std::ifstream in{ path.data() };
    if (!in.is_open()) throw std::runtime_error{ "Error : readFile : bad open file"s };

    std::string result{};
    while (in) {
        std::string buf{};
        std::getline(in, buf);
        if (!in) break;
        result += buf;
        result += '\n';
    }

    return result;
}

Program::Program(const std::string_view vertPath, const std::string_view fragPath) {
    recompileShaders(vertPath, fragPath);
}
void Program::recompileShaders(const std::string_view vertPath, const std::string_view fragPath) {
    glDeleteProgram(m_program);
    openGLCheck();

    m_program = glCreateProgram();
    openGLCheck();

    auto vertexShader{ compileShader(GL_VERTEX_SHADER, vertPath) };
    auto fragmentShader{ compileShader(GL_FRAGMENT_SHADER, fragPath) };

    glAttachShader(m_program, vertexShader);
    openGLCheck();

    glAttachShader(m_program, fragmentShader);
    openGLCheck();

    glLinkProgram(m_program);
    openGLCheck();

    GLint linkedStatus{};
    glGetProgramiv(m_program, GL_LINK_STATUS, &linkedStatus);
    openGLCheck();

    if (linkedStatus == 0) {
        GLint infoLen = 0;
        glGetProgramiv(m_program, GL_INFO_LOG_LENGTH, &infoLen);
        openGLCheck();

        std::vector<char> infoChars(infoLen);
        glGetProgramInfoLog(m_program, infoLen, nullptr, infoChars.data());
        openGLCheck();

        glDeleteProgram(m_program);
        openGLCheck();

        throw std::runtime_error{ "Error : recompileShaders : linking error\n"s +
                                  infoChars.data() };
    }

    // NOTE: can use layout(location) instead this
    // glBindAttribLocation(m_program, 0, "a_position");
    // openGLCheck();

    glEnable(GL_DEPTH_TEST);
    openGLCheck();

    glDeleteShader(vertexShader);
    openGLCheck();

    glDeleteShader(fragmentShader);
    openGLCheck();
}
GLuint Program::compileShader(GLenum type, const std::string_view& path) {
    GLuint shader{ glCreateShader(type) };
    openGLCheck();
    std::string shaderSource{ readFile(path) };
    const char* source{ shaderSource.c_str() };

    glShaderSource(shader, 1, &source, nullptr);
    openGLCheck();

    glCompileShader(shader);
    openGLCheck();

    GLint compileStatus{};
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compileStatus);
    openGLCheck();
    if (compileStatus == 0) {
        GLint infoLen{};
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLen);
        openGLCheck();

        std::vector<char> infoChars(infoLen);
        glGetShaderInfoLog(shader, infoLen, nullptr, infoChars.data());
        openGLCheck();

        glDeleteShader(shader);
        openGLCheck();

        throw std::runtime_error{ "Error : createShader : bad compile " +
                                  (type == GL_VERTEX_SHADER ? "vertex"s : "fragment"s) +
                                  " shader\n"s + infoChars.data() };
    }

    return shader;
}
void Program::use() const { glUseProgram(m_program); }

GLuint Program::operator*() const noexcept { return m_program; }

Program::~Program() { glDeleteProgram(m_program); }
