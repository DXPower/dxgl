#pragma once

#include <glm/vec2.hpp>
#include <string>
#include <vector>

enum class TileType {
    Grass,
    Dirt,
    Tile
};

struct TileData {
    TileType type{};

    constexpr bool operator==(const TileData& rhs) const = default;
};

using TileCoord = glm::ivec2;

struct Tile {
    TileCoord coord{};
    TileData data{};
};

struct TileMeta {
    TileType type{};
    std::string friendly_name{};
    // TODO: Use asset manager for this
    std::string spritesheet_name{};
    glm::ivec2 sprite_origin{};
    glm::ivec2 sprite_size{};
};

std::vector<TileMeta> LoadTileMetas();