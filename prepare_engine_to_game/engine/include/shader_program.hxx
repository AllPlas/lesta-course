//
// Created by Алексей Крукович on 16.05.23.
//

#ifndef VERTEX_MORPHING_PROGRAM_HXX
#define VERTEX_MORPHING_PROGRAM_HXX

#include <filesystem>

#include "texture.hxx"

namespace fs = std::filesystem;

class ShaderProgram final
{
private:
    std::uint32_t m_program{};

public:
    ShaderProgram() = default;
    ShaderProgram(const fs::path& vertPath, const fs::path& fragPath);
    ~ShaderProgram();

    ShaderProgram(const ShaderProgram&) = delete;
    ShaderProgram& operator=(const ShaderProgram&) = delete;

    void recompileShaders(const fs::path& vertPath, const fs::path& fragPath);
    void use() const;
    void setUniform(std::string_view name, float value) const;
    void setUniform(std::string_view name, const Texture& texture) const;

    std::uint32_t operator*() const noexcept;

private:
    static std::uint32_t compileShader(std::uint32_t type, const fs::path& path);
};

#endif // VERTEX_MORPHING_PROGRAM_HXX
