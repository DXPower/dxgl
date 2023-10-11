#include <dxgl/Ebo.hpp>

#include <glad/glad.h>

using namespace dxgl;

Ebo::Ebo() {
    glGenBuffers(1, &handle);
}

void Ebo::Upload(std::span<const std::byte> data, BufferUsage usage) {
    int gl_usage = usage == BufferUsage::Static ? GL_STATIC_DRAW : GL_DYNAMIC_DRAW;

    Use();
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, data.size(), data.data(), gl_usage);
}

void Ebo::UseImpl() const {
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, handle);
}

void Ebo::DestroyImpl() const {
    glDeleteBuffers(1, &handle);
}

