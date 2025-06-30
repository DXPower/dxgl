#pragma once

#include <modules/core/RoomManager.hpp>
#include <modules/rendering/DrawQueues.hpp>

#include <dxgl/Ubo.hpp>

#include <glm/vec2.hpp>

namespace rendering {
    void RoomRendererSystem(flecs::world& world);
}