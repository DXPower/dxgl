#include <services/TileGrid.hpp>

using namespace services;

TileGrid::TileGrid(const GlobalConfig& config)
    : m_grid_size(config.map_size),
      m_tile_world_size(config.tile_size)
{
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
    auto& tile = m_tiles[layer][coord.x][coord.y];

    if (tile.data != data) {
        tile.data = data;
        tile_update_signal.fire(*this, tile);
    }
}

const TileData& TileGrid::GetTile(TileCoord coord, TileLayer layer) const {
    return m_tiles[layer][coord.x][coord.y].data;
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

TileCoordNeighbors services::GetTileCoordNeighbors(const TileGrid& grid, TileCoord coord) {
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