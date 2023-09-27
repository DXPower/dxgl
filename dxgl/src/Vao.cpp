#include <dxgl/Vao.hpp>

#include <glad/glad.h>

using namespace dxgl;

Vao::Vao() {
    glGenVertexArrays(1, &handle);
}

void Vao::UseImpl() const {
    glBindVertexArray(handle);
}

void Vao::DestroyImpl() const {
    glDeleteVertexArrays(1, &handle);
}