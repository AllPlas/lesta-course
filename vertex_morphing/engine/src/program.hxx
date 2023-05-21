//
// Created by Алексей Крукович on 16.05.23.
//

#ifndef VERTEX_MORPHING_PROGRAM_HXX
#define VERTEX_MORPHING_PROGRAM_HXX

#include <filesystem>
#include <glad/glad.h>

namespace fs = std::filesystem;

class Program final
{
private:
    GLuint m_program{};

public:
    Program() = default;
    Program(const fs::path& vertPath, const fs::path& fragPath);
    ~Program();

    Program(const Program&) = delete;
    Program& operator=(const Program&) = delete;

    void recompileShaders(const fs::path& vertPath, const fs::path& fragPath);
    void use() const;

    GLuint operator*() const noexcept;

private:
    static GLuint compileShader(GLenum type, const fs::path& path);
};

#endif // VERTEX_MORPHING_PROGRAM_HXX
