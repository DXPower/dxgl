#pragma once

#include <modules/rendering/RenderData.hpp>
#include <modules/rendering/Sprite.hpp>
#include <modules/rendering/DrawQueues.hpp>
#include <modules/rendering/Camera.hpp>
#include <modules/rendering/ScreenbufferComponents.hpp>
#include <dxgl/Ubo.hpp>

#include <flecs.h>

namespace rendering {
    
    class Rendering {
    public:
        Rendering(flecs::world& world);
    };
}