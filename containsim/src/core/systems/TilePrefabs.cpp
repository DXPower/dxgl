#include <systems/TilePrefabs.hpp>
#include <modules/core/Tile.hpp>
#include <modules/core/Transform.hpp>
#include <modules/physics/Physics.hpp>
#include <modules/core/Core.hpp>

using namespace core;

void services::InitTilePrefabs(flecs::world& world) {
    world.import<core::Core>();
    world.import<physics::Physics>();

    const auto& tile_size = world.get<core::TileWorldSize>().value;

    world.prefab("cs:PrefabWall")
        .set(Transform{
            .size = tile_size
        })
        .set(physics::Collider{
            .is_fixed = true
        })
        .add<physics::SquareCollider>()
        .add<TileCoord>();
}
