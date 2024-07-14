#pragma once

#include <common/Tile.hpp>
#include <services/GameState.hpp>
#include <services/TileGrid.hpp>

namespace services {
    class BuildManager {
        // GameState* m_game_state{};
        TileGrid* m_tile_grid{};

    public:
        BuildManager(/*GameState& game_state, */TileGrid& tile_grid);

        void PlaceTile(TileCoord coord, TileType type);
    };
}