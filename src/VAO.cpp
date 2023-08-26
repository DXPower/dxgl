#include "VAO.hpp"

#include <glad/glad.h>

VAO::VAO() {
    glGenVertexArrays(1, &handle);
}

void VAO::UseImpl() const {
    glBindVertexArray(handle);
}

void VAO::DestroyImpl() const {
    glDeleteVertexArrays(1, &handle);
}