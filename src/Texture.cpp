#include "Texture.hpp"
#include <stdexcept>
#include <format>
#include <string>

#include <stb/stb_image.h>
#include <glad/glad.h>

TextureSource::TextureSource(std::string_view file) {
    int num_channels;
    
    stbi_set_flip_vertically_on_load(true);  
    m_data.reset(stbi_load(std::string(file).c_str(), &size.w, &size.h, &num_channels, 0));

    if (!m_data) {
        throw std::runtime_error(std::format("Failed to load image {}", file));
    }

    format = num_channels == 3 ? TextureFormat::RGB : TextureFormat::RGBA;
}

void TextureSource::Deleter::operator()(unsigned char* ptr) {
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

    auto [w, h] = source.GetDims();

    auto gl_format = source.GetFormat() == TextureFormat::RGB ? GL_RGB : GL_RGBA;
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, gl_format, GL_UNSIGNED_BYTE, source.GetData());
    glGenerateMipmap(GL_TEXTURE_2D);
}

void Texture::UseImpl(int texture_unit) const {
    glActiveTexture(GL_TEXTURE0 + texture_unit);
    glBindTexture(GL_TEXTURE_2D, handle);
}

void Texture::DestroyImpl() const {
    glDeleteTextures(1, &handle);
}

Texture LoadTextureFromFile(std::string_view file) {
    TextureSource source(file);
    return Texture(source);
}