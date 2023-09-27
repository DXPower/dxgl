#include <dxgl/Cubemap.hpp>

#include <cassert>

#include <glad/glad.h>

using namespace dxgl;

Cubemap::Cubemap() {
    glGenTextures(1, &handle);
}

Cubemap::Cubemap(std::span<const TextureSource, 6> faces) : Cubemap() {
    Load(faces);
}

void Cubemap::Load(std::span<const TextureSource, 6> faces) {
    Use();

    for (std::size_t i = 0; i < faces.size(); i++) {
        const auto& source = faces[i];

        auto [w, h] = source.GetDims();

        int gl_format;
        switch (source.GetFormat()) {
            using enum TextureFormat;
            case R: gl_format = GL_RED; break;
            case RG: gl_format = GL_RG; break;
            case RGB: gl_format = GL_RGB; break;
            case RGBA: gl_format = GL_RGBA; break;
        }

        glTexImage2D(
            GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 
            0, GL_RGB, w, h, 0,
            gl_format, GL_UNSIGNED_BYTE, source.GetData()
        );
    }
    
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
}

void Cubemap::UseImpl() const {
    glBindTexture(GL_TEXTURE_CUBE_MAP, handle);
}

void Cubemap::DestroyImpl() const {
    glDeleteTextures(1, &handle);
}
