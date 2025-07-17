#include <modules/pathing/Pathing.hpp>
#include <modules/pathing/Pathfinder.hpp>
#include <modules/pathing/PathMover.hpp>
#include <modules/core/Core.hpp>

using namespace pathing;

Pathing::Pathing(flecs::world& world) {
    world.import<core::Core>();

    const auto& tile_grid = world.get<core::TileGrid>();
    
    world.component<Pathfinder>().add(flecs::Sparse);
    world.emplace<Pathfinder>(tile_grid);

    auto& pathfinder = world.get_mut<Pathfinder>();
    pathfinder.PreUpdate(world);

    PathMoverSystem(world);
}