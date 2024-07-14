#include <services/BuildManager.hpp>

using namespace services;

BuildManager::BuildManager(/*GameState& game_state, */TileGrid& tile_grid)
    : /*m_game_state(&game_state),*/ m_tile_grid(&tile_grid) { }

void BuildManager::PlaceTile(TileCoord coord, TileType type) {
    auto tile = m_tile_grid->GetTile(coord);
    tile.type = type;

    m_tile_grid->SetTile(coord, tile);
}