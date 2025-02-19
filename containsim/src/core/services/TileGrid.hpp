#pragma once

#include <common/Tile.hpp>
#include <common/GlobalConfig.hpp>

#include <glm/vec2.hpp>
#include <nano/nano_signal_slot.hpp>
#include <boost/multi_array.hpp>
#include <optional>


namespace services {
    class TileGrid {
        boost::multi_array<Tile, 2> m_tiles{};
        TileCoord m_grid_size{};
        glm::vec2 m_tile_world_size{};

    public:
        mutable Nano::Signal<void(const TileGrid&, TileCoord)> tile_update_signal{};

        TileGrid(const GlobalConfig& config);

        void SetTile(TileCoord coord, TileData data);
        const TileData& GetTile(TileCoord coord) const;

        std::optional<TileCoord> WorldPosToTileCoord(glm::vec2 world_pos) const;
        TileCoord GetGridSize() const { return m_grid_size; }
        glm::vec2 GetTileWorldSize() const { return m_tile_world_size; }

        const auto& GetUnderlyingGrid() const { return m_tiles; }
    };
}

// namespace TileGrid {
//     // class Module {
//     //     class Pimpl;
//     //     struct PimplDeleter {
//     //         void operator()(Pimpl* p) const;
//     //     };

//     //     std::unique_ptr<Pimpl, PimplDeleter> m_pimpl{};

//     // public:
//     //     // Module(flecs::world& world);
//     //     // Module(Module&& move);
//     //     // ~Module();
//     //     // Module& operator=(Module&& move);

//     //     void SetTile(const glm::ivec2& coord, const TileData& data);
//     //     glm::ivec2 WorldPosToTileCoord(glm::vec2 world_pos) const;

//     //     // void Render(DrawQueues& draws) const;
//     // };
// }