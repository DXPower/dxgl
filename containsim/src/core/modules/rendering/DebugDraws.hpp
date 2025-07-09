#pragma once

#include <modules/rendering/DrawQueues.hpp>
#include <dxgl/Draw.hpp>
#include <dxgl/Ubo.hpp>
#include <glm/vec4.hpp>

namespace rendering {
class DebugDraws {
    dxgl::Program m_debug_program{};
    
public:
    DebugDraws(dxgl::UboBindingManager& ubos);

    // Creates a draw but does not fill in prim type or options.
    // Points should be provided in world space.
    dxgl::Draw MakeWorldDraw(std::span<const glm::vec2> points, glm::vec4 color, dxgl::PrimType prim_type);
};
}