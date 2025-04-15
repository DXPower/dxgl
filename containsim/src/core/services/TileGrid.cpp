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

std::optional<glm::ivec2> TileGrid::WorldPosToTileCoord(glm::vec2 world_pos) const {
    auto res = (world_pos + (m_tile_world_size / glm::vec2(2, 2))) / m_tile_world_size;
    
    if (res.x < 0 || res.y < 0 || res.x >= m_grid_size.x || res.y >= m_grid_size.y) [[unlikely]]
        return std::nullopt;

    return static_cast<glm::ivec2>(res);
}