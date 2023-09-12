#include "Framebuffer.hpp"

#include <glad/glad.h>

Framebuffer::Framebuffer() {
    glGenFramebuffers(1, &handle);
}

void Framebuffer::UseImpl() const {
    glBindFramebuffer(GL_FRAMEBUFFER, handle);
}

void Framebuffer::DestroyImpl() const {
    glDeleteFramebuffers(1, &handle);
}