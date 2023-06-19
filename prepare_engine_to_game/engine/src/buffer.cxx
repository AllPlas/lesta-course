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

    updateData();
}

template <typename V>
VertexBuffer<V>::VertexBuffer(const std::vector<V>& vertices) : m_vertices{ vertices } {
    glGenBuffers(1, &m_vertexBuffer);
    openGLCheck();

    updateData();
}

template <typename V>
void VertexBuffer<V>::updateData(std::vector<V>&& vertices) {
    m_vertices = std::move(vertices);
    updateData();
}

template <typename V>
void VertexBuffer<V>::updateData(const std::vector<V>& vertices) {
    m_vertices = vertices;
    updateData();
}

template <typename V>
void VertexBuffer<V>::updateData() const {
    bind();

    glBufferData(GL_ARRAY_BUFFER, m_vertices.size() * sizeof(V), m_vertices.data(), GL_STATIC_DRAW);
    openGLCheck();
}

template <typename V>
void VertexBuffer<V>::addData(std::vector<V>&& vertices) {
    m_vertices.insert(m_vertices.end(),
                      std::make_move_iterator(vertices.begin()),
                      std::make_move_iterator(vertices.end()));

    updateData();
}

template <typename V>
void VertexBuffer<V>::addData(const std::vector<V>& vertices) {
    m_vertices.insert(m_vertices.end(), vertices.begin(), vertices.end());

    updateData();
}

template <typename V>
void VertexBuffer<V>::clear() {
    m_vertices.clear();

    glBufferData(GL_ARRAY_BUFFER, 0, nullptr, GL_STATIC_DRAW);
    openGLCheck();
}

template <typename V>
void VertexBuffer<V>::bind() const {
    glBindBuffer(GL_ARRAY_BUFFER, m_vertexBuffer);
    openGLCheck();
}

template <typename V>
std::size_t VertexBuffer<V>::size() const noexcept {
    return m_vertices.size();
}

template <typename V>
VertexBuffer<V>::~VertexBuffer() {
    glDeleteBuffers(1, &m_vertexBuffer);
}

template <typename T>
IndexBuffer<T>::IndexBuffer(std::vector<T>&& indices) : m_indices{ std::move(indices) } {
    glGenBuffers(1, &m_indexBuffer);
    openGLCheck();

    updateData();
}

template <typename T>
IndexBuffer<T>::IndexBuffer(const std::vector<T>& indices) : m_indices{ indices } {
    glGenBuffers(1, &m_indexBuffer);
    openGLCheck();

    updateData();
}

template <typename T>
void IndexBuffer<T>::updateData(std::vector<T>&& indices) {
    m_indices = std::move(indices);
    updateData();
}

template <typename T>
void IndexBuffer<T>::updateData(const std::vector<T>& indices) {
    m_indices = indices;
    updateData();
}

template <typename T>
void IndexBuffer<T>::updateData() const {
    bind();

    glBufferData(
        GL_ELEMENT_ARRAY_BUFFER, m_indices.size() * sizeof(T), m_indices.data(), GL_STATIC_DRAW);
    openGLCheck();
}

template <typename T>
void IndexBuffer<T>::addData(std::vector<T>&& indices) {
    m_indices.insert(m_indices.end(),
                     std::make_move_iterator(indices.begin()),
                     std::make_move_iterator(indices.end()));
    updateData();
}

template <typename T>
void IndexBuffer<T>::addData(const std::vector<T>& indices) {
    m_indices.insert(m_indices.end(), indices.begin(), indices.end());
    updateData();
}

template <typename T>
void IndexBuffer<T>::clear() {
    m_indices.clear();

    glBufferData(GL_ELEMENT_ARRAY_BUFFER, 0, nullptr, GL_STATIC_DRAW);
    openGLCheck();
}

template <typename T>
void IndexBuffer<T>::bind() const {
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indexBuffer);
    openGLCheck();
}

template <typename T>
std::size_t IndexBuffer<T>::size() const noexcept {
    return m_indices.size();
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