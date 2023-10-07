#include <dxgl/Vbo.hpp>

#include <stdexcept>
#include <glad/glad.h>

using namespace dxgl;

Vbo::Vbo() {
    glGenBuffers(1, &handle);
}

void Vbo::Upload(std::span<const std::byte> data, BufferUsage usage) {
    int gl_usage = usage == BufferUsage::Static ? GL_STATIC_DRAW : GL_DYNAMIC_DRAW;

    Use();
    glBufferData(GL_ARRAY_BUFFER, data.size(), data.data(), gl_usage);
    buffer_size = data.size();
}

void Vbo::Update(std::span<const std::byte> data, std::size_t offset) {
    if (data.size() + offset > buffer_size) {
        throw std::runtime_error("VBO Update exceeds previously allocated size");
    }

    Use();
    glBufferSubData(GL_ARRAY_BUFFER, offset, data.size(), data.data());
}

void Vbo::UseImpl() const {
    glBindBuffer(GL_ARRAY_BUFFER, handle);
}

void Vbo::DestroyImpl() const {
    glDeleteBuffers(1, &handle);
}

