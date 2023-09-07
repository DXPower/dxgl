#pragma once

#include "Texture.hpp"

#include <unordered_map>

class TextureStore {
    std::unordered_map<std::string, Texture> textures;

public:
    TextureRef LoadTexture(std::string_view )
};