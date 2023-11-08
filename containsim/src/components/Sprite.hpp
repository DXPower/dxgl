#pragma once

#include <dxgl/Texture.hpp>
#include <common/Rect.hpp>

namespace components {
    struct Sprite {
        dxgl::TextureRef spritesheet{};
        Rect cutout{};
    };

    struct SpriteRenderer { };
}

