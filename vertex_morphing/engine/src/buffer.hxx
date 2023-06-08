//
// Created by Алексей Крукович on 6.06.23.
//

#ifndef VERTEX_MORPHING_BUFFER_HXX
#define VERTEX_MORPHING_BUFFER_HXX

#include <cstdint>
#include <iosfwd>
#include <memory>
#include <vector>

struct Vertex
{
    float x{};
    float y{};
    float z{};

    float texX{};
    float texY{};
};

struct Vertex2
{
    float x{};
    float y{};

    float texX{};
    float texY{};

    std::uint32_t rgba{};
};

std::ifstream& operator>>(std::ifstream& in, Vertex& vertex);
std::ifstream& operator>>(std::ifstream& in, Vertex2& vertex);

template <typename V = Vertex2>
class VertexBuffer final
{
private:
    std::vector<V> m_vertices{};
    std::uint32_t m_vertexBuffer{};

public:
    explicit VertexBuffer(std::vector<V>&& vertices);
    ~VertexBuffer();

    void bind() const;
    [[nodiscard]] std::size_t size() const noexcept;
};

template <typename T = std::int16_t>
class IndexBuffer final
{
private:
    std::vector<T> m_indices{};
    std::uint32_t m_indexBuffer{};

public:
    explicit IndexBuffer(std::vector<T>&& indices);
    ~IndexBuffer();

    void bind() const;
    [[nodiscard]] std::size_t size() const noexcept;
};

#endif // VERTEX_MORPHING_BUFFER_HXX
