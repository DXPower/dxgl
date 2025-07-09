#include <modules/core/TileGrid.hpp>
#include <modules/core/Configuration.hpp>
#include <components/Transform.hpp>

using namespace core;

TileGrid::TileGrid(flecs::world& world)
    : m_world(&world)
{
    m_tile_world_size = world.get<core::TileWorldSize>().value;
    m_grid_size = world.get<core::MapSize>().value;

    for (auto& layer : m_tiles) {
        layer.resize(boost::extents[m_grid_size.x][m_grid_size.y]);
    }

    for (auto layer : magic_enum::enum_values<TileLayer>()) {
        for (int x = 0; (std::size_t) x < m_tiles[layer].size(); x++) {
            for (int y = 0; (std::size_t) y < m_tiles[layer][x].size(); y++) {
                auto& tile = m_tiles[layer][x][y];
                tile.coord = {x, y};
                tile.layer = layer;
                tile.data.type = TileType::Nothing;
            }
        }
    }
}

void TileGrid::SetTile(TileCoord coord, TileLayer layer, TileData data) {
    static auto tile_metas = LoadTileMetas();

    auto& tile = m_tiles[layer][coord.x][coord.y];

    if (tile.data != data) {
        // Destroy the old entity if it exists
        if (tile.entity)
            tile.entity.destruct();

        tile.data = data;

        // Lookup the prefab
        auto meta_it = tile_metas.find(data.type);

        if (meta_it != tile_metas.end() && meta_it->second.prefab_name.has_value()) {
            const auto& meta = meta_it->second;
            auto prefab = m_world->lookup(meta.prefab_name->c_str());

            if (!prefab) {
                throw std::runtime_error("Prefab not found: " + *meta.prefab_name);
            }

            assert(prefab.has<components::Transform>());

            tile.entity = m_world->entity()
                .is_a(prefab)
                .set<TileCoord>(coord);

            tile.entity.get_mut<components::Transform>().position = TileCoordToWorldPos(coord);
        }

        tile_update_signal.fire(*this, tile);
    }
}

const TileData& TileGrid::GetTile(TileCoord coord, TileLayer layer) const {
    return m_tiles[layer][coord.x][coord.y].data;
}

RoomId TileGrid::GetRoomAt(TileCoord coord) const {
    return m_tiles[TileLayer::Ceiling][coord.x][coord.y].room;
}

void TileGrid::SetRoomAt(TileCoord coord, RoomId room_id) {
    for (auto layer : magic_enum::enum_values<TileLayer>()) {
        m_tiles[layer][coord.x][coord.y].room = room_id;
    }
}

std::optional<TileCoord> TileGrid::WorldPosToTileCoord(glm::vec2 world_pos) const {
    auto res = (world_pos + (m_tile_world_size / glm::vec2(2, 2))) / m_tile_world_size;
    
    if (res.x < 0 || res.y < 0 || res.x >= (float) m_grid_size.x || res.y >= (float) m_grid_size.y) [[unlikely]]
        return std::nullopt;

    return static_cast<TileCoord>(res);
}

glm::vec2 TileGrid::TileCoordToWorldPos(TileCoord coord) const {
    // Don't need to offset because the tile center is the origin in world space
    return glm::vec2(coord) * m_tile_world_size;
}

TileCoordNeighbors core::GetTileCoordNeighbors(const TileGrid& grid, TileCoord coord) {
    TileCoordNeighbors neighbors{};
    auto grid_size = grid.GetGridSize();

    if (coord.x > 0) {
        neighbors.west = {coord.x - 1, coord.y};
        if (coord.y > 0) {
            neighbors.northwest = {coord.x - 1, coord.y - 1};
        }
        if (coord.y < grid_size.y - 1) {
            neighbors.southwest = {coord.x - 1, coord.y + 1};
        }
    }

    if (coord.x < grid_size.x - 1) {
        neighbors.east = {coord.x + 1, coord.y};
        if (coord.y > 0) {
            neighbors.northeast = {coord.x + 1, coord.y - 1};
        }
        if (coord.y < grid_size.y - 1) {
            neighbors.southeast = {coord.x + 1, coord.y + 1};
        }
    }

    if (coord.y > 0) {
        neighbors.north = {coord.x, coord.y - 1};
    }

    if (coord.y < grid_size.y - 1) {
        neighbors.south = {coord.x, coord.y + 1};
    }

    return neighbors;
}