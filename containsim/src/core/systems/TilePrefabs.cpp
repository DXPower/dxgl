#include <systems/TilePrefabs.hpp>
#include <common/Tile.hpp>
#include <components/Transform.hpp>
#include <components/Collider.hpp>

using namespace components;

void services::InitTilePrefabs(flecs::world& world, const GlobalConfig& config) {
    world.prefab("cs:PrefabWall")
        .set(Transform{
            .size = config.tile_size
        })
        .set(Collider{
            .is_fixed = true
        })
        .add<SquareCollider>()
        .add<TileCoord>();
}
