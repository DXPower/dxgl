#include <modules/core/Configuration.hpp>

using namespace core;

void core::RegisterCoreConfigurations(flecs::world& world) {
    world.component<MapSize>();
    world.component<TileWorldSize>();
    world.component<TickRate>();

    world.set(MapSize{{20, 10}});         // NOLINT
    world.set(TileWorldSize{{100, 100}}); // NOLINT
    world.set(TickRate{ch::duration<float>(1) / 4}); // NOLINT
}