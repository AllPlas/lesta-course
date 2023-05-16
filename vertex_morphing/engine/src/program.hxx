//
// Created by Алексей Крукович on 16.05.23.
//

#ifndef VERTEX_MORPHING_PROGRAM_HXX
#define VERTEX_MORPHING_PROGRAM_HXX

#include <glad/glad.h>
#include <string_view>

class Program final
{
private:
    GLuint m_program{};

public:
    Program() = default;
    Program(std::string_view vertPath, std::string_view fragPath);
    ~Program();

    Program(const Program&) = delete;
    Program& operator=(const Program&) = delete;

    void recompileShaders(const std::string_view vertPath, const std::string_view fragPath);
    void use() const;

    GLuint operator*() const noexcept;

private:
    static GLuint compileShader(GLenum type, const std::string_view& path);
};

#endif // VERTEX_MORPHING_PROGRAM_HXX
