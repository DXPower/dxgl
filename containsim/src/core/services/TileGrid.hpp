#pragma once

#include <common/Tile.hpp>
#include <common/GlobalConfig.hpp>

#include <glm/vec2.hpp>
#include <flecs.h>
#include <nano/nano_signal_slot.hpp>
#include <boost/multi_array.hpp>
#include <magic_enum/magic_enum_containers.hpp>
#include <optional>


namespace services {
    class TileGrid {
        magic_enum::containers::array<TileLayer, boost::multi_array<Tile, 2>> m_tiles;
        TileCoord m_grid_size{};
        glm::vec2 m_tile_world_size{};
        flecs::world* m_world{};

    public:
        mutable Nano::Signal<void(const TileGrid&, const Tile&)> tile_update_signal{};

        TileGrid(const GlobalConfig& config, flecs::world& world);

        void SetTile(TileCoord coord, TileLayer layer, TileData data);
        const TileData& GetTile(TileCoord coord, TileLayer layer) const;

        std::optional<TileCoord> WorldPosToTileCoord(glm::vec2 world_pos) const;
        glm::vec2 TileCoordToWorldPos(TileCoord coord) const;
        
        TileCoord GetGridSize() const { return m_grid_size; }
        glm::vec2 GetTileWorldSize() const { return m_tile_world_size; }

        const auto& GetUnderlyingGrid() const { return m_tiles; }
    };

    TileCoordNeighbors GetTileCoordNeighbors(const TileGrid& grid, TileCoord coord);
}
