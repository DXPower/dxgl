#include <modules/core/Core.hpp>
#include <modules/application/Application.hpp>

using namespace core;

Core::Core(flecs::world& world) {
    world.import<application::Application>();

    // Register core configurations
    core::RegisterCoreConfigurations(world);

    world.component<Actor>();
    world.component<Transform>();
    world.component<Mobility>();

    world.component<TileGrid>().add(flecs::Sparse);
    world.emplace<TileGrid>(world);

    auto& tile_grid = world.get_mut<TileGrid>();
    auto& event_manager = world.get_mut<application::EventManager>();

    world.component<BuildManager>().add(flecs::Sparse);
    world.emplace<BuildManager>(tile_grid, event_manager);

    world.component<RoomManager>().add(flecs::Sparse);
    world.emplace<RoomManager>(tile_grid, event_manager);
}