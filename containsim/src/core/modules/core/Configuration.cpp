#include <modules/core/Configuration.hpp>

using namespace core;

void core::RegisterCoreConfigurations(flecs::world& world) {
    world.set(MapSize{{20, 10}});         // NOLINT
    world.set(TileWorldSize{{100, 100}}); // NOLINT
}