//
// Created by Алексей Крукович on 6.06.23.
//

#include "buffer.hxx"

#include <fstream>
#include <glad/glad.h>

#include "opengl_check.hxx"

std::ifstream& operator>>(std::ifstream& in, Vertex& vertex) {
    in >> vertex.x >> vertex.y >> vertex.z >> vertex.texX >> vertex.texY;

    return in;
}

std::ifstream& operator>>(std::ifstream& in, Vertex2& vertex) {
    in >> vertex.x >> vertex.y >> vertex.texX >> vertex.texY >> vertex.rgba;

    return in;
}

template <typename V>
VertexBuffer<V>::VertexBuffer(std::vector<V>&& vertices) : m_vertices{ std::move(vertices) } {
    glGenBuffers(1, &m_vertexBuffer);
    openGLCheck();

    bind();

    glBufferData(GL_ARRAY_BUFFER, m_vertices.size() * sizeof(V), m_vertices.data(), GL_STATIC_DRAW);
    openGLCheck();
}

template <typename V>
void VertexBuffer<V>::bind() const {
    glBindBuffer(GL_ARRAY_BUFFER, m_vertexBuffer);
    openGLCheck();
}

template <typename V>
VertexBuffer<V>::~VertexBuffer() {
    glDeleteBuffers(1, &m_vertexBuffer);
}

template <typename T>
IndexBuffer<T>::IndexBuffer(std::vector<T>&& indices) : m_indices{ std::move(indices) } {
    glGenBuffers(1, &m_indexBuffer);
    openGLCheck();

    bind();

    glBufferData(
        GL_ELEMENT_ARRAY_BUFFER, m_indices.size() * sizeof(T), m_indices.data(), GL_STATIC_DRAW);
    openGLCheck();
}

template <typename T>
void IndexBuffer<T>::bind() const {
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indexBuffer);
    openGLCheck();
}

template <typename T>
IndexBuffer<T>::~IndexBuffer() {
    glDeleteBuffers(1, &m_indexBuffer);
}

template class VertexBuffer<Vertex>;
template class VertexBuffer<Vertex2>;

template class IndexBuffer<std::uint8_t>;
template class IndexBuffer<std::uint16_t>;
template class IndexBuffer<std::uint32_t>;
template class IndexBuffer<std::uint64_t>;