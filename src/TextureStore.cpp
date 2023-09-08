#include "TextureStore.hpp"

TextureRef TextureStore::LoadTexture(dxtl::cstring_view file) {
    if (auto tex = textures.find(file); tex != textures.end())
        return tex->second;

    auto tex = textures.emplace(file, LoadTextureFromFile(file));
    return tex.first->second;
}