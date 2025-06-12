#include <systems/TilePrefabs.hpp>
#include <common/Tile.hpp>
#include <components/Transform.hpp>
#include <modules/physics/Collider.hpp>

using namespace components;

void services::InitTilePrefabs(flecs::world& world, const GlobalConfig& config) {
    world.prefab("cs:PrefabWall")
        .set(Transform{
            .size = config.tile_size
        })
        .set(physics::Collider{
            .is_fixed = true
        })
        .add<physics::SquareCollider>()
        .add<TileCoord>();
}
