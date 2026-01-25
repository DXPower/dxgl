#include <modules/core/BuildManager.hpp>

#include <format>
#include <ranges>

using namespace core;

BuildManager::BuildManager(core::TileGrid& tile_grid, const ResourceRegistry& resources, application::EventManager& em)
    : EventCommandable(em)
    , m_tile_grid(&tile_grid)
    , m_resources(&resources) {
}

void BuildManager::PlaceTile(TileCoord coord, TileType type) {
    const auto meta = m_resources->TryGetById<TileTypeMeta>(type);

    if (!meta) {
        throw std::runtime_error(std::format("Invalid tile type: {}", type));
    }

    auto tile = m_tile_grid->GetTile(coord, meta->layer);
    tile.type = type;

    m_tile_grid->SetTile(coord, meta->layer, tile);
}

void BuildManager::DeleteTopmostTile(TileCoord coord, TileLayer stop_at) {
    for (auto layer : magic_enum::enum_values<TileLayer>() | std::views::reverse) {
        const auto& tile = m_tile_grid->GetTile(coord, layer);

        if (tile.type != NothingTile) {
            m_tile_grid->SetTile(coord, layer, TileData{});
            return;
        }

        if (layer == stop_at)
            return;
    }
}