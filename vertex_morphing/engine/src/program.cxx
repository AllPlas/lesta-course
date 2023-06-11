#include "program.hxx"

#include <fstream>
#include <glad/glad.h>
#include <vector>

#include "opengl_check.hxx"

using namespace std::literals;

static std::string readFile(const fs::path& path) {
    std::ifstream in{ path };
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

ShaderProgram::ShaderProgram(const fs::path& vertPath, const fs::path& fragPath) {
    recompileShaders(vertPath, fragPath);
}

void ShaderProgram::recompileShaders(const fs::path& vertPath, const fs::path& fragPath) {
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

    glDeleteShader(vertexShader);
    openGLCheck();

    glDeleteShader(fragmentShader);
    openGLCheck();
}

GLuint ShaderProgram::compileShader(GLenum type, const fs::path& path) {
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
void ShaderProgram::use() const {
    glUseProgram(m_program);
    openGLCheck();
}

GLuint ShaderProgram::operator*() const noexcept { return m_program; }

ShaderProgram::~ShaderProgram() {
    if (m_program) glDeleteProgram(m_program);
}
