#include <modules/core/Core.hpp>

using namespace core;

Core::Core(flecs::world& world) {
    // Register core configurations
    core::RegisterCoreConfigurations(world);

    world.component<TileGrid>().add(flecs::Sparse);
    world.emplace<TileGrid>(world);

    world.component<RoomManager>().add(flecs::Sparse);
    world.emplace<RoomManager>(world.get_mut<TileGrid>());

    world.component<EventManager>().add(flecs::Sparse);
    world.add<EventManager>();
}