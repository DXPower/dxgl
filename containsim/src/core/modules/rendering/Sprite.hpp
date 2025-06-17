#pragma once

#include <dxgl/Texture.hpp>
#include <common/Rect.hpp>

namespace rendering {
    struct Sprite {
        dxgl::TextureRef spritesheet{};
        Rect cutout{};
    };

    struct SpriteRenderer { };
}

