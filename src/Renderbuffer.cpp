#include "Renderbuffer.hpp"

#include <glad/glad.h>

Renderbuffer::Renderbuffer() {
    glGenRenderbuffers(1, &handle);
}

void Renderbuffer::UseImpl() const {
    glBindRenderbuffer(GL_RENDERBUFFER, handle);
}

void Renderbuffer::DestroyImpl() const {
    glDeleteRenderbuffers(1, &handle);
}