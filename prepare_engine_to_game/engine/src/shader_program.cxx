#include "shader_program.hxx"

#include <glm/gtc/type_ptr.hpp>

#include <fstream>
#include <glad/glad.h>
#include <vector>

#include "opengl_check.hxx"

using namespace std::literals;

#ifndef __ANDROID__
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
#else
#    include <SDL3/SDL.h>

static std::string readFile(const fs::path& path) {
    SDL_RWops* io = SDL_RWFromFile(path.c_str(), "rb");
    if (nullptr == io) { throw std::runtime_error("can't load file: " + std::string(path)); }

    Sint64 file_size = io->size(io);
    if (-1 == file_size) {
        throw std::runtime_error("can't determine size of file: " + std::string(path));
    }
    const size_t size = static_cast<size_t>(file_size);
    std::vector<char> mem(size);

    const size_t num_read_objects = io->read(io, mem.data(), size);
    if (num_read_objects != size) {
        throw std::runtime_error("can't read all content from file: " + std::string(path));
    }

    if (0 != io->close(io)) {
        throw std::runtime_error("failed to close file: " + std::string(path));
    }

    return { mem.begin(), mem.end() };
}
#endif

ShaderProgram::ShaderProgram(const fs::path& vertPath, const fs::path& fragPath) {
    recompileShaders(vertPath, fragPath);
}

void ShaderProgram::recompileShaders(const fs::path& vertPath, const fs::path& fragPath) {
    if(m_program) glDeleteProgram(m_program);
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
    std::string shaderSource{ s_glslVersion + '\n' + readFile(path) };
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

void ShaderProgram::setUniform(std::string_view name, float value) const {
    auto location{ glGetUniformLocation(m_program, name.data()) };
    openGLCheck();

    glUniform1f(location, value);
    openGLCheck();
}

void ShaderProgram::setUniform(std::string_view name, const Texture& texture) const {
    auto location{ glGetUniformLocation(m_program, name.data()) };
    openGLCheck();

    glUniform1i(location, 0);
    openGLCheck();

    glActiveTexture(GL_TEXTURE0);
    openGLCheck();
}

void ShaderProgram::setUniform(std::string_view name, const glm::mat3& matrix) const {
    auto location{ glGetUniformLocation(m_program, name.data()) };
    openGLCheck();

    glUniformMatrix3fv(location, 1, GL_FALSE, glm::value_ptr(matrix));
    openGLCheck();
}

void ShaderProgram::setGLSLVersion(const std::string& version) { s_glslVersion = version; }

void ShaderProgram::clear() {
    glDeleteProgram(m_program);
    openGLCheck();
    m_program = 0;
}
