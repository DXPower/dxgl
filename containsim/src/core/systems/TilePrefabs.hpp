#pragma once
#include <common/GlobalConfig.hpp>
#include <flecs.h>
#include <string_view>

namespace services {
    void InitTilePrefabs(flecs::world& world, const GlobalConfig& config);
}