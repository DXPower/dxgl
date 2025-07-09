#include <modules/core/Core.hpp>

using namespace core;

Core::Core(flecs::world& world) {
    // Register core configurations
    core::RegisterCoreConfigurations(world);

    world.component<Actor>();
    world.component<Transform>();
    world.component<Mobility>();

    world.component<TileGrid>().add(flecs::Sparse);
    world.emplace<TileGrid>(world);

    auto& tile_grid = world.get_mut<TileGrid>();

    world.component<BuildManager>().add(flecs::Sparse);
    world.emplace<BuildManager>(tile_grid);

    world.component<RoomManager>().add(flecs::Sparse);
    world.emplace<RoomManager>(world.get_mut<TileGrid>());
}