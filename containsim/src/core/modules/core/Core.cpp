#include <modules/core/Core.hpp>

using namespace core;

Core::Core(flecs::world& world) {
    // Register core configurations
    core::RegisterCoreConfigurations(world);

    // Register TileGrid
    world.component<TileGrid>().add(flecs::Sparse);
    world.emplace<TileGrid>(world);

    // Register RoomManager
    world.component<RoomManager>().add(flecs::Sparse);
    world.emplace<RoomManager>(world.get_mut<TileGrid>());
}