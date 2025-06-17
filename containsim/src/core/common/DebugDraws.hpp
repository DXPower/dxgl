#pragma once

#include <modules/rendering/DrawQueues.hpp>
#include <dxgl/Draw.hpp>
#include <dxgl/Ubo.hpp>
#include <glm/vec4.hpp>

class DebugDraws {
    inline static rendering::DrawQueues* m_queues{};

public:
    static void Init(dxgl::UboBindingManager& ubos, rendering::DrawQueues& queues);

    // Creates a draw but does not fill in prim type or options.
    // Points should be provided in world space.
    static void MakeWorldDraw(std::span<const glm::vec2> points, glm::vec4 color, dxgl::PrimType prim_type);
};