#include <modules/pathing/Pathing.hpp>
#include <modules/pathing/Pathfinder.hpp>
#include <modules/pathing/PathMover.hpp>

using namespace pathing;

Pathing::Pathing(flecs::world& world) {
    const auto& tile_grid = world.get<services::TileGrid>();
    
    world.component<Pathfinder>().add(flecs::Sparse);
    world.emplace<Pathfinder>(tile_grid);

    auto& pathfinder = world.get_mut<Pathfinder>();
    pathfinder.PreUpdate(world);

    PathMoverSystem(world);
}