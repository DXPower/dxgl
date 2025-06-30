#include <systems/TilePrefabs.hpp>
#include <common/Tile.hpp>
#include <components/Transform.hpp>
#include <modules/physics/Physics.hpp>
#include <modules/core/Core.hpp>

using namespace components;

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
