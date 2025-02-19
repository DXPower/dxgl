#include <services/TileGrid.hpp>

using namespace services;

TileGrid::TileGrid(const GlobalConfig& config)
    : m_tiles(boost::extents[config.map_size.x][config.map_size.y]),
      m_grid_size(config.map_size),
      m_tile_world_size(config.tile_size)
{
    for (int x = 0; (std::size_t) x < m_tiles.size(); x++) {
        for (int y = 0; (std::size_t) y < m_tiles[x].size(); y++) {
            auto& tile = m_tiles[x][y];
            tile.coord = {x, y};
        }
    }
}

void TileGrid::SetTile(TileCoord coord, TileData data) {
    auto& tile = m_tiles[coord.x][coord.y];

    if (tile.data != data) {
        tile.data = data;
        tile_update_signal.fire(*this, coord);
    }
}

const TileData& TileGrid::GetTile(TileCoord coord) const {
    return m_tiles[coord.x][coord.y].data;
}

std::optional<glm::ivec2> TileGrid::WorldPosToTileCoord(glm::vec2 world_pos) const {
    auto res = (world_pos + (m_tile_world_size / glm::vec2(2, 2))) / m_tile_world_size;
    
    if (res.x < 0 || res.y < 0 || res.x >= m_grid_size.x || res.y >= m_grid_size.y) [[unlikely]]
        return std::nullopt;

    return static_cast<glm::ivec2>(res);
}