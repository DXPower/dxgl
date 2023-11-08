#include <dxgl/Texture.hpp>
#include <dxtl/cstring_view.hpp>
#include <stdexcept>
#include <format>
#include <string>

#include <stb/stb_image.h>
#include <glad/glad.h>

using namespace dxgl;

TextureSource::TextureSource(dxtl::cstring_view file) {
    int num_channels;
    
    stbi_set_flip_vertically_on_load(true);  
    m_data.reset(stbi_load(file.c_str(), &size.x, &size.y, &num_channels, 0));

    if (!m_data) {
        throw std::runtime_error(std::format("Failed to load image {}", file.c_str()));
    }

    format = static_cast<TextureFormat>(num_channels);
}

void TextureSource::Deleter::operator()(unsigned char* ptr) const {
    stbi_image_free(ptr);
}

Texture::Texture() {
    glGenTextures(1, &handle);
}

Texture::Texture(const TextureSource& source) : Texture() {
    Load(source);
}

void Texture::Load(const TextureSource& source) {
    glBindTexture(GL_TEXTURE_2D, handle);

    auto [w, h] = size = source.GetDims();

    int gl_format;
    switch (source.GetFormat()) {
        using enum TextureFormat;
        case R: gl_format = GL_RED; break;
        case RG: gl_format = GL_RG; break;
        case RGB: gl_format = GL_RGB; break;
        case RGBA: gl_format = GL_RGBA; break;
    }
    
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, gl_format, GL_UNSIGNED_BYTE, source.GetData());
    glGenerateMipmap(GL_TEXTURE_2D);
}

void Texture::SetWrapMode(WrapMode mode) {
    glBindTexture(GL_TEXTURE_2D, handle);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, static_cast<int>(mode));
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, static_cast<int>(mode));
}

void Texture::SetFilterMode(FilterMode mode) {
    glBindTexture(GL_TEXTURE_2D, handle);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, static_cast<int>(mode));
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, static_cast<int>(mode));
}

void Texture::UseImpl(int texture_unit) const {
    glActiveTexture(GL_TEXTURE0 + texture_unit);
    glBindTexture(GL_TEXTURE_2D, handle);
}

void Texture::DestroyImpl() const {
    glDeleteTextures(1, &handle);
}

Texture dxgl::LoadTextureFromFile(dxtl::cstring_view file) {
    TextureSource source(file);
    return Texture(source);
}