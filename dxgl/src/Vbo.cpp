#include <dxgl/Vbo.hpp>

#include <glad/glad.h>

using namespace dxgl;

Vbo::Vbo() {
    glGenBuffers(1, &handle);
}

void Vbo::Upload(std::span<const std::byte> data) {
    Use();
    
    if (buffer_size != data.size()) {
        glBufferData(GL_ARRAY_BUFFER, data.size(), data.data(), GL_STATIC_DRAW);
        buffer_size = data.size();
    } else {
        glBufferSubData(GL_ARRAY_BUFFER, 0, data.size(), data.data());
    }
}

void Vbo::UseImpl() const {
    glBindBuffer(GL_ARRAY_BUFFER, handle);
}

void Vbo::DestroyImpl() const {
    glDeleteBuffers(1, &handle);
}

