#include "Vao.hpp"

#include <glad/glad.h>

Vao::Vao() {
    glGenVertexArrays(1, &handle);
}

void Vao::UseImpl() const {
    glBindVertexArray(handle);
}

void Vao::DestroyImpl() const {
    glDeleteVertexArrays(1, &handle);
}