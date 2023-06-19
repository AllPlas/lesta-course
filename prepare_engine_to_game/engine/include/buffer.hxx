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
    explicit VertexBuffer(const std::vector<V>& vertices);
    ~VertexBuffer();

    VertexBuffer(const VertexBuffer&) = delete;
    VertexBuffer& operator=(const VertexBuffer&) = delete;

    void updateData(std::vector<V>&& vertices);
    void updateData(const std::vector<V>& vertices);

    void addData(std::vector<V>&& vertices);
    void addData(const std::vector<V>& vertices);

    void clear();
    void bind() const;
    [[nodiscard]] std::size_t size() const noexcept;

private:
    void updateData() const;
};

template <typename T = std::int16_t>
class IndexBuffer final
{
private:
    std::vector<T> m_indices{};
    std::uint32_t m_indexBuffer{};

public:
    explicit IndexBuffer(std::vector<T>&& indices);
    explicit IndexBuffer(const std::vector<T>& indices);
    ~IndexBuffer();

    IndexBuffer(const IndexBuffer&) = delete;
    IndexBuffer& operator=(const IndexBuffer&) = delete;

    void updateData(std::vector<T>&& indices);
    void updateData(const std::vector<T>& indices);

    void addData(std::vector<T>&& indices);
    void addData(const std::vector<T>& indices);

    void clear();
    void bind() const;
    [[nodiscard]] std::size_t size() const noexcept;

private:
    void updateData() const;
};

#endif // VERTEX_MORPHING_BUFFER_HXX
