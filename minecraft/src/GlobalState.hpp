#pragma once

#include <dxgl/Ubo.hpp>
#include "Debug.hpp"

enum class UboLocs {
    Camera = 0
};

struct GlobalState {
    dxgl::UboBindingManager ubo_manager{};
    DebugDraws debug_draws{};
};