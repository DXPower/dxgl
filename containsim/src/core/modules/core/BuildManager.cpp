#include <modules/core/BuildManager.hpp>
#include <magic_enum/magic_enum.hpp>

#include <ranges>

using namespace core;

BuildManager::BuildManager(core::TileGrid& tile_grid, application::EventManager& em)
    : EventCommandable(em)
    , m_tile_grid(&tile_grid) {
}

void BuildManager::PlaceTile(TileCoord coord, TileType type) {
    // TODO: Use a proper asset manager for this
    static const auto metas = LoadTileMetas();

    const auto layer = metas.at(type).layer;
    auto tile = m_tile_grid->GetTile(coord, layer);
    tile.type = type;

    m_tile_grid->SetTile(coord, layer, tile);
}

void BuildManager::DeleteTopmostTile(TileCoord coord, TileLayer stop_at) {
    for (auto layer : magic_enum::enum_values<TileLayer>() | std::views::reverse) {
        const auto& tile = m_tile_grid->GetTile(coord, layer);

        if (tile.type != TileType::Nothing) {
            m_tile_grid->SetTile(coord, layer, TileData{});
            return;
        }

        if (layer == stop_at)
            return;
    }
}