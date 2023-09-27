#pragma once

#include "Texture.hpp"
#include "dxtl/cstring_view.hpp"

#include <unordered_map>

namespace dxgl {
    class TextureStore {
        std::unordered_map<std::string, Texture> textures;

    public:
        TextureRef LoadTexture(dxtl::cstring_view file);

        const auto& GetTextures() const { return textures; }
    };
}