#pragma once

#include <dxgl/Ubo.hpp>

enum class UboLocs {
    Camera = 0
};

struct GlobalState {
    dxgl::UboBindingManager ubo_manager{};
};